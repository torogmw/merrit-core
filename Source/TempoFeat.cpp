#include "TempoFeat.h"



TempoFeat::TempoFeat() 
{
	onsetRate = -1;
	tempoBPM = -1;
	
	int i;
	int len = (int) (SAMPLERATE*FrameTime);
	hamWin = new float[len];
	hamWin4Pitch = new float[Frame_Len];
	float tmp = (float) (PI*2.0F/(float)(len-1));
	float tmp2 = (float) (PI*2.0F/(float)(Frame_Len-1));
	for (i=0; i<len; i++)
	{
		hamWin[i] = (float)(0.54-0.46*cos(tmp*i));
	}
	for (i=0; i<Frame_Len; i++)
	{
		hamWin4Pitch[i] = (float)(0.54-0.46*cos(tmp2*i));
	}
	fft = new SplitRadixFFT(FFTINDEX);
}

TempoFeat::~TempoFeat()
{
	delete []hamWin;
	delete []hamWin4Pitch;
	delete fft;
}

// I presume the sampling rate is 16kHz
int TempoFeat::OnsetRate(short* buffer, unsigned long length)
{
	float *pPeakPos = new float[300];
	float *pPeakConf = new float[300];
	int nPeakNum = 0;
	if(Onset_Detect(buffer, length, 16000, pPeakPos, pPeakConf, nPeakNum)==-1)
	{
		printf("Onset Detection fails!\n");
		return -1;
	}

	delete []pPeakPos;
	delete []pPeakConf;
	float durationinMinute = (float) length / 16000 / 60;
	onsetRate = (float) nPeakNum / (float) durationinMinute;
	return 0;
}

int TempoFeat::TempoBPM(short* buffer, unsigned long length)
{
	if (length < ((Pitch_Buf_Size-2)*128+512)*128)
	{
		printf("The length of song does not reach the minimum value %u\n", ((Pitch_Buf_Size-2)*128+512)*128);
		return -1;
	}
	else 
	{
		BPM_Estimate(buffer, length, tempoBPM);
		return 0;
	}
	
}

// **** onset.cpp **** //

//
//  detection function via spectral-flux 
//
void TempoFeat::Detect_Func_log(short* pRawData, int nDataLen, int nSampleRate, 
				     float* &DF, int &idxNum)
{
	// down-sample the incoming data to 16KHz SAMPLERATE
	// 
	if (nSampleRate != 16000)
	{
		printf("WAV format doesn't match 16k, mono, 16b\n"); 
		return;
	} 

	int i,j,k;
	long Length = nDataLen;
	float re[FFTSIZE];
    int nFrameLen = (int) (SAMPLERATE*FrameTime);  // get the data length of frame (nFrameLen = 256)
	int SPLITFRAMENUM = Length/nFrameLen;  // SPLITFRAMENUM = nDataLen/FFTSIZE
	int INDEXNUM = SPLITFRAMENUM*2-1;      // half frame-length overlap

	// **** INDEXNUM is the number of frames with length of 256

    float *pSubBandEn[SUBBANDNUM];
	for(i=0; i<SUBBANDNUM; i++) 
	{
		pSubBandEn[i] = new float[INDEXNUM];
		
	    if(pSubBandEn[i] == NULL)
		{
			return;
		}
	}
   

	// pData is divided into SPLITFRMAENUM frames (50% overlap)
	// calculate the energy of each sub-band of every frame
	// 
	for (i=0; i<INDEXNUM; i++)
	{  	
		for(k=0; k<nFrameLen; k++)
		{
			re[k] = (float) (pRawData[k+i*nFrameLen/2]/32767.0); // normalization
		}
		for(k=nFrameLen; k<FFTSIZE; k++)
		{ 
			re[k] = 0; 
		}
			
		HanningWindow(re, nFrameLen); // Hanning Window
		
		fft->XForm(re);
		re[0] = re[0]*re[0];
		for(k=1; k<FFTSIZE/2; k++)	
		{
			re[k] = (float) sqrt(re[k]*re[k]+re[FFTSIZE-k]*re[FFTSIZE-k]); // sqrt for module
		}

		for(j=0; j<SUBBANDNUM; j++) 
		{
			pSubBandEn[j][i] = 0; // initialize all freq-channels
		}

		for(k=0; k<SUBBANDNUM; k++)
		{
			pSubBandEn[k][i] = re[k]; // discard the DC value?
		}
	}


	// calculate the envelope of each sub-band
	// 
	float *pEnvelope[SUBBANDNUM];
	float m_pWindow[HalfHannLen]; 
	int m_nHanningLength = HalfHanning_Time(m_pWindow, HAMLEN);
	for (i=0; i<SUBBANDNUM; i++) 
	{
		pEnvelope[i] = new float[INDEXNUM];

		if (pEnvelope[i]==NULL ||
			Convolve(pSubBandEn[i], INDEXNUM, m_pWindow, m_nHanningLength, pEnvelope[i])<0)
		{
            for (j=0; j <SUBBANDNUM; j++)
            {
                SAFE_DELETE(pSubBandEn[j]);
            }
            for (j=0; j <=i; j++)
            {
				SAFE_DELETE(pEnvelope[j]);
            }
			return;
		}
	}


#if 0
 	// calculate the log-compression of each sub-band
	//
	for (i=0; i<SUBBANDNUM; i++)
	{	
		logComp(pEnvelope[i], INDEXNUM);
	}
#endif


	// calculate the difference of each sub-band
	// 
	float *pDifference[SUBBANDNUM];
	for (i=0; i<SUBBANDNUM; i++) 
	{
		pDifference[i] = new float[INDEXNUM];

		if (pDifference[i]==NULL ||
			Canny_Estimate(pEnvelope[i], pDifference[i], INDEXNUM, CANNYLEN)<0)
		{
			for (j=0; j<SUBBANDNUM; j++)
			{
				SAFE_DELETE(pSubBandEn[j]);
				SAFE_DELETE(pEnvelope[j]);
			}
			for (j=0; j<=i; j++)
			{
				SAFE_DELETE(pDifference[j]);
			}
			return;
		}	
	}
	
	
	// calculate the HWR of each sub-band
	// 
	for (i=0; i<SUBBANDNUM; i++) 
	{
		Rectify(pDifference[i], INDEXNUM);
	}


	// calculate the sum of differences across all channels
    //
    float *pDifferenceSum = new float[INDEXNUM];

    if (pDifferenceSum == NULL)
    {
        for (i=0; i<SUBBANDNUM; i++)
        {
         	SAFE_DELETE(pSubBandEn[i]);
			SAFE_DELETE(pEnvelope[i]);
			SAFE_DELETE(pDifference[i]);
        }
        return;
    }

	for (i=0; i<INDEXNUM; i++)
	{
		pDifferenceSum[i] = 0;

        for (j=0; j<SUBBANDNUM; j++)
        {
			pDifferenceSum[i] += pDifference[j][i]; // discard DC value? (FFT-bin0)
        }
	}


	// release memory
	//
	for (i=0; i<SUBBANDNUM; i++)
	{
		SAFE_DELETE(pSubBandEn[i]);
        SAFE_DELETE(pEnvelope[i]);
		SAFE_DELETE(pDifference[i]);
	}    
	
	
	// return value
	idxNum = INDEXNUM;
	DF = pDifferenceSum;
}


