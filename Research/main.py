import sys
import scipy.io.wavfile as wavfile
from scipy.signal import resample
import matplotlib.pyplot as plt
import numpy
from numpy.fft import rfft

fs_mir = 11025.0
frame_time = 100.0 # in ms
hop_time = 50.0
fft_point = 1024
zero_padding_rate = 0.9
min_freq = 80.0 # in Hz
max_freq = 3000.0 # in Hz
NUM_NOTES = 120
FREQREF = 7.943049790954413
_2ROOT12 = 1.059463094
LOG10_2ROOT12 = numpy.log10(_2ROOT12)
spectral_flux_diff_size = 5
SILENCE = -90.

def get_cavatina_score():
    return [(1.0/6.0, 71), (2.0/6.0, 56), (3.0/6.0, 59), (4.0/6.0, 64), (5.0/6.0, 59), (6.0/6.0, 56)]

def peak_picking(spectrum, num_peaks, radius=3):
    peaks = []
    for i in range(len(spectrum)):
        if spectrum[i] == 0 or spectrum[i-1] >= spectrum[i] or spectrum[i] <= spectrum[i+1]:
            continue
        j = i - 1
        k = i + 1
        area = 0 # above the curve
        while spectrum[j] < spectrum[j+1] and spectrum[k] < spectrum[k-1]:
            area += 2*spectrum[i] - spectrum[j] - spectrum[k]
            j -= 1
            k += 1
        if k - i > radius:
            bottom_val = min(spectrum[j], spectrum[k])
            peaks.append((i, (spectrum[i] - bottom_val) * (k - j + 1) - area))
    peaks.sort(reverse=True, key=lambda x: x[1])
    return peaks[:num_peaks]

def normalize_matrix(mat):
    mat_max = numpy.max(numpy.max(mat))
    mat_min = numpy.min(numpy.min(mat))
    mat -= mat_min
    mat /= (mat_max - mat_min)
    return mat

if __name__ == "__main__":
    filename = "../Resources/first_bar.wav"
    if len(sys.argv) == 2:
        filename = sys.argv[1]
    [fs, audio] = wavfile.read(open(filename, 'r'))
    # stereo to mono
    audio = numpy.average(audio, axis=1)
    
    # normalize
    maxval = max(max(audio), abs(min(audio)))
    audio /= maxval

    # resample
    audio = resample(audio, audio.size * fs_mir / fs)

    # frame-level analysis
    frame_size = int(frame_time / 1000.0 * fs_mir)
    hop_size = int(hop_time / 1000.0 * fs_mir)
    num_frames = (audio.size - frame_size) / hop_size + 1

    while fft_point < frame_size * (1 + zero_padding_rate):
        fft_point *= 2

    freq_res = fs_mir / fft_point
    min_bin = int(min_freq / freq_res)
    max_bin = int(max_freq / freq_res)
    spectrum_size = fft_point / 2

    # pre-calculate FFT_bin to MIDI note mapping
    FFT_bin_2_MIDI_note_mapping = [0] * spectrum_size
    MIDI_note_width = [0] * NUM_NOTES
    for i in range(spectrum_size):
        if i > 0:
            FFT_bin_2_MIDI_note_mapping[i] = int(numpy.log10(i * freq_res / FREQREF) / LOG10_2ROOT12)
    for i in range(NUM_NOTES):
        MIDI_note_width[i] = _2ROOT12 ** i
    min_note = FFT_bin_2_MIDI_note_mapping[min_bin]
    max_note = FFT_bin_2_MIDI_note_mapping[max_bin]

    # calculate chroma
    chroma_matrix = numpy.zeros((num_frames, NUM_NOTES)) # store raw energy
    for i in range(num_frames):
        fft_result = rfft(audio[i*hop_size:(i+1)*hop_size], fft_point)
        for j in range(spectrum_size):
            if j >= min_bin and j <= max_bin:
                chroma_matrix[i, FFT_bin_2_MIDI_note_mapping[j]] += abs(fft_result[j])

    # take into account the note widths
    for i in range(num_frames):
        for j in range(NUM_NOTES):
            chroma_matrix[i, j] /= MIDI_note_width[j]

    # pick local peaks in chroma
    pitch_matrix = numpy.zeros((num_frames, NUM_NOTES))
    for i in range(num_frames):
        for j in range(NUM_NOTES):
            if j >= min_note and j <= max_note and chroma_matrix[i, j-1] < chroma_matrix[i, j] and chroma_matrix[i, j] > chroma_matrix[i, j+1]:
                pitch_matrix[i, j] = chroma_matrix[i, j]
    pitch_matrix = normalize_matrix(pitch_matrix) # Hey, it's normalized!

    # spectral flux and find all onsets (lots of false positives!)
    notes = [] # frame index, midi note number and value
    for j in range(NUM_NOTES):
        flux = numpy.zeros(num_frames)
        for i in range(num_frames):
            if i < spectral_flux_diff_size and pitch_matrix[i, j] > 0.:
                energy_ahead = sum(pitch_matrix[i:spectral_flux_diff_size, j]) / (spectral_flux_diff_size-i)
                energy_behind = sum(pitch_matrix[0:i, j]) / i if i > 0 else 0.
                flux[i] = energy_ahead - energy_behind
            elif i >= spectral_flux_diff_size and i <= num_frames - spectral_flux_diff_size:
                energy_ahead = sum(pitch_matrix[i:(i+spectral_flux_diff_size), j])
                energy_behind = sum(pitch_matrix[(i-spectral_flux_diff_size):i, j])
                flux[i] = (energy_ahead - energy_behind) / spectral_flux_diff_size if energy_ahead > energy_behind else 0.

        # pick local peaks in flux
        if flux[0] > 0.:
            notes.append((0, j, pitch_matrix[0, j]))
        for i in range(1, num_frames-1):
            if flux[i-1] < flux[i] and flux[i] > flux[i+1]:
                notes.append((i, j, pitch_matrix[i, j]))

    note_sequence = [list() for _ in range(num_frames)]
    for note in notes:
        note_sequence[note[0]].append((note[1], note[2]))

    # # dynamic programming for alignment
    score = get_cavatina_score()
    num_notes_in_score = len(score)
    S = numpy.zeros((num_frames+1, num_notes_in_score+1))
    P = numpy.zeros((num_frames+1, num_notes_in_score+1)) # 0:from i, 1:from j, 2:from diag

    for i in range(1, num_frames+1):
        for j in range(1, num_notes_in_score+1):
            value = 0.
            for note in note_sequence[i-1]:
                if note[0] == score[j-1][1]:
                    value = note[1] + S[i-1, j-1]
                    break
            if value > S[i-1, j] and value > S[i, j-1]:
                S[i, j] = value
                P[i, j] = 2
            elif S[i-1, j] > S[i, j-1]:
                S[i, j] = S[i-1, j]
                P[i, j] = 0
            else:
                S[i, j] = S[i, j-1]
                P[i, j] = 1

    # backtracking
    curr_i = num_frames
    curr_j = num_notes_in_score
    aligned_notes = []
    while curr_i > 0 and curr_j > 0:
        direction = P[curr_i, curr_j]
        if direction == 0:
            curr_i -= 1
        elif direction == 1:
            curr_j -= 1
        else:
            aligned_notes.append((score[curr_j-1][0], curr_i-1))
            curr_i -= 1
            curr_j -= 1
    print "(time_in_score, time_in_audio)"
    print aligned_notes[::-1]

    # worth ploting: chroma_matrix, pitch_matrix, S, P
    plt.imshow(numpy.log10(chroma_matrix+0.000001), interpolation='nearest', aspect='auto')
    plt.show()