//
//  AudioAnalyzer.h
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#ifndef __merrit_core__AudioAnalyzer__
#define __merrit_core__AudioAnalyzer__

#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include "rsrfft.h"
#include "Song.h"

#define FS_MIR              44100.0
#define FRAME_TIME          100.0 // in ms
#define MIN_FFT_POINT       1024
#define MIN_FFT_ORDER       10
#define ZERO_PADDING_RATE   0.9
#define MIN_FREQ            80.0 // in Hz
#define MAX_FREQ            3000.0
#define NUM_NOTES           120
#define FREQREF             7.943049790954413
#define _2ROOT12            1.059463094
#define LOG_2ROOT12         0.057762265 // log(_2ROOT12)
#define SPECTRAL_FLUX_SIZE  5

typedef std::map<float, NotesAtTime> TimedNotes;

class AudioAnalyzer
{
public:
    AudioAnalyzer(float fs, uint32_t block_size);
    ~AudioAnalyzer();
    int UpdateFrameBuffer(const float *new_buffer, uint32_t buffer_size);
    int FrameAnalysis(const float *buffer);
    int FrameAnalysis(const float *buffer, float *out);
    int SubbandAnalysis(std::vector<float> &subband_signal, uint32_t midi_note); // find notes
    int SetScore(std::vector<struct Note> notes, std::vector<float> times);
    float AudioScoreAlignment();
    int Clear();
    float fs;
    uint32_t frame_size;
    uint32_t hop_size;
    uint32_t fft_point;
    uint32_t min_bin;
    uint32_t max_bin;
    uint32_t min_note;
    uint32_t max_note;
    uint32_t spectrum_size;
    uint32_t *FFT_bin_2_MIDI_note_mapping; // which MIDI note does a bin belong to?
    float MIDI_note_width[NUM_NOTES]; // how wide in a MIDI note?
    float *hammingWin;
    SplitRadixFFT *fft;
    uint32_t feature_size;
    float *frame_buffer;
    std::vector<float> *subband_signals;
    uint32_t frame_num;
    TimedNotes audio_notes;
    TimedNotes score_notes;
};

#endif /* defined(__merrit_core__AudioAnalyzer__) */