//
// onset detection by spectral-flux 
//
int TempoFeat::Onset_Detect(short *pRawData, unsigned long nDataLen, int nSampleRate,  
				 float *pPeakPos, float *pPeakConf, int &nPeakNum)
{
	// down-sample the incoming data to 16KHz SAMPLERATE
	// 
	if (nSampleRate != 16000)
	{
		printf("WAV format doesn't match 16k, mono, 16b\n"); 
		return -1;
	} 

	int i,j,k;
	unsigned long Length = nDataLen;

	float re[FFTSIZE];
    int nFrameLen = (int) (SAMPLERATE*FrameTime);  // get the data length of frame (nFrameLen = 256)
	int SPLITFRAMENUM = Length/nFrameLen;  // SPLITFRAMENUM = nDataLen/FFTSIZE
	int INDEXNUM = SPLITFRAMENUM*2-1;      // half frame-length overlap
    float *pSubBandEn[SUBBANDNUM];
	for(i=0; i<SUBBANDNUM; i++) 
	{ 
		pSubBandEn[i] = new float[INDEXNUM];
		
	    if(pSubBandEn[i] == NULL)
		{
			return -1;
		}
	}


	// pData is divided into SPLITFRMAENUM frames (50% overlap)
	// calculate the energy of each sub-band of every frame
	// 
	for (i=0; i<INDEXNUM; i++)
	{
		for(k=0; k<nFrameLen; k++)
		{
			re[k] = (float) (pRawData[k+i*nFrameLen/2]/32767.0); // normalization
		}
		for(k=nFrameLen; k<FFTSIZE; k++)
		{ 
			re[k] = 0; 
		}
		
		HanningWindow(re, nFrameLen); // Hanning Window

		fft->XForm(re);
		re[0] = re[0]*re[0];
		for(k=1; k<FFTSIZE/2; k++)	
		{
			re[k] = (float) sqrt(re[k]*re[k]+re[FFTSIZE-k]*re[FFTSIZE-k]); // sqrt for module
		}



		for(j=0; j<SUBBANDNUM; j++) 
		{
			pSubBandEn[j][i] = 0; // initialize all freq-channels
		}

		for(k=0; k<SUBBANDNUM; k++)
		{
			pSubBandEn[k][i] = re[k]; // discard the DC value?
 
		}

	}
 

	// calculate the envelope of each sub-band
	// 
	float *pEnvelope[SUBBANDNUM];
	float m_pWindow[HalfHannLen]; 
	int m_nHanningLength = HalfHanning_Time(m_pWindow, HAMLEN);
	for (i=0; i<SUBBANDNUM; i++) 
	{
		pEnvelope[i] = new float[INDEXNUM];

		if (pEnvelope[i]==NULL ||
			Convolve(pSubBandEn[i], INDEXNUM, m_pWindow, m_nHanningLength, pEnvelope[i])<0)
		{// **** If getting here, it means something wrong
            for (j=0; j <SUBBANDNUM; j++)
            {
                SAFE_DELETE(pSubBandEn[j]);
            }
            for (j=0; j <=i; j++)
            {
				SAFE_DELETE(pEnvelope[j]);
            }
			return -1;
		}
	}
 

	// calculate the log-compression of each sub-band
	//
	for (i=0; i<SUBBANDNUM; i++)
	{			

		logComp(pEnvelope[i], INDEXNUM);

	}


	// calculate the difference of each sub-band
	// 
	float *pDifference[SUBBANDNUM];
	for (i=0; i<SUBBANDNUM; i++) 
	{
		pDifference[i] = new float[INDEXNUM];

		if (pDifference[i]==NULL ||
			Canny_Estimate(pEnvelope[i], pDifference[i], INDEXNUM, CANNYLEN)<0)
		{
			for (j=0; j<SUBBANDNUM; j++)
			{
				SAFE_DELETE(pSubBandEn[j]);
				SAFE_DELETE(pEnvelope[j]);
			}
			for (j=0; j<=i; j++)
			{
				SAFE_DELETE(pDifference[j]);
			}
			return -1;
		}	
	}
	
	
	// calculate the HWR of each sub-band
	// 
	for (i=0; i<SUBBANDNUM; i++) 
	{
		Rectify(pDifference[i], INDEXNUM);
	}



	// calculate the sum of differences across all channels
    //
    float *pDifferenceSum = new float[INDEXNUM];

    if (pDifferenceSum == NULL)
    {
        for (i=0; i<SUBBANDNUM; i++)
        {
         	SAFE_DELETE(pSubBandEn[i]);
			SAFE_DELETE(pEnvelope[i]);
			SAFE_DELETE(pDifference[i]);
        }
        return -1;
    }

	for (i=0; i<INDEXNUM; i++)
	{
		pDifferenceSum[i] = 0;

        for (j=0; j<SUBBANDNUM; j++)
        {
			pDifferenceSum[i] += pDifference[j][i]; // discard DC value? (FFT-bin0)
        }
	}

	// thresholding and peak-picking of the detection function
    //
	Peak_Select(pDifferenceSum, INDEXNUM, pPeakPos, pPeakConf, nPeakNum);

	
	// release memory
	//
	for (i=0; i<SUBBANDNUM; i++)
	{
		SAFE_DELETE(pSubBandEn[i]);
        SAFE_DELETE(pEnvelope[i]);
		SAFE_DELETE(pDifference[i]);
	}
	SAFE_DELETE(pDifferenceSum);

    return 0;
}


