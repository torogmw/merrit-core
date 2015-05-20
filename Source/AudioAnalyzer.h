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
#include <iostream>
#include <aubio/aubio.h>

class AudioAnalyzer
{
public:
    AudioAnalyzer(const float *audio, uint32_t num_samples, float fs, uint32_t frame_size, uint32_t hop_size);
    ~AudioAnalyzer();
    const float *audio;
    float fs;
    uint32_t frame_size;
    uint32_t hop_size;
    uint32_t num_frames;
};

#endif /* defined(__merrit_core__AudioAnalyzer__) */
