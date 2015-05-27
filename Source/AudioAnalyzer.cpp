//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer(float fs, uint32_t block_size)
{
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

int AudioAnalyzer::Clear()
{
    for (int i=0; i<feature_size; i++) {
        subband_signals[i].clear();
    }
    frame_num = 0;
    return 0;
}