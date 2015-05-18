//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer(uint32_t length_)
{
    length = length_;
    chromaFeat = new ChromaFeat(length);
}

AudioAnalyzer::~AudioAnalyzer()
{
    delete chromaFeat;
}

int AudioAnalyzer::Analyze(const float *buffer)
{
    chromaFeat->Chroma(buffer);
    for (int i=0; i<NUMBEROFCHROMES; i++) {
        printf("%f\n", chromaFeat->chroma[i]);
    }
    return 0;
}