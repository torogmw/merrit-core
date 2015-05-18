//#include <iostream>
//using namespace std;
//#include <fstream>
//#include "wave.h"
//#include "rsrfft.h"
//#include <cmath>
//
//#include "TemporalFeat.h"
//#include "ChromaFeat.h"
//#include "TempoFeat.h"
//#include "SpectralContrastFeat.h"
//
//int main()
//{
//
//	// initialize
//	Wave song = Wave("audio/1.wav"); // read the specific wav file
//	cout<<"Length of the song: "<<song.length<<endl;
//	short* buffer = &song.data[0];
//	unsigned long length = song.Time2Sample(100); // buffer's length
//	cout<<"Length of a frame: "<<length<<endl;
//
//	unsigned i;
//
//	/*
//	TemporalFeat tf;
//	tf.ZCR(buffer, length);
//	cout<<"ZCR: "<<tf.zcr<<endl;
//
//	tf.Energy(buffer, length);
//	cout<<"Energy: "<<tf.energy<<endl;
//	*/
//
//	
//	ChromaFeat hf(length);
///*
//	unsigned long hopSize = (unsigned long) length*0.5;
//		
//	unsigned long numFrm = (unsigned long) floor((song.length-length)/hopSize);
//	cout<<numFrm<<endl;
//	ofstream fout("Chroma.txt", ios::binary);
//	
//	for (j=0; j<numFrm; j++)
//	{
//		hf.Chroma(&buffer[j*hopSize]);
//		//cout<<j*hopSize<<endl;
//		for (i=0; i<12; i++) 
//		{
//			fout.write((char*)&hf.chroma[i], sizeof(float));
//		}
//	}
//	fout.close();
//*/	
//hf.Chroma(buffer);
//for (i=0; i<12; i++) 
//{
//	cout<<hf.chroma[i];
//}
//
///*
//	// frame-by-frame experiment
//	short hopSize = 0.5 * length;
//	short frameNum = (song.length - length) / hopSize + 1;
//	for (short i2=0; i2<frameNum; i2++) {
//		buffer = &song.data[i2 * hopSize];
//		
//		HarmonicFeat hf;
//		hf.Chroma(buffer, length);
//		float max = hf.chroma[0];
//		short max_pos = 0;
//		for (short i3=1; i3<12; i3++) {
//			if (max < hf.chroma[i3]) {
//				
//				max = hf.chroma[i3];
//				max_pos = i3;
//			}	
//		}
//		cout<<"The no."<<i2<<" frame's dominant chroma is at "<<max_pos<<endl;
//	}
//*/	
///*
//	// experiment with song's function
//	SPECTRAL_SHAPEBASEINFO s_info;
//	s_info.smpPeriod = 625;
//	s_info.framePeriod = 250000;
//	s_info.winSize = 500000;
//	SpectralShape ss;
//	ss.Initialize(s_info);
//	ss.AddWavedata(buffer, length);
//	ss.SpectralCent();
//	cout<<ss.m_fCent<<endl;
//*/
//
//	// Onset rate experiment
//	// choose a large buffer or the onset rate doesn't make any sense!
//	TempoFeat tf2;
//
//	tf2.TempoBPM(song.data, song.length);
//	cout<<"BPM: "<<tf2.tempoBPM<<endl;
//	tf2.OnsetRate(song.data, song.length);
//	cout<<"Onset rate: "<<tf2.onsetRate<<endl;
//
//	// Raw Spectral Contrast Feature experiment
//	SpectralContrastFeat scf(length);
//	scf.RawSpectralContrast(buffer);
//	cout<<"Raw Spectral Contrast: "<<endl;
//	for (i=0; i<6; i++)
//	{
//		cout<<"Raw Spectral Contrast at subband "<<i<<" "<<scf.rawSpectralContrast[2*i]<<endl;
//		cout<<"Valley at subband "<<i<<" "<<scf.rawSpectralContrast[2*i+1]<<endl;
//	}
//	
//	return 0;
//
//}