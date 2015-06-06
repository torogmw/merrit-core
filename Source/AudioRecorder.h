//
//  AudioRecorder.h
//  merrit-core
//
//  Created by Minwei Gu on 5/19/15.
//
//

#ifndef __merrit_core__AudioRecorder__
#define __merrit_core__AudioRecorder__

#include <stdio.h>
#include "JuceHeader.h"
#include "AudioAnalyzer.h"

class AudioRecorder : public AudioIODeviceCallback
{
public:
    AudioRecorder(AudioAnalyzer *audioAnalyzer);
    ~AudioRecorder();
    void startRecording(const File& file);
    void stop();
    bool isRecording() const;
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallback(const float** inputChannelData, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels,
                                int numSamples) override;
    
private:
    TimeSliceThread backgroundThread; // the thread that will write our audio data to disk
    ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    double sampleRate;
    int64 nextSampleNum;
    AudioAnalyzer *audioAnalyzer;
    
    CriticalSection writerLock;
    AudioFormatWriter::ThreadedWriter* volatile activeWriter;
};
#endif /* defined(__merrit_core__AudioRecorder__) */
