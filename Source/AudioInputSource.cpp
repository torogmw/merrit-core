//
//  AudioInputSource.cpp
//  merrit_core
//
//  Created by Minwei Gu on 5/18/15.
//
//

#include "AudioInputSource.h"

AudioInputSource::AudioInputSource(AudioDeviceManager& deviceManager_, AudioAnalyzer *audioAnalyzer_):deviceManager(deviceManager_), playingThread("audio Input source")
{
    formatManager.registerBasicFormats();
    audioSourcePlayer.setSource(&transportSource);
    playingThread.startThread();
    audioAnalyzer = audioAnalyzer_;
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
        transportSource.setSource(fileSource,32768,&playingThread,FS_MIR);
//        deviceManager.addAudioCallback(this); // will call audioDeviceIOCallback
        
        // for file input, get the whole buffer and run MIR here
        fullBuffer.clear();
        tempReader->read(&fullBuffer, 0, tempReader->lengthInSamples, 0, true, false);
        numSamplesReadFromFile = tempReader->lengthInSamples;
        
        audioAnalyzer->Clear();
        
        for (int i=0; (i+audioAnalyzer->frame_size) < numSamplesReadFromFile; i+=audioAnalyzer->hop_size) {
            audioAnalyzer->UpdateFrameBuffer(fullBuffer.getReadPointer(0, i), audioAnalyzer->hop_size);
            audioAnalyzer->FrameAnalysis(audioAnalyzer->frame_buffer);
        }
        
        for (int i=0; i<audioAnalyzer->feature_size; i++) {
            audioAnalyzer->SubbandAnalysis(audioAnalyzer->subband_signals[i], i+audioAnalyzer->min_note);
        }
        
//        for (TimedNotes::iterator it = audioAnalyzer->audio_notes.begin(); it != audioAnalyzer->audio_notes.end(); it++) {
//            printf("%f:", it->first);
//            for (std::vector<struct Note>::iterator kt=it->second.begin(); kt!=it->second.end(); kt++) {
//                printf("%u,%f ", kt->midi_pitch, kt->valence);
//            }
//            printf("\n");
//        }
        
        float grade = audioAnalyzer->AudioScoreAlignment();
        printf("grade=%f\n", grade);
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