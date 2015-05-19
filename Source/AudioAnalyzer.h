//
//  AudioAnalyzer.h
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#ifndef __merrit_core__AudioAnalyzer__
#define __merrit_core__AudioAnalyzer__

#include <stdio.h>
#include <stdint.h>
#include "ChromaFeat.h"
#include <iostream>

class AudioAnalyzer
{
public:
    AudioAnalyzer(const float *audio, uint32_t num_samples, float fs, uint32_t frame_size, uint32_t hop_size);
    ~AudioAnalyzer();
    int AnalyzeFrame(const float *buffer, float *output);
    const float *audio;
    float fs;
    uint32_t frame_size;
    uint32_t hop_size;
    uint32_t num_frames;
    ChromaFeat *chromaFeat;
    float **frame_features;
    uint32_t frame_feature_dimension;
};

#endif /* defined(__merrit_core__AudioAnalyzer__) */
