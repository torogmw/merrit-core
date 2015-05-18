#ifndef _CHROMA_FEAT
#define _CHROMA_FEAT

#include "rsrfft.h"
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>

#define FS				 16000
#define NUMBEROFCHROMES		12
#define NUMBEROFNOTES		120
#define _2ROOT12		1.059463094f
#define FREQREF		7.943049790954413f	// the frequency of (midi = -0.5), which is the left boundary for C0
#define ZERO_PADDING_RATE	0.9f



class ChromaFeat {
public:
	int Chroma(const float* buffer);
	//int Chroma2(const uint16_t* buffer);
	ChromaFeat(uint32_t lengthArg);
	~ChromaFeat();
	float *chroma;
	
protected:
	SplitRadixFFT *fft;
	uint32_t length;
	float *hammingWin;
	float *powerOf2ROOT12;
	uint32_t FFT_Point;
	uint16_t FFT_Order;
};

#endif