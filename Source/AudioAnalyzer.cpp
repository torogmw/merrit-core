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
    aubio_onset_t *onset_mir = new_aubio_onset(onset_method, frame_size, hop_size, fs);
    fvec_t *onset = new_fvec(1);
    smpl_t onset_threshold = 0.;
    
    char_t * pitch_method = "default";
    aubio_pitch_t *pitch_mir = new_aubio_pitch (pitch_method, frame_size * 4, hop_size, fs);
    fvec_t *pitch = new_fvec(1);
    smpl_t pitch_tolerance = 0.;
    
    smpl_t silence_threshold = -90.;
    
    fvec_t *buffer = new_fvec(frame_size);
    
    for (int i=0; i<num_frames; i++) {
        buffer->data = (smpl_t *)audio + i*hop_size;
        smpl_t curlevel = aubio_level_detection(buffer, silence_threshold);
        aubio_onset_do(onset_mir, buffer, onset);
        aubio_pitch_do (pitch_mir, buffer, pitch);
        printf("onset:%f pitch:%f level:%f\n", fvec_get_sample(onset, 0), fvec_get_sample(pitch, 0), curlevel);
    }
    
    del_aubio_onset(onset_mir);
    del_aubio_pitch(pitch_mir);

}

AudioAnalyzer::~AudioAnalyzer()
{
}

