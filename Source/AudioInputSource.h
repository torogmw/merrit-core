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

#define LIVE_INPUT 0
#define FILE_INPUT 1

#define RECORDSIZE 44544

class AudioInputSource : public AudioIODeviceCallback
{
public:
    AudioInputSource(AudioDeviceManager& deviceManager, int choice);
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
    int getCurrentPitch() const;
    int getCurrentTech() const;
    void setThredhold(float sliderValue);

private:
    
    AudioDeviceManager& deviceManager;
    AudioSourcePlayer audioSourcePlayer;
    AudioFormatReaderSource* fileSource;
    AudioTransportSource transportSource;
    AudioFormatManager formatManager;
    TimeSliceThread playingThread;
    
    AudioSampleBuffer sampleBuffer = AudioSampleBuffer(1,RECORDSIZE); //the buffer is for store;
    AudioSampleBuffer calculateBuffer = AudioSampleBuffer(1,RECORDSIZE); //the buffer is throwing to the pitchtail
    AudioSampleBuffer tempBuffer = AudioSampleBuffer(1,RECORDSIZE); // this buffer is for sliding buffer window
    
    int choice;
    bool inputToggle;
    bool bufferReady;
    int bufferIndex;
    bool ok;
    
};

#endif /* defined(__merrit_core__AudioInputSource__) */
