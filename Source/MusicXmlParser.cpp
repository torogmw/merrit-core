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
    // first, identify instrument
    ScopedPointer<XmlElement> insElement = mainScoreElement -> getChildByName("part-list");

    // second, get the core Eleement and count the measures
    ScopedPointer<XmlElement> coreElement = mainScoreElement -> getChildByName("part");
    int measureCount = coreElement -> getNumChildElements(); // get number of measures
    XmlElement* measureElement;
    for (int i = 0; i < measureCount; i++)
    {
        // for each measure, store all the note / volume / duration into a vector of hashmap
        measureElement = coreElement -> getChildElement(i);
        int elementPerMeasure = measureElement -> getNumChildElements();
        for (int j = 0; j<elementPerMeasure; j++)
        {
            if (measureElement->getChildElement(j)->getTagName() == "note") {
                std::cout<<"note!"<<std::endl;
                
            } else {
                std::cout<<"fuck!"<<std::endl;
            }
        }
    }
    measureElement = nullptr;
}

void MusicXmlParser::parseScoreTimewise()
{
    
}