//
// Calculate the convolve result
//
int TempoFeat::Convolve(float pX1[], int Length1, float pX2[], int Length2, float pY[])
{
	int i, j;
    float *pInverseX2 = new float[Length2];

    if (pInverseX2 == NULL)
    {
        return -1;
    }

    // Inverse the data
    //
    for (i = 0; i < Length2; i++)
	{
		pInverseX2[i] = pX2[Length2-i-1];
	}

	// Convolve the data
    //
    for (i = 0; i < Length1; i++)
	{
		pY[i] = 0;
		for (j = 0; j < Length2; j++)
		{
			if (i + j - Length2 + 1 >= 0)
			{
				pY[i] += pX1[i+j-Length2+1] * pInverseX2[j];
			}
		}
	}
	
    delete pInverseX2;

    return 0;
}


//
// Calculate the halfhanning
//
int TempoFeat::HalfHanning_Time(float pWindow[], int time)
{
	int Length = (int)(time/(1000.0*FrameTime/2));
	int i;

	for (i=0; i<Length; i++)
	{
		pWindow[i] = (float) (0.5-0.5*cos(2*PI*i/(Length*2-1)));
	}

	return(Length);
}


//
// Calculate the differences
//
int TempoFeat::Canny_Estimate(float pX[], float pY[], int Length, float Sigma)
{
	// get the length of canny
    //
    int CannyHalfLength = int(Sigma*2+0.5);
	int CannyLength = CannyHalfLength*2;
	float *pCanny = new float[CannyLength+1];

    if(pCanny == NULL)
    {
        return -1;
    }

	// calculate the shape of canny
    //
    int i, j;

	for (i=-CannyHalfLength; i<0; i++)
	{
		pCanny[i+CannyHalfLength] = (float) (i/(Sigma*Sigma)*exp(-(i*i)/(2*Sigma*Sigma)));
	}
	pCanny[CannyHalfLength] = 0;
	for (i=0; i<CannyHalfLength; i++)
	{
		pCanny[i+CannyHalfLength+1] = -pCanny[i];
	}

    // convolve the pX with pCanny
    //
	for (i=0; i<Length; i++)
	{
		pY[i] = 0; 
		for(j=-CannyHalfLength; j<=CannyHalfLength; j++)
		{
			if(i+j>=0 && i+j<Length)
			{
				pY[i] += pX[i+j]*pCanny[j+CannyHalfLength];
			}
		}                   
	}

	delete pCanny;

    return 0;
}


//
// Select the peaks
//
void TempoFeat::Peak_Select(float pX[], int Length, float *pPeakPos, float *pPeakConf, int &nPeakNum)
{
    int *PrePeakPos = new int [Length/2];
	float *PrePeakConf = new float [Length/2];
	float *CopyPrePeakConf = new float [Length/2];
	int PrePeakNum = 0;
    bool bIsPeak;
	int i,j;


	// Calculate the threshold
    //
    float mean = 0;
	int nNoneZeroNum = 0; 
	for (i=0; i<Length; i++)
	{
		if (pX[i] != 0.0)
		{
			mean += pX[i];
			nNoneZeroNum++;
		}
	}   
    if (nNoneZeroNum == 0) // avoid divided by zero
    {
        nPeakNum = 0;
        return;
    }
	mean /= nNoneZeroNum; // non-zero average
	float omega = 0;
	for (i=0; i<Length; i++)
	{
		if (pX[i] != 0.0)
		{
			omega += (pX[i]-mean)*(pX[i]-mean);
		}
	}  
	omega /= nNoneZeroNum;
	omega = (float) sqrt(omega);
	float fThreshold = (float) (omega*alpha); // non-zero standard deviation
	//printf("fThreshold = %f\n\n", fThreshold);
#if 0
	// Calculate the threshold 
    //
    float mean = 0;
	for (i=0; i<Length; i++)
	{
		mean += pX[i];
	}
	mean /= Length;
	float dev = 0;
	for (i=0; i<Length; i++)
	{
		dev += (pX[i]-mean)*(pX[i]-mean);
	}
	dev = dev/Length;
	dev = sqrt(dev);
	float fThreshold = dev*1.0; // standard deviation
	//printf("fThreshold = %f\n\n", fThreshold);
#endif

	// find local peaks above the threshold
    //
	for (i=range; i<Length-range; i++)
	{
		bIsPeak = true;
		for (j=1; j<=range; j++)
		{
			if (pX[i]<pX[i-j] || pX[i]<pX[i+j]) 
            {
                bIsPeak = false;
                break;
            }
		}
		if (bIsPeak && pX[i]>fThreshold)
		{
			PrePeakPos[PrePeakNum] = i;
			PrePeakNum++;
		}
	}
#if 0
	// find peaks above the threshold
    //
	for (i=0; i<Length; i++)
	{
		if (pX[i] >= fThreshold)
		{
			PrePeakPos[PrePeakNum] = i;
			PrePeakNum++;
		}
	}
#endif


	// Calculate the intensity for each peak;
    //  
    for (i=0; i<PrePeakNum; i++)
	{
		// Using height instead of area
		PrePeakConf[i] = pX[PrePeakPos[i]];
		CopyPrePeakConf[i] = PrePeakConf[i];
	}


	// Merge some near points
    //
	for (i=0; i<PrePeakNum-1; i++)
	{
		if (PrePeakPos[i+1]-PrePeakPos[i] <= merge) // merge near points (100ms)
		{
			if(CopyPrePeakConf[i] > CopyPrePeakConf[i+1])
			{
				PrePeakConf[i+1] = 0;
			}
			else
			{
				PrePeakConf[i] = 0;
			}
		}
	}


	// Select the peaks
    //
	nPeakNum = 0;
	for (i=0; i<PrePeakNum; i++)
	{
		if (PrePeakConf[i] > fThreshold)
		{
			pPeakPos[nPeakNum]  = (float) (PrePeakPos[i]*FrameTime/2);
			pPeakConf[nPeakNum] = PrePeakConf[i];
			nPeakNum++;
		}
	}


    // release memory
	//
	delete PrePeakPos;
	delete PrePeakConf;
	delete CopyPrePeakConf;
}


