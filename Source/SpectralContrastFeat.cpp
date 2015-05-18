#include "SpectralContrastFeat.h"
#include <stdlib.h>

int fcmp(const void *a, const void *b)// for qsort
{
	return *(float *)a > *(float *)b ? 1: -1;
}

SpectralContrastFeat::SpectralContrastFeat(unsigned long lengthArg)
{
	unsigned long i;
	length = lengthArg;
	rawSpectralContrast = new float[DIMENSION_OF_RSC];
	
	// We only calculate this slow math once
	hammingWin = new float[length];
	float hammingCoeff = (float) 6.2831852 / (length - 1);
	for (i=0; i<length; i++) {
		hammingWin[i] = (float) (0.54 - 0.46 * cos(hammingCoeff * i));
	}

	
	// compute necessary FFT number
	FFT_Point = 1;
	FFT_Order = 0;
	unsigned long minLength4FFT = (unsigned long) ((1 + ZERO_PADDING_RATE) * length);
	while (FFT_Point < minLength4FFT)
	{
		FFT_Point = FFT_Point << 1;
		FFT_Order ++;
	}
	fft = new SplitRadixFFT(FFT_Order);

	// compute the FFT index at the beginning of each subband and corresponding width
	boundaryTable = new unsigned long[SUBBAND_NUM];
	subbandSize = new unsigned long[SUBBAND_NUM];
	float boundaryFreqTable[SUBBAND_NUM] = {0, 200, 400, 800, 1600, 3200};

	for (i=0; i<SUBBAND_NUM; i++)
	{
		boundaryTable[i] = (unsigned long) ceil(boundaryFreqTable[i] / SAMPLERATE * FFT_Point);
	}
	
	for (i=0; i<SUBBAND_NUM-1; i++)
	{
		subbandSize[i] = boundaryTable[i+1] - boundaryTable[i];
	}
	subbandSize[SUBBAND_NUM-1] = (unsigned long)FFT_Point/2 - boundaryTable[SUBBAND_NUM-1];

}

SpectralContrastFeat::~SpectralContrastFeat()
{
	delete []rawSpectralContrast;
	delete []hammingWin;
	delete []boundaryTable;
	delete []subbandSize;
	delete fft;
}

int SpectralContrastFeat::RawSpectralContrast(short *buffer)
{

	// check if the input arguments are legal
	if (length < 0 || buffer == NULL || (buffer+length-1) == NULL) {
		//printf("Error: illegal arguments.");
		return -1;
	}
	
	unsigned long i, j;
	
	if (FFT_Point < length)
	{
		//printf("Error: FFT_Point is larger than frame length.");
		return -1;
	}
	
	// We don't do 'in-place' FFT so we copy it into a new array first
	// In the meantime, we multiply a hamming window
	
	float* X = new float[FFT_Point];
	for (i=0; i<length; i++) 
	{
		X[i] = (float) buffer[i] * hammingWin[i];
	}
	
	for (i=length; i<FFT_Point; i++) 
	{
		X[i] = 0;
	}
	
	fft->XForm(X);

	// amplitude spectrum
	X[0] = (float) sqrt(X[0]*X[0]);
	for (i=1; i<FFT_Point/2; i++)
	{
		X[i] = (float) sqrt(X[i]*X[i]+X[FFT_Point-i]*X[FFT_Point-i]);
	}

	// pointer to each subband
	float **subband_pointer = new float* [SUBBAND_NUM];
	for (i=0; i<SUBBAND_NUM; i++)
	{
		subband_pointer[i] = &X[boundaryTable[i]];

		// sort each subband in ascending order
		qsort(subband_pointer[i], subbandSize[i], sizeof(float), fcmp);

		// compute peak and valley
		float numFFT_Considered = subbandSize[i]*NEIGHBORHOOD_ALPHA;
		float peak = 0;
		for (j=0; j<numFFT_Considered; j++)
		{
			peak += *(subband_pointer[i]+subbandSize[i]-1-j);
			//printf("For subband [%u] Peak add [%f]\n", i, *(subband_pointer[i]+subbandSize[i]-1-j));
		}
		float valley = 0;

		for (j=0; j<numFFT_Considered; j++)
		{
			valley += *(subband_pointer[i]+j);
			//printf("For subband [%u] Valley add [%f]\n", i, *(subband_pointer[i]+j));
		}
		rawSpectralContrast[2*i] = (float) log(peak/numFFT_Considered);
		rawSpectralContrast[2*i+1] = (float) log(valley/numFFT_Considered);
		rawSpectralContrast[2*i] = rawSpectralContrast[2*i] - rawSpectralContrast[2*i+1];
	}

	delete []subband_pointer;
	delete []X;
	return 0;

}
