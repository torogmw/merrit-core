//
//  AudioInputSource.cpp
//  merrit_core
//
//  Created by Minwei Gu on 5/18/15.
//
//

#include "AudioInputSource.h"

AudioInputSource::AudioInputSource(AudioDeviceManager& deviceManager_):deviceManager(deviceManager_), playingThread("audio Input source")
{
    formatManager.registerBasicFormats();
    audioSourcePlayer.setSource(&transportSource);
    playingThread.startThread();
    audioAnalyzer = new AudioAnalyzer(FS_MIR, 512);
    numSamplesReadFromFile = 0;
}


AudioInputSource::~AudioInputSource()
{
    deviceManager.removeAudioCallback(this);
    transportSource.setSource(0);
    deleteAndZero(fileSource);
    audioSourcePlayer.setSource(0);
}

void AudioInputSource::setFile(File audioFile)
{
    if(audioFile.exists())
    {
        AudioFormatReader* tempReader = formatManager.createReaderFor(audioFile);
        fileSource = new AudioFormatReaderSource(tempReader,true);
        transportSource.setSource(fileSource,32768,&playingThread,FS);
//        deviceManager.addAudioCallback(this); // will call audioDeviceIOCallback
        
        // for file input, get the whole buffer and run MIR here
        fullBuffer.clear();
        tempReader->read(&fullBuffer, 0, tempReader->lengthInSamples, 0, true, false);
        numSamplesReadFromFile = tempReader->lengthInSamples;
        
//        for (int i=0; i<numSamplesReadFromFile; i++) {
//            printf("%f\n", fullBuffer.getReadPointer(0, i));
//        }
        
        audioAnalyzer->Clear();
        for (int i=0; (i+audioAnalyzer->frame_size) < numSamplesReadFromFile; i+=audioAnalyzer->hop_size) {
            audioAnalyzer->UpdateFrameBuffer(fullBuffer.getReadPointer(0, i), audioAnalyzer->hop_size);
            audioAnalyzer->FrameAnalysis(audioAnalyzer->frame_buffer);
        }
        
//        for (int i=0; i<audioAnalyzer->frame_num; i++) {
//            printf("%f\n", audioAnalyzer->subband_signals[68-audioAnalyzer->min_note][i]);
//        }
    }
}

void AudioInputSource::audioDeviceIOCallback(const float **inputChannelData, int totalNumInputChannels, float **outputChannelData, int totalNumOutputChannels, int numSamples)
{
    audioSourcePlayer.audioDeviceIOCallback (inputChannelData, totalNumInputChannels, outputChannelData, totalNumOutputChannels, numSamples);
    //pass the output to the player
    
    // won't work for file input
//    numSamplesReadFromFile += numSamples;
//    if (audioAnalyzer->UpdateFrameBuffer(inputChannelData[0], numSamples) < 0) {
//        printf("oh no!\n");
//    }
//    float features[120];
//    audioAnalyzer->FrameAnalysis(audioAnalyzer->frame_buffer, features);
//    if (numSamplesReadFromFile > fileSource->getTotalLength()) {
//        deviceManager.removeAudioCallback(this);
//    }
}

void AudioInputSource::filePlayingControl()
{
    if(transportSource.isPlaying())
        transportSource.stop();
    else
        transportSource.start();
}

void AudioInputSource::audioDeviceAboutToStart(AudioIODevice* device)
{
    audioSourcePlayer.audioDeviceAboutToStart (device);
    
    // prepare for a new recording
    numSamplesReadFromFile = 0;
}

void AudioInputSource::audioDeviceStopped()
{
    audioSourcePlayer.audioDeviceStopped();
    printf("Run dynamic programming here!");
}