//
// Half-Wave Rectify
//
void TempoFeat::Rectify(float pX[], int Length)
{
	int i;
	for (i=0; i<Length; i++)
	{
		if (pX[i] < 0)
		{
			pX[i] = 0;
		}
	}
}


//
// log-compression
//
void TempoFeat::logComp(float pX[], int Length)
{
	int i;
	for (i=0; i<Length; i++)
	{
		if (pX[i] < 0)
		{
			printf("WARNING: log-compression value below zero\n");
			exit(0);
		}
		
		// formula 1
#if 0
        if (pX[i] < 1)
		{
			pX[i] = pX[i]; // linear when 0<=x<1
		}
		else
		{
			pX[i] = log(pX[i])+1; // logarithm when x>=1
		}
#endif

		// formula 2
#if 1
		pX[i] = (float) log(1+comp*pX[i]); // u-law compression
#endif

		// formula 3
#if 0
		pX[i] = log(pX[i]+1.0); // logarithm since all data above 1
#endif
	}
}

	
//
// Hanning Window 
//
int TempoFeat::HanningWindow(float *data, int Length)
{
	for (int i=0; i<Length; i++)
	{
		float winCoeff = (float) (0.5-0.5*cos(2*3.1415926*i/(Length)));
		data[i] *= winCoeff; 
	}

	return 1;
}


//
// median analysis
//
float TempoFeat::find_median(float *head, int media_len)
{
	float *seq;
	seq = new float[media_len];
	
	int i,j;
	for(i=0; i<media_len; i++)
	{
		seq[i] = head[i];
	}

	int index;
	for(i=0; i<media_len/2; i++)
	{
		index = 0;
		for(j=1; j<media_len; j++)
		{
			if(seq[j] > seq[index])    
			{
				index = j;
			}
		}
		seq[index] = 0;
	}
	
	index =0;
	for(j=1; j<media_len; j++)
	{
		if(seq[j] > seq[index])		
		{
			index = j;
		}
	}
	
	float ret = seq[index]; 
	delete seq;
	return ret;
}


//
// median filter
//
void TempoFeat::median_filter(float head[], int len , int media_len)
{
	float *temp;
	temp = new float[len];
	memcpy(temp, head, len*sizeof(float));

	int start, end;
	if(len < media_len)	
	{
		return;
	}

	start = media_len/2;
	end = len - start;
	for(int i=start; i<end; i++)
	{
		head[i] = find_median(&temp[i-start], media_len);
	}

	delete temp;
}

// **** pitch.cpp **** //

// Gaussian likelihood
//
float TempoFeat::GaussianF0(float x)
{
	float mid = (float)((log(Lowest_F0*60.0+1.0)+log(Highest_F0*60.0+1.0))/2);
    float dev = (float)((mid-log(Lowest_F0*60.0+1.0))*F0_Gauss_Scale); 
	float p = (float) ((log(x*60+1.0)-mid)*(log(x*60+1.0)-mid)/(2*dev*dev));
	float z = (float) (1/(sqrt(2*PI)*dev)*exp(-p));
	return z;
}


// linear smoothing for the pitch array
//
void TempoFeat::LinearSmoothArray(float *pitch, int numPitch)
{
	int	i, j, t;
	float smoothed_f0;
	float *f0_bak = (float*)calloc(numPitch, sizeof(float));
	assert(f0_bak);

	for (i=0; i<numPitch; i++)
	{
		f0_bak[i] = pitch[i];
	}

	// assert odd number for smooth range
	assert(Linear_Smooth_Range%2 != 0);
	int	radius = (Linear_Smooth_Range-1)/2;
	
    // use the smoothed pitch value as the final pitch value
	float numerator;
	float denominator = (float) ((1+radius)*radius*2);
	assert(denominator > 0);
    for (t=0; t<numPitch; t++)
	{
		smoothed_f0 = 0.0f;
		if (t-radius<0 || t+radius>=numPitch)
		{
			smoothed_f0 = f0_bak[t];
		}
		else
		{
			for (i=-radius; i<=radius; i++)
			{
				j = t+i;
				if (i == 0)
				{
					numerator = 0;
				}
				else
				{
					numerator = (float)(abs(abs(i)-radius)+1);
				}
				smoothed_f0 += f0_bak[j]*numerator/denominator;
			}
			smoothed_f0 += f0_bak[t]/2.0f;
		}
		pitch[t] = smoothed_f0;
	}

	free(f0_bak);
}


// median smoothing for the pitch array
//
void TempoFeat::MedianSmoothArray(float *pitch, int numPitch)
{
	float *f0_bak, *pitchTemp, smoothed_f0;
	int	i, j, t;
	int	index, curIndex;

	/* 
	data structure: 
	process: pitch(original data)->pitchTemp(process data)->f0_bak(result)  
	copy: f0_bak->pitch 
	*/

	f0_bak = (float*)calloc(numPitch, sizeof(float));
	assert(f0_bak);

	pitchTemp = (float*)calloc(numPitch, sizeof(float));
	assert(pitchTemp);

	// assert odd number for median range
	assert(Median_Smooth_Range % 2 != 0);
	int	radius = (Median_Smooth_Range-1)/2;

	// use the median pitch value as the final pitch value
	for (t=numPitch-1; t>=0; t--) // equal to "for(t=0; t<num-1; t++)"
	{
		curIndex = t;
		if (t-radius<0 || t+radius>=numPitch)
		{
			smoothed_f0 = pitch[curIndex];
		}
		else
		{
			int	temp, outerIndex, innerIndex;
			for (i=-radius; i<=radius; i++)
			{
				index = curIndex+i;
				pitchTemp[index] = pitch[index];
			}
			
			//	sort them and get the median value
			for (i=-radius; i<=0; i++)
			{
				outerIndex = curIndex+i;
				for (j = i + 1; j <= radius; j++)
				{
					innerIndex = curIndex+j;
					if (pitchTemp[outerIndex] < pitchTemp[innerIndex])
					{
						temp = (int) pitchTemp[outerIndex];
						pitchTemp[outerIndex] = pitchTemp[innerIndex];
						pitchTemp[innerIndex] = (float) temp;
					}
				}
			}
			smoothed_f0 = pitchTemp[curIndex];
		}
		f0_bak[curIndex] = smoothed_f0;
	}

	for (index=0; index<numPitch; index++)
	{
		pitch[index] = f0_bak[index];
	}

	free(pitchTemp);
	free(f0_bak);
}


