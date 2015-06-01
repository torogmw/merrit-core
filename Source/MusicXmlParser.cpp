//
//  MusicXmlParser.cpp
//  merrit-core
//
//  Created by Minwei Gu on 5/24/15.
//
//

#include "MusicXmlParser.h"


MusicXmlParser :: MusicXmlParser(const File &xmlFile)
{
    mainScoreElement = new XmlElement(*XmlDocument :: parse(xmlFile));
    String scoreType = mainScoreElement -> getTagName();
    if (scoreType == "score-partwise") {
        // parse the partwise score
        parseScorePartwise();
        
    } else if (scoreType == "score-timewise") {
        // parse the timewise score
    } else {
        std::cout<< "parse error" <<std::endl;
    }
    
    // add midi note hashmap
    midiBase["C"] = 0;
    midiBase["C#"] = 1;
    midiBase["D"] = 2;
    midiBase["D#"] = 3;
    midiBase["E"] = 4;
    midiBase["F"] = 5;
    midiBase["F#"] = 6;
    midiBase["G"] = 7;
    midiBase["G#"] = 8;
    midiBase["A"] = 9;
    midiBase["A#"] = 10;
    midiBase["B"] = 11;
    
}

MusicXmlParser :: ~MusicXmlParser()
{
    noteCount = 0;
    mainScoreElement = nullptr;
}

int MusicXmlParser :: countNotes() const
{
    return noteCount;
}

void MusicXmlParser::parseScorePartwise()
{
    // first, identify instrument and parse global metadata
    ScopedPointer<XmlElement> insElement = mainScoreElement -> getChildByName("part-list");
    globalMeasureLength = 12;
    globalTempo = 80;  // hardcode for now
    
    // second, get the core Eleement and count the measures
    ScopedPointer<XmlElement> coreElement = mainScoreElement -> getChildByName("part");
    int measureCount = coreElement -> getNumChildElements(); // get number of measures
    XmlElement* measureElement;
    XmlElement* noteElement;
    for (int i = 0; i < measureCount; i++)
    {
        // for each measure, store all the note / volume / duration into a vector of hashmap
        float measureIndex = float(i+1);
        measureElement = coreElement -> getChildElement(i);
        int numElementPerMeasure = measureElement -> getNumChildElements();
        for (int j = 0; j<numElementPerMeasure; j++)
        {
            if (measureElement->getChildElement(j)->getTagName() == "note") {
                std::cout<<"note!"<<std::endl;
                noteElement = measureElement->getChildElement(j);
                measureIndex = generateNoteUnit(noteElement, measureIndex); // update the measure index
            } else if (measureElement->getChildElement(j)->getTagName() == "backup") {
                noteElement = measureElement->getChildElement(j);
                String backDuration = noteElement->getChildByName("duration")->getAllSubText();
                measureIndex -= std::stof(backDuration.toStdString()) / globalMeasureLength;
                std::cout<<"backup!"<<std::endl;
            } else {
                std::cout<<"fuck!"<<std::endl;
            }
        }
    }
    noteElement = nullptr;
    measureElement = nullptr;
}

void MusicXmlParser::parseScoreTimewise()
{
    
}

float MusicXmlParser::generateNoteUnit(XmlElement* noteElement, float measureIndex)
{
    NoteUnit noteUnit;
    if (noteElement->getChildByName("pitch"))
    {
        // this is a pitch
        String step = noteElement->getChildByName("pitch")->getChildByName("step")->getAllSubText();
        String octave = noteElement->getChildByName("pitch")->getChildByName("octave")->getAllSubText();
        String duration = noteElement->getChildByName("duration")->getAllSubText();
        String voice = noteElement->getChildByName("voice")->getAllSubText();
        String type = noteElement->getChildByName("type")->getAllSubText();
        noteUnit.pitch = pitchToMidi(step, std::stoi(octave.toStdString()));
        noteUnit.duration = std::stoi(duration.toStdString());
        noteUnit.onsetTime = measureIndex;
        noteUnit.voice = std::stoi(voice.toStdString());
        noteUnit.dynamics = 0;  // set to 0 for now
        notes.push_back(noteUnit);
        float newMeasureIndex = measureIndex + float(noteUnit.duration) / globalMeasureLength;
        return newMeasureIndex;
    } else {
        return measureIndex;
    }
}

std::vector<NoteUnit> MusicXmlParser::getNotes()
{
    return notes;
}

int MusicXmlParser::pitchToMidi(String pitch, int octave)
{
    int isSharp = 0;
    
    //todo: add sharp
    return (octave + 1) * 12 + midiBase[pitch] + isSharp;
    
}
