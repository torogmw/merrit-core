//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

#define BACKTRACK_I 0
#define BACKTRACK_J 1
#define BACKTRACK_X 2

AudioAnalyzer::AudioAnalyzer(float fs_, uint32_t block_size)
{
    fs = fs_;
    frame_size = (uint32_t)(FRAME_TIME / 1000.0 * fs);
    hop_size = block_size;
    if (frame_size < hop_size) {
        printf("hop size does not look right!\n");
    }
    
    fft_point = MIN_FFT_POINT;
    uint32_t fft_order = MIN_FFT_ORDER;
    while (fft_point < frame_size * (1+ZERO_PADDING_RATE)) {
        fft_point *= 2;
        fft_order ++;
    }
    fft = new SplitRadixFFT(fft_order);
    
    float freq_res = fs / fft_point;
    min_bin = (uint32_t)(MIN_FREQ / freq_res);
    max_bin = (uint32_t)(MAX_FREQ / freq_res);
    spectrum_size = fft_point / 2;
    
    FFT_bin_2_MIDI_note_mapping = new uint32_t[spectrum_size];
    FFT_bin_2_MIDI_note_mapping[0] = 0;
    for (int i=1; i<spectrum_size; i++) {
        FFT_bin_2_MIDI_note_mapping[i] = (uint32_t)(log(i * freq_res / FREQREF) / LOG_2ROOT12);
    }
    min_note = FFT_bin_2_MIDI_note_mapping[min_bin];
    max_note = FFT_bin_2_MIDI_note_mapping[max_bin];
    
    for (int i=0; i<NUM_NOTES; i++) {
        MIDI_note_width[i] = pow(_2ROOT12, i);
    }
    
    hammingWin = new float[frame_size];
    float hammingCoeff = (float) 6.2831852 / (frame_size - 1);
    for (int i=0; i<frame_size; i++) {
        hammingWin[i] = (float) (0.54 - 0.46 * cos(hammingCoeff * i));
    }
    
    feature_size = max_note - min_note + 1;
    
    frame_buffer = new float[frame_size];
    subband_signals = new std::vector<float>[feature_size];
    frame_num = 0;
}

AudioAnalyzer::~AudioAnalyzer()
{
    delete [] FFT_bin_2_MIDI_note_mapping;
    delete [] hammingWin;
    delete fft;
    delete [] frame_buffer;
    delete [] subband_signals;
}

int AudioAnalyzer::UpdateFrameBuffer(const float *new_buffer, uint32_t buffer_size)
{
    if (buffer_size != hop_size) {
        return -1;
    }
    for (int i=0; i<frame_size-hop_size; i++) {
        frame_buffer[i] = frame_buffer[i+hop_size];
    }
    for (int i=0; i<hop_size; i++) {
        frame_buffer[frame_size-hop_size+i] = new_buffer[i];
    }
    return 0;
}

int AudioAnalyzer::FrameAnalysis(const float *buffer)
{
    if (buffer == NULL || (buffer+frame_size-1) == NULL) {
        printf("Error: buffer corrupted.");
        return -1;
    }
    float *frame_feature = new float[feature_size];
    int ret = FrameAnalysis(buffer, frame_feature);
    for (int i=0; i<feature_size; i++) {
        subband_signals[i].push_back(frame_feature[i]);
    }
    frame_num++;
    delete [] frame_feature;
    return ret;
}

int AudioAnalyzer::FrameAnalysis(const float *buffer, float *out)
{
    float* X = new float[fft_point];
    for (int i=0; i<frame_size; i++)
    {
        X[i] = (float) buffer[i] * hammingWin[i];
    }
    
    for (int i=frame_size; i<fft_point; i++)
    {
        X[i] = 0;
    }
    
    fft->XForm(X);
    
    float *temp = new float[feature_size];
    memset(temp, 0, feature_size*sizeof(float));
    memset(out, 0, feature_size*sizeof(float));
    
    // chroma - sum of energy in a note
    for (int i=min_bin; i<=max_bin; i++) {
        temp[FFT_bin_2_MIDI_note_mapping[i]-min_note] += (float) (sqrt(X[i] * X[i] + X[fft_point-i] * X[fft_point-i]));
    }
    
    // take into account width
    for (int i=min_note; i<=max_note; i++) {
        temp[i-min_note] /= MIDI_note_width[i];
    }
    
    // only keep those local maximum
    if (temp[0] > temp[1]) {
        out[0] = temp[0];
    }
    for (int i=1; i<feature_size-1; i++) {
        if (temp[i] > temp[i-1] && temp[i] >= temp[i+1]) {
            out[i] = temp[i];
        }
    }
    if (temp[feature_size-1] > temp[feature_size-2]) {
        out[feature_size-1] = temp[feature_size-1];
    }
    
    delete [] temp;
    delete [] X;
    return 0;
}