// hamming window
//
void TempoFeat::HammingWindow(float *data, int len)
{

	for (int i=0; i<len; i++)
	{
		data[i] *= hamWin[i];
	}
}

void TempoFeat::HammingWindow4Pitch(float *data)
{
	
	for (int i=0; i<Frame_Len; i++)
	{
		data[i] *= hamWin4Pitch[i];
	}
}

// initialize the kernel of pitch tracker
//
void TempoFeat::InitPitchTracker(PitchTrackGroup *tracker)
{
	int	i, j;
	
	// intialize the memory space according to NUM_F0
	tracker->F0Point = new float[NUM_F0];
	for (i=0; i<Pitch_Buf_Size; i++)
	{
		tracker->Harmonics[i] = new float[NUM_F0];
	}
		
	// initialize the Harmonics value of each F0 candidate in the buffer
	for (i=0; i<Pitch_Buf_Size; i++)
	{
		for (j=0; j<NUM_F0; j++)
		{
			tracker->Harmonics[i][j] = 0.0;
		}
	}

	// initialize the parameters
	tracker->curIndex = 0;
	tracker->frameIndex = 0;
	
	// get the pitch candidates at the Freq_Step resolution
	float floatF0 = Lowest_F0;
	for (i=0; i<NUM_F0; i++)
	{
		tracker->F0Point[i] = floatF0; // may cause false values
		floatF0 *= Freq_Step;
	}

	// set the weight for each harmonic component
	float weight = 1;
	for (i=1; i<=Max_Harmonics_Num; i++) // Max_Harmonics_Freq 1250 Hz 
	{
		tracker->weightHarm[i] = weight;
		weight = weight*Harmonics_Factor;
	}
}


// decompose the kernel of pitch tracker
//
void TempoFeat::FreePitchTracker(PitchTrackGroup *tracker)
{
	// free the memory
	delete tracker->F0Point;
	tracker->F0Point = NULL;
    for(int i=0; i<Pitch_Buf_Size; i++)
	{
		delete tracker->Harmonics[i];
		tracker->Harmonics[i] = NULL;
	}	
}


// spectral harmonic summation (original version)
//
void TempoFeat::Harmonics(PitchTrackGroup *tracker)
{
	int	i, HarmNum;
	float F0;
	float *HarmF0 = new float[NUM_F0];
	float fftSum[Num_FFT/2];
	
	for (i=0; i<NUM_F0; i++) // for all F0 candidates
	{
		HarmF0[i] = 0.0f; // for spectral summation
	}

	fftSum[0] = tracker->fft[0];
	for (i=1; i<Num_FFT/2; i++)
	{
		fftSum[i] = fftSum[i-1]+tracker->fft[i]; // sum[i] = sum of the first "i" bins
	}

	int	harmIdx, low, high;
	float harmFreq, avgE;

	//	get the processing result of harmonics
	for (i=0; i<NUM_F0; i++)
	{
		F0 = tracker->F0Point[i]; // F0 candidates in Freq_Step resolution 
		for (HarmNum=1; F0*HarmNum<=Max_Harmonics_Freq; HarmNum++) // from F0 to Max_Harmonics_Freq && (HarmNum<=18)
		{
			harmFreq = F0*HarmNum;
			harmIdx = (int)(harmFreq/Bin_Width+0.5F);
			low = (int)((harmFreq-F0*Norm_Factor)/Bin_Width+0.5f) - 1; // normalization factor = 1/2
			high = (int)((harmFreq+F0*Norm_Factor)/Bin_Width+0.5f);
			low  = (low<1)? 1 : low;
			high = (high<Num_FFT/2)? high : Num_FFT/2;
			assert((high-low) != 0);
			avgE = (fftSum[high]-fftSum[low])/(high-low);

			if (avgE < 1e-10)
			{
				continue;
			}
			else
			{
				HarmF0[i] += tracker->weightHarm[HarmNum]*(tracker->fft[harmIdx]/avgE);
				//HarmF0[i] += tracker->weightHarm[HarmNum]*tracker->fft[harmIdx];
			}
		}
	}

	//	store the results of harmonics of all F0 candidates for the current frame
	for (i=0; i<NUM_F0; i++)
	{
		assert(HarmF0[i] >= 0);
		tracker->Harmonics[tracker->curIndex][i] = HarmF0[i];
	}

	// compute the best pitch for this frame (temporal sum instead of DP)
	float *sumHarmF0 = new float[NUM_F0]; 
	int	frame, idx;
	
	for (i=0; i<NUM_F0; i++) // summed by F0 candidates not by bins 
	{
		sumHarmF0[i] = 0;
		for (frame=-(Pitch_Buf_Size-1); frame<=0; frame++) 
		{
			idx = tracker->curIndex + frame;
			idx = (idx + Pitch_Buf_Size) % Pitch_Buf_Size;
			assert(idx >= 0 && idx < Pitch_Buf_Size);
			sumHarmF0[i] += tracker->Harmonics[idx][i];
		}
	}

	// find the largest peak
	float max = -1;
	for (i=0; i<NUM_F0; i++) 
	{
		if (sumHarmF0[i] > max)
		{
			max = sumHarmF0[i];
			tracker->roughF0 = tracker->F0Point[i];
		}
	}

	// free memory
	delete sumHarmF0;
	delete HarmF0;
}


