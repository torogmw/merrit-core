#include "ChromaFeat.h"

ChromaFeat::ChromaFeat(uint32_t lengthArg)
{
	uint32_t i;
	length = lengthArg;
	chroma = new float[NUMBEROFCHROMES];

	// We only calculate these slow math once
	hammingWin = new float[length];
	powerOf2ROOT12 = new float[NUMBEROFNOTES];
	float hammingCoeff = (float) 6.2831852 / (length - 1);
	for (i=0; i<length; i++) {
		hammingWin[i] = (float) (0.54 - 0.46 * cos(hammingCoeff * i));
	}
	for (i=0; i<NUMBEROFNOTES; i++)
	{
		powerOf2ROOT12[i] = (float) pow(_2ROOT12, i);
	}

	
	// compute necessary FFT number
	FFT_Point = 1;
	FFT_Order = 0;
	uint32_t minLength4FFT = (uint32_t) ((1 + ZERO_PADDING_RATE) * length);
	while (FFT_Point < minLength4FFT)
	{
		FFT_Point = FFT_Point << 1;
		FFT_Order ++;
	}
	fft = new SplitRadixFFT(FFT_Order);
}

ChromaFeat::~ChromaFeat() 
{
	delete []chroma;
	delete []hammingWin;
	delete []powerOf2ROOT12;
	delete fft;
}

// Calculate 12-dimensional chroma vector
// I presume the sampling frequency is 16kHz
int ChromaFeat::Chroma(const uint16_t* buffer) {
	
	// check if the input arguments are legal
	if (length == 0 || buffer == NULL || (buffer+length-1) == NULL) {
		printf("Error: illegal arguments.");
		return -1;
	}
	
	uint32_t i, j, k;

	if (FFT_Point < length)
	{
		printf("Error: FFT_Point is larger than frame length.");
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
	
	// we will use the formula f = (2^(1/12))^n * f_ref
	// to transform midi notes into corresponding frequencies
	// and further, k = f/delta_f to transform into FFT indices
	float* indexBoundary = new float[NUMBEROFNOTES + 1]; 
	float freqResolution = (float)FS / FFT_Point;

	// transformation from midi note to FFT index
	for (i=0; i<=NUMBEROFNOTES; i++) 
	{
		indexBoundary[i] = (float) (powerOf2ROOT12[i] * FREQREF / freqResolution);
	}
	
	// We can safely calculate chroma vector now
	
	
	// the 'i' loop covers each key, where '0' indicates C, '6' indicates 'F#', etc.
	for (i=0; i<NUMBEROFCHROMES; i++) 
	{
		chroma[i] = 0;
		
		// the 'j' loop covers each pitch of one key, i.e. C0, C2, ...
		for (j=0; j<NUMBEROFNOTES; j=j+NUMBEROFCHROMES) 
		{
			
			// so this is how we determine both sides of FFT index
			uint32_t left = (uint32_t) ceil(indexBoundary[i + j]);
			uint32_t right = (uint32_t) floor(indexBoundary[i + j + 1]);
			float FFT_sum = 0;

			// the 'k' loop sums over all FFT values belonging to one key.
			for (k=left; k<=right; k++) 
			{
				FFT_sum += (float) (sqrt(X[k] * X[k] + X[FFT_Point-k] * X[FFT_Point-k]));
			}

			chroma[i] += FFT_sum;
		}
	}
	
	// clean up
	delete []X;
	delete []indexBoundary;
	
	return 0;
}