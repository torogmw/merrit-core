import sys
import scipy.io.wavfile as wavfile
from scipy.signal import resample
import matplotlib.pyplot as plt
import numpy
from numpy.fft import rfft

def peak_picking(spectrum, num_peaks, radius=3):
    peaks = []
    for i in range(len(spectrum)):
        if spectrum[i] == 0 or spectrum[i-1] >= spectrum[i] or spectrum[i] <= spectrum[i+1]:
            continue
        j = i - 1
        k = i + 1
        while spectrum[j] < spectrum[j+1] and spectrum[k] < spectrum[k-1]:
            j -= 1
            k += 1
        if k - i > radius:
            bottom_val = min(spectrum[j], spectrum[k])
            peaks.append((i, k-i))
    peaks.sort(reverse=True, key=lambda x: x[1])
    return [r[0] for r in peaks[:num_peaks]]

if __name__ == "__main__":
    [fs, audio] = wavfile.read(open(sys.argv[1], 'r'))
    # stereo to mono
    audio = numpy.average(audio, axis=1)
    
    # normalize
    maxval = max(max(audio), abs(min(audio)))
    audio /= maxval

    # resample
    fs_mir = 11025.0
    audio = resample(audio, audio.size * fs_mir / fs)

    # frame-level analysis
    frame_time = 100.0 # in ms
    hop_time = 50.0
    fft_point = 1024
    zero_padding_rate = 0.9
    min_freq = 80.0 # in Hz
    max_freq = 3000.0 # in Hz

    frame_size = int(frame_time / 1000.0 * fs_mir)
    hop_size = int(hop_time / 1000.0 * fs_mir)
    num_frames = (audio.size - frame_size) / hop_size + 1

    while fft_point < frame_size * (1 + zero_padding_rate):
        fft_point *= 2

    freq_res = fs_mir / fft_point
    min_bin = int(min_freq / freq_res)
    max_bin = int(max_freq / freq_res)
    spectrum_size = fft_point / 2

    pitch_matrix = numpy.zeros((num_frames, spectrum_size))

    for i in range(num_frames):
        # if i != 1:
        #     continue
        spectrum = numpy.ones(spectrum_size) * (-90.)
        fft_result = rfft(audio[i*hop_size:(i+1)*hop_size], fft_point)
        for j in range(spectrum_size):
            if j >= min_bin and j <= max_bin:
                spectrum[j] = numpy.log10(abs(fft_result[j])) * (20.)
        peaks = peak_picking(spectrum, 10)
        for peak in peaks:
            pitch_matrix[i][peak] = 1
        # plt.plot(spectrum)
        # plt.show()

    plt.imshow(pitch_matrix, interpolation='nearest', aspect='auto')
    plt.show()