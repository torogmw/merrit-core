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
    
    title = "Cavatina";
    keySignature = "emajor";
    timeSignature = "3/4";
    
    scoreHeader = "tabstave notation=true tablature=false key=E time=3/4\n";
    
    int num_segments = 4;
    segments = std::vector<Segment>(num_segments);
    
    int s0_num_notes = 7;
    int s0_midi_pitches[] = {52, 83, 68, 71, 76, 71, 68};
    float s0_times[] = {1.0/6.0, 1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[0].scoreForAnalyzer = std::vector<Note>(s0_num_notes);
    segments[0].timesForAnalyzer = std::vector<float>(s0_num_notes);
    segments[0].scoreForDisplay = "\nvoice\nnotes :hd B/5\nvoice\nnotes :8 E/3 G/4 B/4 E/5 B/4 G/4\n";
    for (i=0; i<s0_num_notes; i++) {
        segments[0].scoreForAnalyzer[i].midi_pitch = s0_midi_pitches[i] - 12;
        segments[0].timesForAnalyzer[i] = s0_times[i];
    }
    
    int s1_num_notes = 7;
    int s1_midi_pitches[] = {66, 83, 68, 75, 80, 75, 68};
    float s1_times[] = {1.0/6.0, 1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[1].scoreForAnalyzer = std::vector<struct Note>(s1_num_notes);
    segments[1].timesForAnalyzer = std::vector<float>(s1_num_notes);
    segments[1].scoreForDisplay = "voice\nnotes :hd B/5\nvoice\nnotes :8 D/4 G/4 D/5 G/5 D/5 G/4\n";
    for (i=0; i<s1_num_notes; i++) {
        segments[1].scoreForAnalyzer[i].midi_pitch = s1_midi_pitches[i] - 12;
        segments[1].timesForAnalyzer[i] = s1_times[i];
    }
    
    int s2_num_notes = 9;
    int s2_midi_pitches[] = {64, 81, 83, 73, 85, 76, 81, 76, 73};
    float s2_times[] = {1.0/6.0, 1.0/6.0, 1.0/6.0+1.0/12.0, 2.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[2].scoreForAnalyzer = std::vector<struct Note>(s2_num_notes);
    segments[2].timesForAnalyzer = std::vector<float>(s2_num_notes);
    segments[2].scoreForDisplay = "voice\nnotes :16 A/5 B/5 :8 C/6 :h C/6\nvoice\nnotes :8 C/4 C/5 E/5 A/5 E/5 C/5\n";
    for (i=0; i<s2_num_notes; i++) {
        segments[2].scoreForAnalyzer[i].midi_pitch = s2_midi_pitches[i] - 12;
        segments[2].timesForAnalyzer[i] = s2_times[i];
    }
    
    int s3_num_notes = 6;
    int s3_midi_pitches[] = {66, 73, 76, 81, 76, 73};
    float s3_times[] = {1.0/6.0, 2.0/6.0, 3.0/6.0, 4.0/6.0, 5.0/6.0, 6.0/6.0};
    segments[3].scoreForAnalyzer = std::vector<struct Note>(s3_num_notes);
    segments[3].timesForAnalyzer = std::vector<float>(s3_num_notes);
    segments[3].scoreForDisplay = "voice\nnotes :hd C/6\nvoice\nnotes :8 F/4 C/5 E/5 A/5 E/5 C/5\n";
    for (i=0; i<s3_num_notes; i++) {
        segments[3].scoreForAnalyzer[i].midi_pitch = s3_midi_pitches[i] - 12;
        segments[3].timesForAnalyzer[i] = s3_times[i];
    }
}

Song::Song(const std::vector<MeasureUnit>& measures) {
    if (measures.size() == 0) {
        return;
    }

    title = "Cavatina";
    keySignature = "emajor";
    timeSignature = "3/4";
    
    scoreHeader = "tabstave notation=true tablature=false key=E time=3/4\n";
    
    // why do we need this?
    int num_segments = measures.size();
    segments = std::vector<Segment>(num_segments);
    
    for (int i = 0; i<num_segments; i++) {
        int num_notes = measures[i].notes.size();
        segments[i].scoreForDisplay = measures[i].scoreForDisplay;
        // push pitch
        for (int j = 0; j<num_notes; j++) {
            Note note;
            note.midi_pitch = measures[i].notes[j].pitch;
            note.valence = measures[i].notes[j].dynamics;
            segments[i].scoreForAnalyzer.push_back(note);
            segments[i].timesForAnalyzer.push_back(measures[i].notes[j].onsetTime - i - 1); // minus measure index, who starts from 1
        }
    }
}

Song::~Song() {}