// get the pitch of the current frame
//
void TempoFeat::GetCurPitch(PitchTrackGroup *tracker, short *wave, int len, float *curPitch)
{
	int	i;
	float re[Num_FFT], fft_buf;
	assert(len <= Num_FFT);

	// zero padding
	for (i=0; i<Num_FFT; i++)
	{
		re[i] = 0.0f;
	}	
	
	// initialize FFT buffer
	for (i=0; i<len; i++)
	{
		tracker->signal[i] = wave[i]; 
		re[i] = (float) (wave[i]/32767.0); // normalization
	}
	
	// hamming window
	HammingWindow(re, len);	

	fft->XForm(re);
	
	// store FFT result in kernel
	fft_buf = re[0]*re[0];
	tracker->fft[0] = fft_buf;
	for(i=1; i<Num_FFT/2; i++)	
	{
		fft_buf = re[i]*re[i]+re[Num_FFT-i]*re[Num_FFT-i];
		//tracker->fft[i] = sqrt(fft_buf); // |X(K)|
		tracker->fft[i] = fft_buf;         // |X(k)|^2   
	}

	/* following is the main steps for the pitch tracking
	   but first you should initialize the PitchTracker before use it */

	Harmonics(tracker);
	assert(tracker->roughF0);
	*curPitch = tracker->roughF0;

	// increase index
	tracker->curIndex++;
	tracker->curIndex = tracker->curIndex % Pitch_Buf_Size;
	tracker->frameIndex++;
}


//  interface of pitch tracker
//
void TempoFeat::PitchTracker(short	*waveData, int dataLen, float *pitch, int &pitchNum)
{
	int	i;
	pitchNum = 1 + (dataLen-Frame_Len)/FRAME_SHIFT;
	PitchTrackGroup	*tracker = new PitchTrackGroup;
	InitPitchTracker(tracker);

	for (i=0; i<pitchNum; i++)
	{
		GetCurPitch(tracker, &waveData[i*FRAME_SHIFT], Frame_Len, &pitch[i]);
	}

	for (i=0; i<pitchNum-(int)(Pitch_Buf_Size/2); i++)
	{
		pitch[i] = pitch[i+(int)(Pitch_Buf_Size/2)]; // rolling: [*, *, *, *, P] -> [*, *, P, *, *]
	}

	// the sequence of "Linear" and "Median" seems doesn't crucial
	LinearSmoothArray(pitch, pitchNum);
	MedianSmoothArray(pitch, pitchNum); // median seems not so effective
		
	FreePitchTracker(tracker);
	delete tracker;
}


// spectral harmonic summation (tempo version)
//
void TempoFeat::BPMHarmonics(PitchTrackGroup *tracker)
{
	int	i, j, k, HarmNum;
	float F0;
	float *HarmF0 = new float[NUM_F0];
	float fftSum[Num_FFT/2];
	
	for (i=0; i<NUM_F0; i++) // for all F0 candidates
	{
		HarmF0[i] = 0.0f; // for spectral summation
	}

	fftSum[0] = tracker->fft[0];
	for (i=1; i<Num_FFT/2; i++)
	{
		fftSum[i] = fftSum[i-1]+tracker->fft[i]; // sum[i] = sum of the first "i" bins
	}

	int	harmIdx, low, high;
	float harmFreq, avgE;

	//	get the summation of harmonics
	for (i=0; i<NUM_F0; i++)
	{
		F0 = tracker->F0Point[i]; // F0 candidates in Freq_Step resolution 
		for (HarmNum=1; F0*HarmNum<=Max_Harmonics_Freq; HarmNum++) // from F0 to Max_Harmonics_Freq
		{
			harmFreq = F0*HarmNum;
			harmIdx = (int)(harmFreq/Bin_Width+0.5F);
			low = (int)((harmFreq-F0*Norm_Factor)/Bin_Width+0.5f) - 1; // normalization factor = 1/2
			high = (int)((harmFreq+F0*Norm_Factor)/Bin_Width+0.5f);
			low  = (low<1)? 1 : low;
			high = (high<Num_FFT/2)? high : Num_FFT/2;
			assert((high-low) != 0);
			avgE = (fftSum[high]-fftSum[low])/(high-low);

			if (avgE < 1e-10)
			{
				continue;
			}
			else
			{
				HarmF0[i] += tracker->weightHarm[HarmNum]*(tracker->fft[harmIdx]/avgE);
				//HarmF0[i] += tracker->weightHarm[HarmNum]*tracker->fft[harmIdx];
			}
		}
	}

	//	store summed harmonics of all F0 candidates for the current frame
	for (i=0; i<NUM_F0; i++)
	{
		assert(HarmF0[i] >= 0);
		HarmF0[i] *= GaussianF0(tracker->F0Point[i]); // weighted by Gaussian likelihood
		tracker->Harmonics[tracker->curIndex][i] = HarmF0[i];
	}

	// temporal-summation of summed harmonics
	float *sumHarmF0 = new float[NUM_F0]; 
	int	frame, idx;
	for (i=0; i<NUM_F0; i++) /* summed by F0 candidates not by bins */
	{
		sumHarmF0[i] = 0;
		for (frame=-(Pitch_Buf_Size-1); frame<=0; frame++) 
		{
			idx = tracker->curIndex + frame;
			idx = (idx + Pitch_Buf_Size) % Pitch_Buf_Size;
			assert(idx >= 0 && idx < Pitch_Buf_Size);
			sumHarmF0[i] += tracker->Harmonics[idx][i];
		}
	}

	// find local maxima of sumHarmF0[i]
    int area = (int)(0.5+F0_Peak_Range*NUM_F0/(60.0*(Highest_F0-Lowest_F0))); // F0_Range in BPM
	int *PeakPos = new int [NUM_F0];
	int PeakNum = 0;
    bool bIsPeak;
	for (i=area; i<NUM_F0-area; i++)
	{
		bIsPeak = true;
		for (j=1; j<=area; j++)
		{
			if (sumHarmF0[i]<sumHarmF0[i-j] || sumHarmF0[i]<sumHarmF0[i+j]) 
            {
                bIsPeak = false;
                break;
            }
		}
		if (bIsPeak)
		{
			PeakPos[PeakNum] = i;
			PeakNum++;
		}
	}
	
	// find the 1st, 2nd, 3rd ... largest local maxima;
	for(i=0; i<F0_Peak_Num; i++)
	{
		tracker->F0Peak[i] = 0.0;
		tracker->F0Score[i] = 0.0;
	}
	for(i=0; i<PeakNum; i++)
	{
		for(j=0; j<F0_Peak_Num; j++)
		{
			if(sumHarmF0[PeakPos[i]]>tracker->F0Score[j])
			{
				if(j != F0_Peak_Num-1)
				{
					for(k=F0_Peak_Num-2; k>=j; k--)
					{
						tracker->F0Peak[k+1] = tracker->F0Peak[k];
						tracker->F0Score[k+1] = tracker->F0Score[k];
					}
				}
				tracker->F0Peak[j] = tracker->F0Point[PeakPos[i]];
				tracker->F0Score[j] = sumHarmF0[PeakPos[i]];
				break;
			}
		}
	}
			
	// free memory
	delete PeakPos;
	delete sumHarmF0;
	delete HarmF0;
}


