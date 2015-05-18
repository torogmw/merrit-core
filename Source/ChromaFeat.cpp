#include "ChromaFeat.h"

ChromaFeat::ChromaFeat(unsigned long lengthArg)
{
	unsigned long i;
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
	unsigned long minLength4FFT = (unsigned long) ((1 + ZERO_PADDING_RATE) * length);
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
int ChromaFeat::Chroma(const short* buffer) {
	
	// check if the input arguments are legal
	if (length < 0 || buffer == NULL || (buffer+length-1) == NULL) {
        //std::cout<<"Error: illegal arguments.";
		return -1;
	}
	
	unsigned long i, j, k;

	if (FFT_Point < length)
	{
        //std::cout<<"Error: FFT_Point is larger than frame length.";
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
			unsigned long left = (unsigned long) ceil(indexBoundary[i + j]);
			unsigned long right = (unsigned long) floor(indexBoundary[i + j + 1]);
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


// don't use Chroma2 these days since many details are under heavy construction
/*
// use Goto's idea. It should improve
// i.e. a note has a span of 200 cents, a hanning window tuned at the center
int ChromaFeat::Chroma2(const short* buffer, unsigned long length) {
	// check if the input arguments are legal
	if (length < 0 || buffer == NULL || (buffer+length-1) == NULL) {
		printf("Error: illegal arguments.");
		return -1;
	}
	
	short fs = 16000; // sampling frequency
	
	short FFT_order = 11; // order of FFT (number of butterfly layers)
	short FFT_point = 1 << FFT_order; // 2^11 = 2048
	
	// We don't do 'in-place' FFT so we copy it into a new array first
	// In the meantime, we multiply a hamming window
	float hamming = 6.2831852 / (length - 1);
	float* X = new float[FFT_point];
	for (short i=0; i<length; i++) {
		X[i] = (float) buffer[i] * (0.54 - 0.46 * cos(hamming * i));
	}
	for (short i0=length; i0<FFT_point; i0++) {
		X[i0] = 0;
	}
	SplitRadixFFT fft = SplitRadixFFT(FFT_order);
	fft.XForm(X);
	// Now X is the FFT result. Note that it is stored in this way:
	// [Re(0),Re(1),....,Re(N/2),Im(N/2-1),...Im(1)]
	// So X[k] + j * X[FFT_point - k] represents the kth FFT value
	
	// We set the range of midi notes we want to consider here.
	short NumOfNote = 120; // this is actually the maximum midi note rather than the total number of midi notes we consider
	// because for low frequency the FFT resolution is not high enough so the loop will skip some.
	const float _2ROOT12 = 1.059463094f; // we will use the formula f = (2^(1/12))^n * f_ref
	// to transform midi notes into corresponding frequencies
	// and further, k = f/f_res to transform into FFT indices
	// therefore k = (2^(1/12))^n * f_ref / f_res
	const float _FREQREF = 8.1757989156f; // the frequency of (midi = 0), which is  C0
	const float _PI = 3.1415926f;
	float freqResolution = (float)fs/FFT_point; // f_res mentioned above
	
	const float _LOG_2ROOT12 = 0.057762265046662f; // used in hanning band-pass filter
	float logFREQREFbyfreqRe = log(_FREQREF / freqResolution); // also
	
	// We can safely calculate chroma vector now
	chroma = new float[12];
	
	// the 'i2' loop covers each key, where '0' indicates C, '6' indicates 'F#', etc.
	for (short i2=0; i2<12; i2++) {
		chroma[i2] = 0;
		int count = 0;
		// the 'j2' loop covers each pitch of one key, i.e. C0, C2, ...
		for (short j2=0; j2<NumOfNote; j2=j2+12) {
			
			// so this is how we determine both sides of FFT index using Goto's idea
			// i.e. a note has a span of 200 cents, a hanning window (band-pass filter) tuned at the center
			// i2 + j2 is the note we are working on
			short left = ceil(pow(_2ROOT12, i2 + j2 - 1) * _FREQREF / freqResolution);
			short right = floor(pow(_2ROOT12, i2 + j2 + 1) * _FREQREF / freqResolution);
			float FFT_sum = 0; // to store total amplitude of a note
			for (int k2=left; k2<=right; k2++) {
				// Hanning value of each FFT index in calculated using this derived formula
				// the Hanning window is symmetric on a log-scale
				float hanning = 0.5 * (1 - cos(_PI * ((log(k2) - logFREQREFbyfreqRe) / _LOG_2ROOT12 - i2 - j2 + 1)));
				// sum over all FFT values belonging to one key.
				FFT_sum += sqrt(X[k2] * X[k2] + X[FFT_point-k2] * X[FFT_point-k2]) * hanning;
			}
			
			count = right-left+1;
			if (count >0) 
				chroma[i2] += FFT_sum;
		}
	}
	
	// clean up
	delete []X;
	
	chroma_flag = 1;
	return 0;
}
*/