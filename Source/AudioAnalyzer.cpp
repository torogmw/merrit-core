//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

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
    for (int i=0; i<feature_size; i++) {
        if (temp[i] > temp[i-1] && temp[i] >= temp[i+1]) {
            out[i] = temp[i];
        }
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
    if (flux[0] > 0) {
        struct Note audio_note = {midi_note, subband_signal[0]};
        audio_notes[0.f] = audio_note;
    }
    for (i=1; i<frame_num-1; i++) {
        if (flux[i-1] < flux[i] && flux[i] > flux[i+1]) {
            struct Note audio_note = {midi_note, subband_signal[i]};
            audio_notes[i*hop_size / fs] = audio_note;
        }
    }
    if (flux[frame_num-2] < flux[frame_num-1]) {
        struct Note audio_note = {midi_note, subband_signal[frame_num-1]};
        audio_notes[(frame_num-1)*hop_size /fs] = audio_note;
    }
    
    delete [] flux;
    return 0;
}

int AudioAnalyzer::SetScore(struct Note *score, float *times, uint32_t note_num)
{
    for (int i = 0; i < note_num; ++i) {
        score_notes[times[i]] = score[i];
    }
    return 0;
}

int AudioAnalyzer::Clear()
{
    for (int i=0; i<feature_size; i++) {
        subband_signals[i].clear();
    }
    frame_num = 0;
    score_notes.clear();
    return 0;
}

int AudioAnalyzer::AudioScoreAlignment()
{
    float **S = new float*[audio_notes.size()+1];
    uint32_t **P = new uint32_t*[audio_notes.size()+1];
    int i, j;
    for (i=0; i<audio_notes.size(); i++) {
        S[i] = new float[score_notes.size()+1];
        P[i] = new uint32_t[score_notes.size()+1];
        memset(S[i], 0, (score_notes.size()+1)*sizeof(float));
        memset(S[i], 0, (score_notes.size()+1)*sizeof(float));
    }
    
    float value = 0.f;
    for (i=1; i<audio_notes.size()+1; i++) {
        for (j=1; j<score_notes.size()+1; j++) {
            value = 0.f;
            
        }
    }
    
    for (i=0; i<audio_notes.size(); i++) {
        delete [] S[i];
        delete [] P[i];
    }
    delete [] S;
    delete [] P;
    return 0;
}