int AudioAnalyzer::SubbandAnalysis(std::vector<float> &subband_signal, uint32_t midi_note)
{
    if (frame_num < SPECTRAL_FLUX_SIZE) {
        printf("spectral flux size too small!\n");
    }
    
    float *flux = new float[frame_num];
    memset(flux, 0, sizeof(float)*frame_num);
    uint32_t i, j;
    float energy_ahead = 0.f, energy_behind = 0.f;
    for (i=0; i<SPECTRAL_FLUX_SIZE; i++) {
        if (subband_signal[i] > 0) {
            energy_ahead = 0.f;
            for (j=i; j<i+SPECTRAL_FLUX_SIZE; j++) {
                energy_ahead += subband_signal[j];
            }
            energy_ahead /= (SPECTRAL_FLUX_SIZE-i);
            energy_behind = 0.f;
            for (j=0; j<i; j++) {
                energy_behind += subband_signal[j];
            }
            energy_behind = i > 0? energy_behind / i : 0.f;
            flux[i] = energy_ahead > energy_behind ? energy_ahead - energy_behind : 0.f;
        }
    }
    
    for (i=SPECTRAL_FLUX_SIZE; i<frame_num-SPECTRAL_FLUX_SIZE+1; i++) {
        if (subband_signal[i] > 0) {
            energy_ahead = 0.f;
            for (j=i; j<i+SPECTRAL_FLUX_SIZE; j++) {
                energy_ahead += subband_signal[j];
            }
            energy_ahead /= SPECTRAL_FLUX_SIZE;
            energy_behind = 0.f;
            for (j=i-SPECTRAL_FLUX_SIZE; j<i; j++) {
                energy_behind += subband_signal[j];
            }
            energy_behind /= SPECTRAL_FLUX_SIZE;
            flux[i] = energy_ahead > energy_behind ? energy_ahead - energy_behind : 0.f;
        }
    }
    
    for (i=frame_num-SPECTRAL_FLUX_SIZE+1; i<frame_num; i++) {
        if (subband_signal[i] > 0) {
            energy_ahead = 0.f;
            for (j=i; j<frame_num; j++) {
                energy_ahead += subband_signal[j];
            }
            energy_ahead /= (frame_num - i);
            energy_behind = 0.f;
            for (j=i-SPECTRAL_FLUX_SIZE; j<i; j++) {
                energy_behind += subband_signal[j];
            }
            energy_behind /= SPECTRAL_FLUX_SIZE;
            flux[i] = energy_ahead > energy_behind ? energy_ahead - energy_behind : 0.f;
        }
    }
    
    // find local peaks in flux
    if (flux[0] > flux[1]) {
        struct Note audio_note = {midi_note, flux[0]};
        audio_notes[0.f].push_back(audio_note);
    }
    for (i=1; i<frame_num-1; i++) {
        if (flux[i-1] < flux[i] && flux[i] > flux[i+1]) {
            struct Note audio_note = {midi_note, flux[i]};
            audio_notes[i*hop_size / fs].push_back(audio_note);
        }
    }
    if (flux[frame_num-2] < flux[frame_num-1]) {
        struct Note audio_note = {midi_note, flux[frame_num-1]};
        audio_notes[(frame_num-1)*hop_size /fs].push_back(audio_note);
    }
    
//    for (i=0; i<frame_num; i++) {
//        printf("%f,", flux[i]);
//    }
//    printf("\n");
    
    delete [] flux;
    return 0;
}

int AudioAnalyzer::SetScore(std::vector<struct Note> notes, std::vector<float> times)
{
    score_notes.clear();
    for (int i = 0; i < notes.size(); ++i) {
        score_notes[times[i]].push_back(notes[i]);
    }
    return 0;
}

int AudioAnalyzer::Clear()
{
    audio_notes.clear();
    for (int i=0; i<feature_size; i++) {
        subband_signals[i].clear();
    }
    frame_num = 0;
    return 0;
}

