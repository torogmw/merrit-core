#ifndef _CHROMA_FEAT
#define _CHROMA_FEAT

#include "rsrfft.h"
#include <cmath>

#define FS				 16000
#define NUMBEROFCHROMES		12
#define NUMBEROFNOTES		120
#define _2ROOT12		1.059463094f
#define FREQREF		7.943049790954413f	// the frequency of (midi = -0.5), which is the left boundary for C0
#define ZERO_PADDING_RATE	0.9f



class ChromaFeat {
public:
	int Chroma(const short* buffer);
	//int Chroma2(const short* buffer);
	ChromaFeat(unsigned long lengthArg);
	~ChromaFeat();
	float *chroma;
	
protected:
	SplitRadixFFT *fft;
	unsigned long length;
	float *hammingWin;
	float *powerOf2ROOT12;
	unsigned long FFT_Point;
	short FFT_Order;
};

#endif