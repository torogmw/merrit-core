/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.0

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PlaybackUI.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
PlaybackUI::PlaybackUI ()
{
    addAndMakeVisible (playStopButton = new TextButton ("play / stop"));
    playStopButton->addListener (this);

    addAndMakeVisible (loadButton = new TextButton ("load button"));
    loadButton->setButtonText (TRANS("load"));
    loadButton->addListener (this);

    addAndMakeVisible (resultLabel = new Label ("result label",
                                                TRANS("result")));
    resultLabel->setFont (Font (15.00f, Font::plain));
    resultLabel->setJustificationType (Justification::centredTop);
    resultLabel->setEditable (false, false, false);
    resultLabel->setColour (TextEditor::textColourId, Colours::black);
    resultLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (title = new Label ("title",
                                          TRANS("Audio PoC 0.1")));
    title->setFont (Font ("Marion", 19.50f, Font::bold));
    title->setJustificationType (Justification::centred);
    title->setEditable (false, false, false);
    title->setColour (TextEditor::textColourId, Colours::black);
    title->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (recordButton = new TextButton ("record button"));
    recordButton->setButtonText (TRANS("Record"));
    recordButton->addListener (this);

    addAndMakeVisible (xmlButton = new TextButton ("xml button"));
    xmlButton->setButtonText (TRANS("readXML"));
    xmlButton->addListener (this);

    addAndMakeVisible (webBrowserComponent = new WebBrowserComponent());
    webBrowserComponent->setName ("web browser component");


    //[UserPreSize]
    playStopButton->setVisible(false);        // invisible by default
    //[/UserPreSize]

    setSize (360, 640);


    //[Constructor] You can add your own custom stuff here..
    audioDeviceSetup.bufferSize = 512; // 0.04s if sample rate is 11.025kHz
    deviceManager.initialise(1, /* number of input channels */
                             2, /* number of output channels */
                             0, /* no XML settings*/
                             true, /* select default device on failure */
                             String::empty, /* preferred device name */
                             &audioDeviceSetup /* preferred setup options */);
    audioAnalyzer = new AudioAnalyzer(FS_MIR, 512);
    inputSource = new AudioInputSource(deviceManager, audioAnalyzer);
    recorder = new AudioRecorder(audioAnalyzer);
    
    String score = "b/4,8;g/3,8,#;b/3,8;e/4,8;b/3,8;g/3,8,#";
    String encoded_score = URL::addEscapeChars(score, true);
    String s = "file://" + File::getCurrentWorkingDirectory().getFullPathName() + "/../../../../Webpages/index.html?score=" + encoded_score;
    printf("%ls\n", s.toWideCharPointer());
    webBrowserComponent->goToURL(s);
    //[/Constructor]
}

PlaybackUI::~PlaybackUI()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    playStopButton = nullptr;
    loadButton = nullptr;
    resultLabel = nullptr;
    title = nullptr;
    recordButton = nullptr;
    xmlButton = nullptr;
    webBrowserComponent = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void PlaybackUI::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void PlaybackUI::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    playStopButton->setBounds (104, 240, 150, 24);
    loadButton->setBounds (104, 120, 150, 24);
    resultLabel->setBounds (104, 296, 152, 32);
    title->setBounds (64, 24, 224, 40);
    recordButton->setBounds (104, 72, 150, 24);
    xmlButton->setBounds (104, 176, 150, 24);
    webBrowserComponent->setBounds (8, 352, 344, 224);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void PlaybackUI::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == playStopButton)
    {
        //[UserButtonCode_playStopButton] -- add your button handler code here..
        inputSource->filePlayingControl();
        //[/UserButtonCode_playStopButton]
    }
    else if (buttonThatWasClicked == loadButton)
    {
        //[UserButtonCode_loadButton] -- add your button handler code here..
        // init the file input here
        FileChooser chooser (("Choose audio file to open"),File::getSpecialLocation(File::userMusicDirectory),"*",true);
        chooser.browseForFileToOpen();
        if(chooser.getResult().exists())
        {
            inputSource->setFile(chooser.getResult());
            resultLabel->setText("File choosed", dontSendNotification);
            playStopButton->setVisible(true);
        }
        //[/UserButtonCode_loadButton]
    }
    else if (buttonThatWasClicked == recordButton)
    {
        //[UserButtonCode_recordButton] -- add your button handler code here..
        if (recordButton->getButtonText().compare("Record") == 0)
            startRecording();
        else if (recordButton->getButtonText().compare("Stop") == 0)
            stopRecording();
        //[/UserButtonCode_recordButton]
    }
    else if (buttonThatWasClicked == xmlButton)
    {
        //[UserButtonCode_xmlButton] -- add your button handler code here..
        FileChooser chooser (("Choose audio file to open"),File::getSpecialLocation(File::userDocumentsDirectory),"*",true);
        chooser.browseForFileToOpen();
        if(chooser.getResult().exists())
        {
            notation = new MusicXmlParser(chooser.getResult());
            std::vector<NoteUnit> notes = notation->getNotes();
            for (std::vector<NoteUnit>::iterator it = notes.begin(); it != notes.end(); it++) {
                printf("%d\n", it->pitch);
            }
        }
        //[/UserButtonCode_xmlButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void PlaybackUI::startRecording()
{
    deviceManager.addAudioCallback(recorder);
    const File file (File::getSpecialLocation (File::userMusicDirectory)
                     .getNonexistentChildFile ("practice", ".wav"));
    recorder->startRecording (file);
    recordButton->setButtonText ("Stop");
}

void PlaybackUI::stopRecording()
{
    recorder->stop();
    recordButton->setButtonText ("Record");
    deviceManager.removeAudioCallback(recorder);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PlaybackUI" componentName=""
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="360" initialHeight="640">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTBUTTON name="play / stop" id="691427fc69b5adc6" memberName="playStopButton"
              virtualName="" explicitFocusOrder="0" pos="104 240 150 24" buttonText="play / stop"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="load button" id="1d59d8ea76ceba03" memberName="loadButton"
              virtualName="" explicitFocusOrder="0" pos="104 120 150 24" buttonText="load"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="result label" id="f3671d4a4efe8c7b" memberName="resultLabel"
         virtualName="" explicitFocusOrder="0" pos="104 296 152 32" edTextCol="ff000000"
         edBkgCol="0" labelText="result" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="12"/>
  <LABEL name="title" id="8c8b576e0a1edba7" memberName="title" virtualName=""
         explicitFocusOrder="0" pos="64 24 224 40" edTextCol="ff000000"
         edBkgCol="0" labelText="Audio PoC 0.1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Marion"
         fontsize="19.5" bold="1" italic="0" justification="36"/>
  <TEXTBUTTON name="record button" id="477e06bd4f00f984" memberName="recordButton"
              virtualName="" explicitFocusOrder="0" pos="104 72 150 24" buttonText="Record"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="xml button" id="25fd44335f65258a" memberName="xmlButton"
              virtualName="" explicitFocusOrder="0" pos="104 176 150 24" buttonText="readXML"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="web browser component" id="6d771a11f7ef330e" memberName="webBrowserComponent"
                    virtualName="" explicitFocusOrder="0" pos="8 352 344 224" class="WebBrowserComponent"
                    params=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