float AudioAnalyzer::AudioScoreAlignment()
{
    int i, j;
    TimedNotes::iterator it, jt;
    
    std::map<int, float> audio_notes_index_to_time;
    std::map<int, float> score_notes_index_to_time;
    for (i=0, it=audio_notes.begin(); i<audio_notes.size(); i++, it++) {
        audio_notes_index_to_time[i] = it->first;
    }
    for (j=0, jt=score_notes.begin(); j<score_notes.size(); j++, jt++) {
        score_notes_index_to_time[j] = jt->first;
    }
    
    float **S = new float*[audio_notes.size()+1];
    uint32_t **P = new uint32_t*[audio_notes.size()+1];
    for (i=0; i<audio_notes.size()+1; i++) {
        S[i] = new float[score_notes.size()+1];
        P[i] = new uint32_t[score_notes.size()+1];
        memset(S[i], 0, (score_notes.size()+1)*sizeof(float));
        memset(P[i], 0, (score_notes.size()+1)*sizeof(float));
    }
    
    float value = 0.f;
    
    NotesAtTime::iterator itt, jtt;
    for (i=1, it=audio_notes.begin(); it!=audio_notes.end(); it++, i++) {
        for (j=1, jt=score_notes.begin(); jt!=score_notes.end(); jt++, j++) {
            value = 0.f;
            for (itt=it->second.begin(); itt!=it->second.end(); itt++) {
                for (jtt=jt->second.begin(); jtt!=jt->second.end(); jtt++) {
                    if (itt->midi_pitch == jtt->midi_pitch) {
                        value += itt->valence;
                    }
                }
            }
            value += S[i-1][j-1];
            if (value > S[i-1][j] && value > S[i][j-1]) {
                S[i][j] = value;
                P[i][j] = BACKTRACK_X;
            }
            else if (S[i-1][j] > S[i][j-1]) {
                S[i][j] = S[i-1][j];
                P[i][j] = BACKTRACK_I;
            }
            else {
                S[i][j] = S[i][j-1];
                P[i][j] = BACKTRACK_J;
            }
        }
    }
    
    uint32_t curr_i = audio_notes.size();
    uint32_t curr_j = score_notes.size();
    std::vector<float> backtracked_is;
    std::vector<float> backtracked_js;
    while (curr_i > 0 && curr_j > 0) {
        if (P[curr_i][curr_j] == BACKTRACK_I) {
            curr_i --;
        }
        else if (P[curr_i][curr_j] == BACKTRACK_J) {
            curr_j --;
        }
        else {
            backtracked_is.push_back(audio_notes_index_to_time[curr_i-1]);
            backtracked_js.push_back(score_notes_index_to_time[curr_j-1]);
            curr_i --;
            curr_j --;
        }
    }
    
    /*
     this is correct alignment for first_bar_44100.wav
     2.194286,1.000000
     1.764717,0.833333
     1.323537,0.666667
     0.928798,0.500000
     0.487619,0.333333
     0.046440,0.166667
     */
    
    // linear regression
    int n = backtracked_is.size();
    float x_bar = 0.;
    float y_bar = 0.;
    float sum_x_y = 0.;
    float sum_x_x = 0.;
    for (i=0; i<n; i++) {
        x_bar += backtracked_is[i];
        y_bar += backtracked_js[i];
        sum_x_y += (backtracked_is[i] * backtracked_js[i]);
        sum_x_x += (backtracked_is[i] * backtracked_is[i]);
    }
    x_bar /= n;
    y_bar /= n;

    float slope = (sum_x_y - n * x_bar * y_bar) / (sum_x_x - n * x_bar * x_bar);
    float intercept = y_bar - slope * x_bar;
    
    // normalize audio time and calculate play grade
    float grade = 0.;
    for (i=0; i<n; i++) {
        float diff = slope * backtracked_is[i] + intercept - backtracked_js[i];
//        printf("%d\t%f\t%f\t%f\n", i, diff, backtracked_is[i], backtracked_js[i]);
        if (diff < 0) diff = -diff;
        grade += (diff > 0.125 ? 0 : cos(12.566*diff));
    }
    grade /= score_notes.size();
    
    for (i=0; i<audio_notes.size(); i++) {
        delete [] S[i];
        delete [] P[i];
    }
    delete [] S;
    delete [] P;
    return grade;
}