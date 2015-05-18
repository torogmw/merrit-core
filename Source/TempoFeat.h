#ifndef _TEMPO_FEAT
#define _TEMPO_FEAT

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "rsrfft.h"

// for batch processing 
#define SAMPLERATE  16000
#define CHANNELNUM  1
#define WAVEDATALEN (SAMPLERATE*DATATIME*CHANNELNUM)
#define FILENUM     235    // for batch file processing
#define STARTTIME   180    // sec | startup position of input file
#define DATATIME    270    // sec | maximum length of input file
// for detection function 
#define FFTSIZE     512  // FFTSIZE = 2^FFTINDEX = SAMPLERATE*FrameTime
#define FFTINDEX    9
#define FrameTime   0.016                                // 16ms
#define SUBBANDNUM  (int)(FFTSIZE/2)                     // all frequncy-channels
#define HalfHannLen (int)(HAMLEN/(1000.0*FrameTime/2)+1) // points
#define HAMLEN      50//100                              // ms 
#define comp        100                               // u-law compression factor
#define CANNYLEN    4//2                                 // points (8ms/point)
// for onset detection
#define range       1                                    // local peak (8ms/point)
#define merge       12                                   // peak merge (8ms/point)
#define alpha       1.5									 // standard-deviation threshold
#define PI			3.14159265428

// for pitch tracking
#define		SAMPLE_RATE		        125//16000//8000//Hz
#define		FRAME_TIME		        4096//32//ms
#define		FRAME_SHIFT_TIME		1024//10//ms
#define		FFT_ORDER		        10//((SAMPLE_RATE == 16000)? 10 : 9)
#define		Num_FFT			        (2<<(FFT_ORDER-1)) 
#define		Frame_Len		        (int)(SAMPLE_RATE / 1000.0F * FRAME_TIME)	//point number per frame
#define		FRAME_SHIFT		        (int)(SAMPLE_RATE / 1000.0F * FRAME_SHIFT_TIME)	//point number per shift
#define		Bin_Width	            ((float)SAMPLE_RATE / Num_FFT)
#define		Lowest_F0		        0.5//30bpm//50//Hz	
#define		Highest_F0		        3.5//210bpm//500//Hz
#define     Max_Harmonics_Num       (int)(Max_Harmonics_Freq/(float)Lowest_F0)
#define		Max_Harmonics_Freq	    ((float)SAMPLE_RATE/2)//2000//Hz the max harmonics frequency being considered
#define		Freq_Step			    (1+Bin_Width/(float)Max_Harmonics_Freq) // Fnxt = Fpre+dF = Fpre+Fpre*0.25 = Fpre(1+0.25)
#define     NUM_F0                  (int)(log(Highest_F0/(float)Lowest_F0)/log(Freq_Step)) // F0 candidates in Freq_step
#define		Harmonics_Factor	    0.9F//0.915F//harmonic compression factor
#define     Norm_Factor             0.5F//(2/3.f)//spectral normalization factor

// **** I change it to 3
#define		Pitch_Buf_Size		    3	//harmonic buffers for adjacent frames
#define     Linear_Smooth_Range		0   //linear smooth window size
#define		Median_Smooth_Range		0	//median smooth window size
// for tempo estimation
#define		F0_Peak_Num			1		// number of F0 candidates for dynamic programming	
#define		F0_Peak_Range		5		// +-(BPM) range for determining local F0 peaks		
#define     F0_Gauss_Scale		2.		// dev weight for GaussianF0
#define		BPM_Gauss_Scale		1/2.	// dev weight for TransProb                
#define		Trans_Prob_Scale	(BPM_Gauss_Scale*Pitch_Buf_Size) // weight for transition probability

#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }



