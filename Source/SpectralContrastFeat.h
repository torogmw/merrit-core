#ifndef _SPECTRALCONTRAST_FEAT
#define _SPECTRALCONTRAST_FEAT

#include "rsrfft.h"
#include <cmath>


#define SAMPLERATE					16000
#define DIMENSION_OF_RSC			12
#define ZERO_PADDING_RATE			0.9f
#define NEIGHBORHOOD_ALPHA			0.02f
#define SUBBAND_NUM					DIMENSION_OF_RSC/2

class SpectralContrastFeat
{
public:
	SpectralContrastFeat(unsigned long lengthArg);
	~SpectralContrastFeat();
	int RawSpectralContrast(short *buffer);

	float *rawSpectralContrast; // the result is in [SC(1) Valley(1) SC(2) Valley(2) ...] order

protected:

	SplitRadixFFT *fft;
	unsigned long length;
	float *hammingWin;
	unsigned long FFT_Point;
	short FFT_Order;
	unsigned long *boundaryTable;
	unsigned long *subbandSize;

};



#endif