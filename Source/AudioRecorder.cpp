//
//  AudioRecorder.cpp
//  merrit-core
//
//  Created by Minwei Gu on 5/19/15.
//
//

#include "AudioRecorder.h"

AudioRecorder::AudioRecorder(AudioAnalyzer *audioAnalyzer_):backgroundThread ("Audio Recorder Thread"),
sampleRate (0), nextSampleNum (0), activeWriter (nullptr)
{
    audioAnalyzer = audioAnalyzer_;
    backgroundThread.startThread();
}

AudioRecorder::~AudioRecorder()
{
    stop();
}

void AudioRecorder::startRecording(const File& file)
{
    stop();

    if (sampleRate > 0)
    {
        // Create an OutputStream to write to our destination file...
        file.deleteFile();
        ScopedPointer<FileOutputStream> fileStream (file.createOutputStream());

        if (fileStream != nullptr)
        {
            // Now create a WAV writer object that writes to our output stream...
            WavAudioFormat wavFormat;
            AudioFormatWriter* writer = wavFormat.createWriterFor (fileStream, sampleRate, 1, 16, StringPairArray(), 0);

            if (writer != nullptr)
            {
                fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                // write the data to disk on our background thread.
                threadedWriter = new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768);

                nextSampleNum = 0;

                // And now, swap over our active writer pointer so that the audio callback will start using it..
                const ScopedLock sl (writerLock);
                activeWriter = threadedWriter;
            }
        }
    }
}

void AudioRecorder::stop()
{
    // First, clear this pointer to stop the audio callback from using our writer object..
    {
        const ScopedLock sl (writerLock);
        activeWriter = nullptr;
    }

    // Now we can delete the writer object. It's done in this order because the deletion could
    // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
    // the audio callback while this happens.
    threadedWriter = nullptr;
}

bool AudioRecorder::isRecording() const
{
    return activeWriter != nullptr;
}


void AudioRecorder::audioDeviceAboutToStart(AudioIODevice* device)
{
    sampleRate = device->getCurrentSampleRate();
    if (sampleRate != FS_MIR) {
        printf("sample rate has problem!\n");
    }
    audioAnalyzer->Clear();
}

void AudioRecorder::audioDeviceStopped()
{
    for (int i=0; i<audioAnalyzer->feature_size; i++) {
        audioAnalyzer->SubbandAnalysis(audioAnalyzer->subband_signals[i], i+audioAnalyzer->min_note);
    }
//    for (TimedNotes::iterator it = audioAnalyzer->audio_notes.begin(); it != audioAnalyzer->audio_notes.end(); it++) {
//        printf("%f:", it->first);
//        for (std::vector<struct Note>::iterator kt=it->second.begin(); kt!=it->second.end(); kt++) {
//            printf("%u,%f ", kt->midi_pitch, kt->valence);
//        }
//        printf("\n");
//    }
    float grade = audioAnalyzer->AudioScoreAlignment();
    printf("grade=%f\n", grade);
}

void AudioRecorder::audioDeviceIOCallback (const float** inputChannelData, int /*numInputChannels*/,
                            float** outputChannelData, int numOutputChannels,
                            int numSamples)
{
    const ScopedLock sl (writerLock);
    
    if (activeWriter != nullptr)
    {
        activeWriter->write (inputChannelData, numSamples);

        // Create an AudioSampleBuffer to wrap our incomming data, note that this does no allocations or copies, it simply references our input data
        const AudioSampleBuffer buffer (const_cast<float**> (inputChannelData), 1, numSamples);
        nextSampleNum += numSamples;
        
        if (numSamples != audioAnalyzer->hop_size) {
            printf("record block size has problem!\n");
        }
        
        audioAnalyzer->UpdateFrameBuffer(inputChannelData[0], numSamples);
        audioAnalyzer->FrameAnalysis(audioAnalyzer->frame_buffer);
    }

    // We need to clear the output buffers, in case they're full of junk..
    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear (outputChannelData[i], numSamples);
}
