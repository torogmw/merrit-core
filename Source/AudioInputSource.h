//
//  AudioInputSource.h
//  merrit_core
//
//  Created by Minwei Gu on 5/18/15.
//
//

#ifndef __merrit_core__AudioInputSource__
#define __merrit_core__AudioInputSource__

#include <stdio.h>

#include <iostream>
#include "JuceHeader.h"
#include "AudioAnalyzer.h"

class AudioInputSource : public AudioIODeviceCallback
{
public:
    AudioInputSource(AudioDeviceManager& deviceManager, AudioAnalyzer *audioAnalyzer);
    ~AudioInputSource();
    void audioDeviceIOCallback(const float** inputChannelData,
                               int totalNumInputChannels,
                               float** outputChannelData,
                               int totalNumOutputChannels,
                               int numSamples);
    void audioDeviceAboutToStart (AudioIODevice* device);
    void audioDeviceStopped();
    void setFile(File audioFile);
    void filePlayingControl();

private:
    
    AudioDeviceManager& deviceManager;
    AudioSourcePlayer audioSourcePlayer;
    AudioFormatReaderSource* fileSource;
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    TimeSliceThread playingThread;
    AudioAnalyzer *audioAnalyzer;
    int numSamplesReadFromFile;
    AudioSampleBuffer fullBuffer = AudioSampleBuffer(1, FS_MIR * 60); // start with an empty buffer and fill with audio data, 661500 = 11025 * 60
};

#endif /* defined(__merrit_core__AudioInputSource__) */
