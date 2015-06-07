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
    int num_segments = 4;
    segments = std::vector<Segment>(num_segments);
    
    int s0_num_notes = 6;
    int s0_midi_pitches[] = {83, 68, 71, 76, 71, 68};
    float s0_times[] = {1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[0].scoreForAnalyzer = std::vector<struct Note>(s0_num_notes);
    segments[0].timesForAnalyzer = std::vector<float>(s0_num_notes);
    segments[0].scoreForDisplay = "b/5,8;g/4,8,#;b/4,8;e/5,8;b/4,8;g/4,8,#";
    for (i=0; i<s0_num_notes; i++) {
        segments[0].scoreForAnalyzer[i].midi_pitch = s0_midi_pitches[i] - 12;
        segments[0].timesForAnalyzer[i] = s0_times[i];
    }
    
    int s1_num_notes = 6;
    int s1_midi_pitches[] = {83, 68, 75, 80, 75, 68};
    float s1_times[] = {1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[1].scoreForAnalyzer = std::vector<struct Note>(s1_num_notes);
    segments[1].timesForAnalyzer = std::vector<float>(s1_num_notes);
    segments[1].scoreForDisplay = "b/5,8;g/4,8,#;d/5,8,#;g/5,8,#;d/5,8,#;g/4,8,#";
    for (i=0; i<s1_num_notes; i++) {
        segments[1].scoreForAnalyzer[i].midi_pitch = s1_midi_pitches[i] - 12;
        segments[1].timesForAnalyzer[i] = s1_times[i];
    }
    
    int s2_num_notes = 7;
    int s2_midi_pitches[] = {81, 83, 85, 76, 81, 76, 73};
    float s2_times[] = {1.0/6.0, 1.0/6.0+1.0/12.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[2].scoreForAnalyzer = std::vector<struct Note>(s2_num_notes);
    segments[2].timesForAnalyzer = std::vector<float>(s2_num_notes);
    segments[2].scoreForDisplay = "a/5,16;b/5,16;c/6,8,#;e/5,8;a/5,8;e/5,8;c/5,8,#";
    for (i=0; i<s2_num_notes; i++) {
        segments[2].scoreForAnalyzer[i].midi_pitch = s2_midi_pitches[i] - 12;
        segments[2].timesForAnalyzer[i] = s2_times[i];
    }
    
    int s3_num_notes = 6;
    int s3_midi_pitches[] = {66, 73, 76, 81, 76, 73};
    float s3_times[] = {1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[3].scoreForAnalyzer = std::vector<struct Note>(s3_num_notes);
    segments[3].timesForAnalyzer = std::vector<float>(s3_num_notes);
    segments[3].scoreForDisplay = "f/4,8,#;c/5,8,#;e/5,8;a/5,8;e/5,8;c/5,8,#";
    for (i=0; i<s3_num_notes; i++) {
        segments[3].scoreForAnalyzer[i].midi_pitch = s3_midi_pitches[i] - 12;
        segments[3].timesForAnalyzer[i] = s3_times[i];
    }
}

Song::~Song() {}