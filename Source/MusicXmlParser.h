//
//  MusicXmlParser.h
//  merrit-core
//
//  Created by Minwei Gu on 5/24/15.
//
//

#ifndef __merrit_core__MusicXmlParser__
#define __merrit_core__MusicXmlParser__

#include <stdio.h>
#include "JuceHeader.h"
#include <vector>
#include <string>
#include <map>

struct NoteUnit {
    int pitch;  // G3, B4 etc, or we can use a midi note? yes midi now
    int duration;
    float onsetTime; //the start of the note, should be measure+beat index
    float dynamics;
    float voice; // not sure what this is
};


class MusicXmlParser
{
public:
    MusicXmlParser(const File &xmlFile);
    ~MusicXmlParser();
    int countNotes() const;
    void parseScoreTimewise();
    void parseScorePartwise();
    float generateNoteUnit(XmlElement* noteElement, float measureIndex);
    std::vector<NoteUnit> getNotes();

private:
    int pitchToMidi(String pitch, int octave);
    
    ScopedPointer<XmlElement> mainScoreElement;
    int noteCount;
    int globalTempo;
    int globalMeasureLength;
    std::vector<NoteUnit> notes;
    std::map<String, int> midiBase;
};

#endif /* defined(__merrit_core__MusicXmlParser__) */
