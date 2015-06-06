//
//  AudioRecorder.cpp
//  merrit-core
//
//  Created by Ruofeng Chen on 6/6/15.
//
//

#include "Song.h"

/* before xmlparser is done, use hard-code data */
Song::Song()
{
    int i;
    int num_segments = 1;
    segments = std::vector<Segment>(num_segments);
    
    int s0_num_notes = 6;
    int s0_midi_pitches[] = {71, 56, 59, 64, 59, 56};
    float s0_times[] = {1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[0].scoreForAnalyzer = std::vector<struct Note>(s0_num_notes);
    segments[0].timesForAnalyzer = std::vector<float>(s0_num_notes);
    segments[0].scoreForDisplay = "b/4,8;g/3,8,#;b/3,8;e/4,8;b/3,8;g/3,8,#";
    for (i=0; i<s0_num_notes; i++) {
        segments[0].scoreForAnalyzer[i].midi_pitch = s0_midi_pitches[i];
        segments[0].timesForAnalyzer[i] = s0_times[i];
    }
}

Song::~Song() {}