//
//  AudioAnalyzer.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 5/18/15.
//
//

#include "AudioAnalyzer.h"

AudioAnalyzer::AudioAnalyzer(const float *audio_, uint32_t num_samples, float fs_, uint32_t frame_size_, uint32_t hop_size_)
{
    audio = audio_;
    fs = fs_;
    frame_size = frame_size_;
    hop_size = hop_size_;
    num_frames = (num_samples - frame_size) / hop_size + 1;

    char_t * onset_method = "default";
    aubio_onset_t *o = new_aubio_onset(onset_method, frame_size, hop_size, fs);
    fvec_t *onset = new_fvec(1);
    smpl_t onset_threshold = 0.;
    
    char_t * pitch_method = "default";
    aubio_pitch_t *pitch = new_aubio_pitch (pitch_method, frame_size * 4, hop_size, fs);
    fvec_t *pitch_obuf = new_fvec(1);
    smpl_t pitch_tolerance = 0.;
    
    fvec_t *note_buffer;
    fvec_t *note_buffer2;
    uint_t median = 6;
    if (median) {
        note_buffer = new_fvec (median);
        note_buffer2 = new_fvec (median);
    }
    
    fvec_t *buffer = new_fvec(frame_size);
    uint_t i = 0;
    buffer->data = (smpl_t *)audio + i;
    
    smpl_t new_pitch, curlevel;
    aubio_onset_do(o, buffer, onset);
    
    aubio_pitch_do (pitch, buffer, pitch_obuf);
    new_pitch = fvec_get_sample(pitch_obuf, 0);
}

AudioAnalyzer::~AudioAnalyzer()
{
}