// get the BPM of the current frame
//
void TempoFeat::GetCurBPM(PitchTrackGroup *tracker, float *DF, int len, float *curPitch, float *curScore)
{
	int	i;
	float re[Num_FFT], fft_buf;
	assert(len <= Num_FFT);

	// zero padding
	for (i=0; i<Num_FFT; i++)
	{
		re[i] = 0.0f;
	}	
	
	// initialize FFT buffer
	for (i=0; i<len; i++)
	{
		tracker->signal[i] = (short) DF[i]; 
		re[i] = DF[i];                 // NO normalization
		//printf("Here[%d]: %f\n", i, re[i]);
	}

	

	

	// hamming window
	HammingWindow4Pitch(re);

	// compute FFT
	fft->XForm(re);
	
	// store FFT result in kernel
	fft_buf = re[0]*re[0];
	tracker->fft[0] = (float) sqrt(fft_buf);
	for(i=0; i<Num_FFT/2; i++)	
	{
		fft_buf = re[i]*re[i]+re[Num_FFT-i]*re[Num_FFT-i];
		tracker->fft[i] = (float) sqrt(fft_buf);   // |X(K)|
		//tracker->fft[i] = fft_buf;       // |X(k)|^2
		
	}

	/* following is the main steps for the pitch tracking
	   but first you should initialize the PitchTracker before use it */

	BPMHarmonics(tracker);
	for(i=0; i<F0_Peak_Num; i++)
	{	
		assert(tracker->F0Peak[i] >= 0);
		assert(tracker->F0Score[i] >= 0);
		curPitch[i] = tracker->F0Peak[i];
		curScore[i] = tracker->F0Score[i];
	}
/*
	for (i=0; i<F0_Peak_Num; i++)
	{
		printf("pitch[%d]: %f\n", i, curPitch[i]);
		printf("score[%d]: %f\n", i, curScore[i]);
	}
*/
	// increase index
	tracker->curIndex++;
	tracker->curIndex = tracker->curIndex % Pitch_Buf_Size;
	tracker->frameIndex++;
}


// interface of tempo tracker
//
/*
 	float **a = new float* [BPM_Num];
    float *b = new float [BPM_Num*F0_Peak_Num];
	a[i] = &b[i*F0_Peak_Num];
	delete a[0];
	delete a;
 */
void TempoFeat::TempoTracker(float *DF, int dataLen, float** &pitch, float** &score, int &pitchNum)
{
	int	i, j;
	
	pitchNum = 1 + (dataLen-Frame_Len)/FRAME_SHIFT;
	pitch = new float* [pitchNum];
	score = new float* [pitchNum];
	float *memory1 = new float [pitchNum*F0_Peak_Num];
	float *memory2 = new float [pitchNum*F0_Peak_Num];
	for(i=0; i<pitchNum; i++)
	{
		pitch[i] = &memory1[i*F0_Peak_Num];
		score[i] = &memory2[i*F0_Peak_Num];
	}
	
	PitchTrackGroup	*tracker = new PitchTrackGroup;
	InitPitchTracker(tracker);

	for (i=0; i<pitchNum; i++)
	{
		GetCurBPM(tracker, &DF[i*FRAME_SHIFT], Frame_Len, pitch[i], score[i]);
	}



	// **** the original code does not have this "if"
	// **** if pitchNum is too small, then we skip this part
	if (pitchNum > (int)(Pitch_Buf_Size-1))
	{
		// **** Pitch_Buf_Size's value is 11
		for (i=0; i<pitchNum-(int)(Pitch_Buf_Size-1); i++) // only keep the most reliable values
		{
			for(j=0; j<F0_Peak_Num; j++)
			{
				pitch[i][j] = pitch[i+(int)(Pitch_Buf_Size-1)][j]; // rolling: [*, *, *, *, P] -> [*, *, P, *, *]
				score[i][j] = score[i+(int)(Pitch_Buf_Size-1)][j];
			}
		}
		pitchNum -= (int)(Pitch_Buf_Size-1); // delete values at front-end & rear-end: [X, X, *, *, *, X, X] -> [*, *, *]
	}


  
	for (i=0; i<pitchNum; i++)
	{
		for(j=0; j<F0_Peak_Num; j++)
		{
			pitch[i][j] *= 60.0; // Hz -> BPM
			score[i][j] = (float) log(score[i][j]+1.0); // log-scale
		}	
	}
	 	
	FreePitchTracker(tracker);
	delete tracker;
}

// **** tempo.cpp **** //

 
// transition probability
float TempoFeat::TransProb(float src, float dst)
{
	float mid = (float) log(src+1.0);
	float dev = (float) ((mid-log(src/2+1.0))*BPM_Gauss_Scale);
	float p = (float) ((log(dst+1.0)-mid)*(log(dst+1.0)-mid)/(2*dev*dev));
	float z = (float) (1/(sqrt(2*PI)*dev)*exp(-p));
	return z;
}


// dynamic programming
void TempoFeat::DP(float **bpm, float **local, int nFrame, int nPos, int *trace)
{
	int		i, j, k;
	float	**accu;
	int		**path;
	float	*trans;
	float	maxCost;
	int		maxIdx;

	// allocate memory
	trans = new float [nPos];
	accu = new float * [nFrame];
	path = new int * [nFrame];

	for (i = 0; i < nFrame; i++)
	{
		accu[i] = new float [nPos];
		path[i] = new int [nPos];
	}

	// init values
	for (j = 0; j < nPos; j++)
	{
		accu[0][j] = local[0][j];
		path[0][j] = -1;
	}

	// dynamic programming
	for (i = 1; i < nFrame; i++)
	{
		for (j = 0; j < nPos; j++)
		{
			for (k = 0; k < nPos; k++)
			{
				trans[k] = accu[i-1][k] + (float) (Trans_Prob_Scale*TransProb(bpm[i-1][k], bpm[i][j]) + local[i][j]); // [i-1][k]->[i][j]
			}
			maxCost = -1E38F;
			maxIdx = -1;
			for (k = 0; k < nPos; k++)
			{
				if (trans[k] > maxCost)
				{
					maxCost = trans[k];
					maxIdx = k;
				}
			}
			assert(maxIdx > -1);
			accu[i][j] = maxCost;
			path[i][j] = maxIdx;
		}
	}

	// trace back path
	maxCost = -1E38F;
	maxIdx = -1;
	for (j = 0; j < nPos; j++)
	{
		if (accu[nFrame-1][j] > maxCost)
		{
			maxCost = accu[nFrame-1][j];
			maxIdx = j;
		}
	}
	assert(maxIdx > -1);

	trace[nFrame-1] = maxIdx;
	for (i = nFrame-1; i > 0; i--) 
	{
		trace[i-1] = path[i][trace[i]];
	}

	// release memory
	for (i = 0; i < nFrame; i++)
	{
		delete	accu[i];
		delete	path[i];
	}
	delete	trans;
	delete	accu;
	delete	path;
}


