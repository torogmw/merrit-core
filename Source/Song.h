//
//  AudioRecorder.h
//  merrit-core
//
//  Created by Ruofeng Chen on 6/6/15.
//
//

#ifndef __merrit_core__Song__
#define __merrit_core__Song__

#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "MusicXmlParser.h"

struct Note
{
    uint32_t midi_pitch;
    float valence;
};

typedef std::vector<Note> NotesAtTime;

struct Segment {
    std::vector<Note> scoreForAnalyzer;
    std::vector<float> timesForAnalyzer;
    std::string scoreForDisplay;
};

class Song {

public:
    Song(char *xmlfilename);
    Song(const std::vector<MeasureUnit>& measures);
    Song();
    ~Song();
    std::string title;
    std::string timeSignature;
    std::string keySignature;
    std::string scoreHeader;
    std::vector<Segment> segments;
};

#endif /* defined(__merrit_core__Song__) */
