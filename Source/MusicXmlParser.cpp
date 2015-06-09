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
    
    // add key signature map, you are fucking crazy to parse anythong beyond +/- 5
    // and we don't care about major, minor shit now
    // follow by this: http://en.wikipedia.org/wiki/Circle_of_fifths
    keyMap[-5] = std::vector<int>{0,-1,-1,0,-1,-1,-1}; // Db - bb
    keyMap[-4] = std::vector<int>{0,-1,-1,0,0,-1,-1};  // Ab - f
    keyMap[-3] = std::vector<int>{0,0,-1,0,0,-1,-1};   // Eb - c
    keyMap[-2] = std::vector<int>{0,0,-1,0,0,0,-1};    // Bb - g
    keyMap[-1] = std::vector<int>{0,0,0,0,0,0,-1};     // F - d
    keyMap[0] = std::vector<int>{0,0,0,0,0,0,0};       // C - a
    keyMap[1] = std::vector<int>{0,0,0,1,0,0,0};       // G - e
    keyMap[2] = std::vector<int>{1,0,0,1,0,0,0};       // D - b
    keyMap[3] = std::vector<int>{1,0,0,1,1,0,0};       // A - f#
    keyMap[4] = std::vector<int>{1,1,0,1,1,0,0};       // E - c#
    keyMap[5] = std::vector<int>{1,1,0,1,1,1,0};       // B - g#
    
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
    ScopedPointer<XmlElement> insElement = mainScoreElement -> getChildByName("part-list");
    ScopedPointer<XmlElement> coreElement = mainScoreElement -> getChildByName("part");
    int measureCount = coreElement -> getNumChildElements(); // get number of measures
    XmlElement* measureElement;
    XmlElement* attributeElement;
    XmlElement* noteElement;

    // first, identify instrument and parse global metadata
    globalTempo = 80;  // hard code tempo for now
    keySignature = 0;
    globalMeasureLength = 0;
    for (int i = 0; i < measureCount; i++)
    {
        float measureIndex = float(i+1);
        measureElement = coreElement -> getChildElement(i);
        int numElementPerMeasure = measureElement -> getNumChildElements();
        for (int j = 0; j<numElementPerMeasure; j++)
        {
            if (measureElement->getChildElement(j)->getTagName() == "attributes") {
                std::cout<<"global metadata!" << std::endl;
                generateGlobalAttribute(measureElement->getChildElement(j));
                break;
            }
        }
    }
    // second, get the core Eleement and count the measures
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
                std::cout<<"backup!"<<std::endl;
                noteElement = measureElement->getChildElement(j);
                String backDuration = noteElement->getChildByName("duration")->getAllSubText();
                measureIndex -= std::stof(backDuration.toStdString()) / globalMeasureLength;
            } else {
                std::cout<<"fuck!"<<std::endl;
            }
        }
    }
    noteElement = nullptr;
    measureElement = nullptr;
    attributeElement = nullptr;
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
        String alter = "0";
        if (noteElement->getChildByName("pitch")->getChildByName("alter"))  // has flat/sharp
        {
            alter = noteElement->getChildByName("pitch")->getChildByName("alter")->getAllSubText();
        }
        String duration = noteElement->getChildByName("duration")->getAllSubText();
        String voice = noteElement->getChildByName("voice")->getAllSubText();
        String type = noteElement->getChildByName("type")->getAllSubText();
        noteUnit.pitch = pitchToMidi(step, std::stoi(octave.toStdString()), std::stoi(alter.toStdString()));
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

void MusicXmlParser::generateGlobalAttribute(XmlElement* attributeElement)
{
    String key = attributeElement -> getChildByName("key")->getChildByName("fifths")->getAllSubText();
    String beatPerMeasure = attributeElement -> getChildByName("time")->getChildByName("beats")->getAllSubText();
    String beatType = attributeElement -> getChildByName("time")->getChildByName("beat-type")->getAllSubText();
    keySignature = std::stoi(key.toStdString());
    globalMeasureLength = std::stoi(beatType.toStdString()) * std::stoi(beatPerMeasure.toStdString());
    std::cout<<"key: "<<keySignature<<" length per measure: "<<globalMeasureLength<<std::endl;
}

std::vector<NoteUnit> MusicXmlParser::getNotes()
{
    return notes;
}

int MusicXmlParser::pitchToMidi(String pitch, int octave, int alter)
{
    return (octave + 1) * 12 + midiBase[pitch] + alter;
}