// median analysis
void TempoFeat::find_mid(float *head, int len, float &mid)
{
	float *seq;
	seq = new float[len];
	
	int i,j;
	for(i=0; i<len; i++)
	{
		seq[i] = head[i];
	}

	int index;
	for(i=0; i<len/2; i++)
	{
		index = 0;
		for(j=1; j<len; j++)
		{
			if(seq[j] > seq[index])    
			{
				index = j;
			}
		}
		seq[index] = 0;
	}
	
	index =0;
	for(j=1; j<len; j++)
	{
		if(seq[j] > seq[index])		
		{
			index = j;
		}
	}
	
	mid = seq[index]; 
	delete seq;
}


// average analysis
void TempoFeat::find_ave(float *data, int len, float &ave)
{
	float sum = 0;
	for (int i=0; i<len; i++)
	{
		sum += data[i];
	}
	ave = sum/(float)len;
}

/*
// pre-processing for WAVE
void TempoFeat::Preprocess(char *WAVEFILE, short* &WavData, int &SampleNum, int &SampleRate)
{
	// command line
	char cmdline[1024] = {0};
	char wavefile[1024] = {0};
	strcpy(wavefile, "\""); 
	strcat(wavefile, WAVEFILE);
	strcat(wavefile, "\"");

	// check out sample rate
	GetWavRate(WAVEFILE, SampleRate);

	// converting WAVE format using "SOX"
	if (SampleRate == 16000)
	{	
		WavData = ReadWave(WAVEFILE, &SampleNum, &SampleRate);
	}
	else
	{
		sprintf(cmdline, "sox.exe %s -s -w -c 1 -r 16000 temp.wav resample -q", wavefile);
		system(cmdline);
		WavData = ReadWave("temp.wav", &SampleNum, &SampleRate);
		remove ("temp.wav");
	}
}
*/
/*
// track the BPM curve in a piece of music
void TempoFeat::TempoCurve(char *WAVEFILE, float* &BPM, int &BPMNUM)
{
	int i;
	// preprocess for WAVE
	int SampleNum = 0;
	int	SampleRate = 0;
	short *WavData = NULL;
	Preprocess(WAVEFILE, WavData, SampleNum, SampleRate);

	// build detection function
	int IdxNum = 0;
	float *DF64 = NULL;
	Detect_Func_log(WavData, SampleNum, SampleRate, DF64, IdxNum);
	
	// transform DF64 into DF32
	float *DF32 = new float[IdxNum];
	for (i=0; i<IdxNum; i++)
	{
		DF32[i] = (float)DF64[i];
	}

	// tempo tracking
	float **BPMBUF = NULL;
	float **SCOREBUF = NULL;
	TempoTracker(DF32, IdxNum, BPMBUF, SCOREBUF, BPMNUM);

	// dynamic programming
	int *TRACK = new int [BPMNUM];
	DP(BPMBUF, SCOREBUF, BPMNUM, F0_Peak_Num, TRACK);

	// single path
	BPM = new float[BPMNUM];
	for(i=0; i<BPMNUM; i++)
	{
		BPM[i] = BPMBUF[i][TRACK[i]];
	}

	// release memory
	delete WavData;
	delete DF64;
	delete DF32;
	delete BPMBUF[0];
	delete BPMBUF;
	delete SCOREBUF[0];
	delete SCOREBUF;
	delete TRACK;
}
*/

// estimate the BPM of a music file
void TempoFeat::BPM_Estimate(short *WavData, unsigned long SampleNum, float &BPM)
{
	int i;

	// build detection function
	int IdxNum = 0;
	float *DF64 = NULL;
	Detect_Func_log(WavData, SampleNum, SAMPLERATE, DF64, IdxNum);
	
	// transform DF64 into DF32
	//IdxNum -= 2*(int)(5.0/(FrameTime/2)); // cut the first & last 5s
	float *DF32 = new float[IdxNum];
	for (i=0; i<IdxNum; i++)
	{
		DF32[i] = (float)DF64[i];
		//DF32[i] = (float)DF64[i + (int)(5.0/(FrameTime/2))];
		//printf("[%d] DF32: %f\n", i, DF32[i]);
	}

	// tempo tracking
	int BPMNUM = 0;
	float **BPMBUF = NULL;
	float **SCOREBUF = NULL;
	TempoTracker(DF32, IdxNum, BPMBUF, SCOREBUF, BPMNUM);

	// dynamic programming
	int *TRACK = new int [BPMNUM];
	DP(BPMBUF, SCOREBUF, BPMNUM, F0_Peak_Num, TRACK);

	// single path
	float *BPMBUF2 = new float[BPMNUM];
	for(i=0; i<BPMNUM; i++)
	{
		BPMBUF2[i] = BPMBUF[i][TRACK[i]];
	}

/*	
	for (i=0; i<BPMNUM; i++)
	{
		printf("BPM[%d]: %f\n", i, BPMBUF2[i]);
	}
*/	
	
	// estimate the overall tempo
	find_mid(BPMBUF2, BPMNUM, BPM);

	// release memory
	//delete WavData;
	delete DF64;
	delete DF32;
	delete BPMBUF[0];
	delete BPMBUF;
	delete SCOREBUF[0];
	delete SCOREBUF;
	delete TRACK;
	delete BPMBUF2;
}

