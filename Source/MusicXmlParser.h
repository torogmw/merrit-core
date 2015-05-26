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

struct NoteUnit {
    std::string pitch;  // G3, B4 etc, or we can use a midi note?
    int noteType;
    int onsetTimeMs; //the start of the note, should we use time frame?
    float dynamics;
    float voice; // not sure what this is
    int measureIndex; // count what is the measure in this
};


class MusicXmlParser
{
public:
    MusicXmlParser(const File &xmlFile);
    ~MusicXmlParser();
    int countNotes() const;
    void parseScoreTimewise();
    void parseScorePartwise();

private:
    ScopedPointer<XmlElement> mainScoreElement;
    int noteCount;
    std::vector<NoteUnit> notes;
};

#endif /* defined(__merrit_core__MusicXmlParser__) */
