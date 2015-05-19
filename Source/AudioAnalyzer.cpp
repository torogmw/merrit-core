//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer(const float *audio_, uint32_t num_samples, float fs_, uint32_t frame_size_, uint32_t hop_size_)
{
    audio = audio_;
    fs = fs_;
    frame_size = frame_size_;
    hop_size = hop_size_;
    num_frames = (num_samples - frame_size) / hop_size + 1;
    chromaFeat = new ChromaFeat(frame_size, fs);
    frame_features = new float*[num_frames];
    frame_feature_dimension = NUMBEROFNOTES;
    
    // MIR - frame-level analysis
    for (int i=0; i<num_frames; i++) {
        frame_features[i] = new float[frame_feature_dimension];
        AnalyzeFrame(audio+i*hop_size, frame_features[i]);
    }
    
    for (int i=0; i<num_frames; i++) {
        float max_val = 0.0;
        int max_idx = -1;
        for (int j=0; j<frame_feature_dimension; j++) {
            if (frame_features[i][j] > max_val) {
                max_val = frame_features[i][j];
                max_idx = j;
            }
        }
        printf("%i\t", max_idx);
    }
    
}

AudioAnalyzer::~AudioAnalyzer()
{
    delete chromaFeat;
    for (int i=0; i<num_frames; i++) {
        delete []frame_features[i];
    }
    delete frame_features;
}

int AudioAnalyzer::AnalyzeFrame(const float *buffer, float* output)
{
    chromaFeat->Chroma(buffer);
    memcpy(output, chromaFeat->chroma, frame_feature_dimension * sizeof(float));
    return 0;
}