// the kernel for pitch tracker
struct	PitchTrackGroup
{
	float	*Harmonics[Pitch_Buf_Size];	// array for storing results of harmonics
	float	roughF0; //	the pitch value
	float	fft[Num_FFT/2];	// real number, the |FFT| result of the current frame
	short	signal[Frame_Len]; // 16-bit data, the sample signal of the current frame
	float	*F0Point; // array for storing values of F0 candidates
	float	weightHarm[Max_Harmonics_Num+1]; // weight for harmonic component
	
	int		curIndex;	//	the current index in the buffer
	int		frameIndex;	//	the frame index in the speech file
	
	float   F0Peak[F0_Peak_Num];	 //  FOR TEMPO: local maxima of F0Point[i]
	float	F0Score[F0_Peak_Num]; //	 FOR TEMPO: scores of local maxima
};

class TempoFeat
{
public:
	int OnsetRate(short* buffer, unsigned long length);
	int TempoBPM(short* buffer, unsigned long length);
	TempoFeat();
	~TempoFeat();

	float onsetRate;
	float tempoBPM;


private:
	SplitRadixFFT *fft;
	float *hamWin;
	float *hamWin4Pitch;
	// **** From onset.h

	// detection detection by means of log-compression
	void Detect_Func_log(short* pRawData, int nDataLen, int nSampleRate, float* &DF, int &idxNum);
	
	// Estimate the onset of note or beat by Spectral Flux analysis 
	int Onset_Detect(short *pData, unsigned long nDataLen, int nSampleRate, float *PeakPos, float *PeakConf, int &nPeakNum);
	
	// Calculate the convolve result
	int Convolve(float pX1[], int Length1, float pX2[], int Length2, float pY[]);
    
	// Calculate the differences
	int Canny_Estimate(float pX[], float pY[], int Length, float Sigma);
    
	// Select peaks with modified standard deviation threshold
	void Peak_Select(float pX[], int Length, float *pPeakPos, float *pPeakConf, int &nPeakNum);
	
	// Calculate the half-hanning
    int HalfHanning_Time(float pWindow[], int time);
    
	// half-wave rectify
	void Rectify(float pX[], int Length);
	
	// log-compression
	void logComp(float pX[], int Length);
    
	// Hanning Window
	int HanningWindow(float *data, int Length);
	
	// median analysis
	float find_median(float *head, int media_len);
	
	// median filter
	void median_filter(float head[], int len, int media_len);

	// **** From pitch.h

	// function interface
	void    HammingWindow(float *data, int len);
	void    HammingWindow4Pitch(float *data);
	void	LinearSmoothArray(float *pitch, int numPitch);
	void	MedianSmoothArray(float *pitch, int numPitch);
	void	Harmonics(PitchTrackGroup *tracker);
	void	GetCurPitch(PitchTrackGroup *tracker, short *wave, int len, float *curPitch);
	void	InitPitchTracker(PitchTrackGroup *tracker);
	void    FreePitchTracker(PitchTrackGroup *tracker);
	void    PitchTracker(short *waveData, int dataLen, float *pitch, int &pitchNum);
	// for tempo estimation
	float	GaussianF0(float x);
	void	BPMHarmonics(PitchTrackGroup *tracker);
	void	GetCurBPM(PitchTrackGroup *tracker, float *DF, int len, float *curPitch, float *curScore);
	void	TempoTracker(float *DF, int dataLen, float** &pitch, float** &score, int &pitchNum);

	// **** From tempo.h

	// transition probability
	float TransProb(float src, float dst);
	
	// dynamic programming
	void DP(float **bpm, float **local, int nFrame, int nPos, int *trace);
	
	// median analysis
	void find_mid(float *head, int len, float &mid);
	
	// average analysis
	void find_ave(float *data, int len, float &ave);
	
	// pre-processing for WAVE
	void Preprocess(char *WAVEFILE, short* &WavData, int &SampleNum, int &SampleRate);
	
	// track the BPM curve in a piece of music
	void TempoCurve(char *WAVEFILE, float* &BPM, int &BPMNUM);
	
	// estimate the BPM of a music file
	void BPM_Estimate(short *buffer, unsigned long length, float &BPM);
};

#endif