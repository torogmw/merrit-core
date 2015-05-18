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
#include "ChromaFeat.h"

class AudioAnalyzer
{
public:
    AudioAnalyzer(uint32_t length);
    ~AudioAnalyzer();
    int Analyze(const float *buffer);
    int numOfFrames;
    ChromaFeat *chromaFeat;
    uint32_t length;
};

#endif /* defined(__merrit_core__AudioAnalyzer__) */
