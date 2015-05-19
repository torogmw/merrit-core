//
//  AudioInputSource.cpp
//  merrit_core
//
//  Created by Minwei Gu on 5/18/15.
//
//

#include "AudioInputSource.h"

AudioInputSource::AudioInputSource(AudioDeviceManager& deviceManager_, int choice_):deviceManager(deviceManager_), playingThread("audio Input source"),choice(choice_)
{
    if (choice == FILE_INPUT)
    {
        formatManager.registerBasicFormats();
        audioSourcePlayer.setSource(&transportSource);
    }
    deviceManager.addAudioCallback(this);
    playingThread.startThread();
    bufferReady = false;
    
    bufferIndex = 0;
    ok = true;
    inputToggle=0;
    
    // start fresh
    fullBuffer.clear();
    numSamplesCopied = 0;
}


AudioInputSource::~AudioInputSource()
{
    deviceManager.removeAudioCallback(this);
    if(choice == FILE_INPUT)
    {
        transportSource.setSource(0);
        deleteAndZero(fileSource);
        audioSourcePlayer.setSource(0);
    }
}

void AudioInputSource::setFile(File audioFile)
{
    if (choice == FILE_INPUT)
    {
        if(audioFile.exists())
        {
            AudioFormatReader* tempReader = formatManager.createReaderFor(audioFile);
            fileSource = new AudioFormatReaderSource(tempReader,true);
            transportSource.setSource(fileSource,32768,&playingThread,FS);
            //transportSource.start();
            inputToggle=1;
        }
    }
    if (choice == LIVE_INPUT)
    {
        // handling live input here
    }
    
    // start fresh
    fullBuffer.clear();
    numSamplesCopied = 0;
}

int AudioInputSource::getCurrentPitch() const
{
    return 0;
}

void AudioInputSource::audioDeviceIOCallback(const float **inputChannelData, int totalNumInputChannels, float **outputChannelData, int totalNumOutputChannels, int numSamples)
{
    
    
    if (choice == FILE_INPUT )
    {
        audioSourcePlayer.audioDeviceIOCallback (inputChannelData, totalNumInputChannels, outputChannelData, totalNumOutputChannels, numSamples);
        //pass the output to the player
        
    }
    if (choice == LIVE_INPUT)
    {
        //std::cout<<*inputChannelData[0]<<std::endl;
        
        for (int i = 0; i < numSamples; ++i)
            for (int j = totalNumOutputChannels; --j >= 0;)
                outputChannelData[j][i] = 0;
        
        if (bufferReady == true){
            bufferReady = false;
        }
        
        if (bufferReady == false)
        {
            sampleBuffer.clear();
            sampleBuffer.copyFrom(0, 0, inputChannelData[0], numSamples);
            tempBuffer.copyFrom(0, 0, calculateBuffer, 0, numSamples, RECORDSIZE - numSamples);
            calculateBuffer.clear();
            tempBuffer.copyFrom(0, RECORDSIZE - numSamples, sampleBuffer, 0, 0, numSamples);
            calculateBuffer.copyFrom(0, 0, tempBuffer, 0, 0, RECORDSIZE);
            tempBuffer.clear();
            bufferReady = true;
        }
        
    }
    
    // stereo to mono, down-sampling
    // for now it doesn't do MIR while reading audio input
    // FIXME: this function is running forever, how to turn the loop off when file read is done / record finishes?
    if (numSamplesCopied < 12 * FS_MIR) { // hard-code termination now
        for (int i=0; i < BLOCK_SIZE; i += SAMPLE_RATE) {
            float value = inputChannelData[0][i];
            if (totalNumInputChannels == 2) {
                value = 0.5 * (inputChannelData[0][i] + inputChannelData[1][i]);
            }
            fullBuffer.copyFrom(0, numSamplesCopied, &value, 1);
            numSamplesCopied++;
        }
    }
}

void AudioInputSource::filePlayingControl()
{
    if(choice==FILE_INPUT)
    {
        if(transportSource.isPlaying())
            transportSource.stop();
        else
            transportSource.start();
    }
    
}


int AudioInputSource::getCurrentTech() const
{

}

void AudioInputSource::setThredhold(float sliderValue)
{
    std::cout<<"set threshold: "<<sliderValue<<std::endl;
}

void AudioInputSource::audioDeviceAboutToStart(AudioIODevice* device)
{
    if (choice == FILE_INPUT)
        audioSourcePlayer.audioDeviceAboutToStart (device);
}

void AudioInputSource::audioDeviceStopped()
{
    if (choice == FILE_INPUT)
        audioSourcePlayer.audioDeviceStopped();
}