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

    addAndMakeVisible (recordButton = new TextButton ("record button"));
    recordButton->setButtonText (TRANS("Record"));
    recordButton->addListener (this);
    recordButton->setColour (TextButton::buttonColourId, Colours::red);

    addAndMakeVisible (xmlButton = new TextButton ("xml button"));
    xmlButton->setButtonText (TRANS("readXML"));
    xmlButton->addListener (this);

    addAndMakeVisible (webBrowserComponent = new WebBrowserComponent());
    webBrowserComponent->setName ("web browser component");

    addAndMakeVisible (demo = new TextButton ("demo"));
    demo->addListener (this);
    demo->setColour (TextButton::buttonColourId, Colour (0xff91f24b));

    addAndMakeVisible (bpmTextbox = new TextEditor ("bpm textbox"));
    bpmTextbox->setMultiLine (false);
    bpmTextbox->setReturnKeyStartsNewLine (false);
    bpmTextbox->setReadOnly (false);
    bpmTextbox->setScrollbarsShown (true);
    bpmTextbox->setCaretVisible (true);
    bpmTextbox->setPopupMenuEnabled (true);
    bpmTextbox->setText (TRANS("120"));

    addAndMakeVisible (bar_textbox = new TextEditor ("bar textbox"));
    bar_textbox->setMultiLine (false);
    bar_textbox->setReturnKeyStartsNewLine (false);
    bar_textbox->setReadOnly (false);
    bar_textbox->setScrollbarsShown (true);
    bar_textbox->setCaretVisible (true);
    bar_textbox->setPopupMenuEnabled (true);
    bar_textbox->setText (TRANS("0"));

    addAndMakeVisible (label = new Label ("new label",
                                          TRANS("BPM")));
    label->setFont (Font (15.00f, Font::plain));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (label2 = new Label ("new label",
                                           TRANS("bar #")));
    label2->setFont (Font (15.00f, Font::plain));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));


    //[UserPreSize]
    playStopButton->setVisible(false);        // invisible by default
    recordButton->setVisible(false);
    loadButton->setVisible(false);
    //[/UserPreSize]

    setSize (640, 360);


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
    recorder = new AudioRecorder(audioAnalyzer, resultLabel);
    beatTimer = new BeatTimer(this);
    current_measure_index = 0;
    //[/Constructor]
}

PlaybackUI::~PlaybackUI()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    playStopButton = nullptr;
    loadButton = nullptr;
    resultLabel = nullptr;
    recordButton = nullptr;
    xmlButton = nullptr;
    webBrowserComponent = nullptr;
    demo = nullptr;
    bpmTextbox = nullptr;
    bar_textbox = nullptr;
    label = nullptr;
    label2 = nullptr;


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

    playStopButton->setBounds (240, 320, 38, 24);
    loadButton->setBounds (192, 320, 38, 24);
    resultLabel->setBounds (296, 320, 152, 32);
    recordButton->setBounds (24, 320, 48, 24);
    xmlButton->setBounds (80, 320, 56, 24);
    webBrowserComponent->setBounds (0, 0, 624, 296);
    demo->setBounds (144, 320, 39, 24);
    bpmTextbox->setBounds (544, 320, 64, 24);
    bar_textbox->setBounds (472, 320, 64, 24);
    label->setBounds (544, 296, 32, 24);
    label2->setBounds (472, 296, 40, 24);
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
            song = Song(notation->getMeasures());
            current_measure_index = bar_textbox->getText().getIntValue();
            displayAndAnalyzeScore();
            recordButton->setVisible(true);
            loadButton->setVisible(true);
            beatTimer->setTimer(bpmTextbox->getText().getIntValue(), 3); // hard-code time-signature for now
        }
        //[/UserButtonCode_xmlButton]
    }
    else if (buttonThatWasClicked == demo)
    {
        //[UserButtonCode_demo] -- add your button handler code here..
        song = Song();
        current_measure_index = bar_textbox->getText().getIntValue();
        displayAndAnalyzeScore();
        recordButton->setVisible(true);
        loadButton->setVisible(true);
        beatTimer->setTimer(bpmTextbox->getText().getIntValue(), 3); // hard-code time-signature for now
        
        //[/UserButtonCode_demo]
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
    beatTimer->startTimer();
    getEverythingReadyForMeasure(getCurrentMeasure());
}

void PlaybackUI::stopRecording()
{
    for (int i=0; i<audioAnalyzer->feature_size; i++) {
        audioAnalyzer->SubbandAnalysis(audioAnalyzer->subband_signals[i], i+audioAnalyzer->min_note);
    }
    float grade = audioAnalyzer->AudioScoreAlignment();
    printf("grade=%f\n", grade);
    resultLabel->setText(String(grade), dontSendNotification);
    recorder->stop();
    recordButton->setButtonText ("Record");
    deviceManager.removeAudioCallback(recorder);
    beatTimer->stopTimer();
}

int PlaybackUI::getCurrentMeasure()
{
    return current_measure_index;
}

void PlaybackUI::getEverythingReadyForMeasure(int measure_index)
{
    if (audioAnalyzer->frame_num > 0) {
        for (int i=0; i<audioAnalyzer->feature_size; i++) {
            audioAnalyzer->SubbandAnalysis(audioAnalyzer->subband_signals[i], i+audioAnalyzer->min_note);
        }
        float grade = audioAnalyzer->AudioScoreAlignment();
        printf("grade=%f\n", grade);
        resultLabel->setText(String(grade), dontSendNotification);
    }

    current_measure_index = measure_index;
    displayAndAnalyzeScore();
}

void PlaybackUI::displayAndAnalyzeScore()
{
    std::string concat_score = song.segments[current_measure_index].scoreForDisplay;
    String encoded_score = URL::addEscapeChars(song.scoreHeader + concat_score, true);
    String s = "file://" + File::getCurrentWorkingDirectory().getFullPathName() + "/../../../../Webpages/index.html?score=" + encoded_score;
//    printf("%ls\n", s.toWideCharPointer());
    webBrowserComponent->goToURL(s);
    audioAnalyzer->SetScore(song.segments[current_measure_index].scoreForAnalyzer, song.segments[current_measure_index].timesForAnalyzer);
}

void PlaybackUI::progressToNextMeasure()
{
    if (current_measure_index < song.segments.size()) {
        current_measure_index++;
    }
    if (current_measure_index == song.segments.size()) {
        current_measure_index--; // stay at the last measure
        stopRecording();
    }
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
                 fixedSize="1" initialWidth="640" initialHeight="360">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTBUTTON name="play / stop" id="691427fc69b5adc6" memberName="playStopButton"
              virtualName="" explicitFocusOrder="0" pos="240 320 38 24" buttonText="play / stop"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="load button" id="1d59d8ea76ceba03" memberName="loadButton"
              virtualName="" explicitFocusOrder="0" pos="192 320 38 24" buttonText="load"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="result label" id="f3671d4a4efe8c7b" memberName="resultLabel"
         virtualName="" explicitFocusOrder="0" pos="296 320 152 32" edTextCol="ff000000"
         edBkgCol="0" labelText="result" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="12"/>
  <TEXTBUTTON name="record button" id="477e06bd4f00f984" memberName="recordButton"
              virtualName="" explicitFocusOrder="0" pos="24 320 48 24" bgColOff="ffff0000"
              buttonText="Record" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="xml button" id="25fd44335f65258a" memberName="xmlButton"
              virtualName="" explicitFocusOrder="0" pos="80 320 56 24" buttonText="readXML"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <GENERICCOMPONENT name="web browser component" id="6d771a11f7ef330e" memberName="webBrowserComponent"
                    virtualName="" explicitFocusOrder="0" pos="0 0 624 296" class="WebBrowserComponent"
                    params=""/>
  <TEXTBUTTON name="demo" id="85cd60f80316b64e" memberName="demo" virtualName=""
              explicitFocusOrder="0" pos="144 320 39 24" bgColOff="ff91f24b"
              buttonText="demo" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTEDITOR name="bpm textbox" id="bb7d1b8c7fdbb5ed" memberName="bpmTextbox"
              virtualName="" explicitFocusOrder="0" pos="544 320 64 24" initialText="120"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="bar textbox" id="66646c0e4ae4e4fd" memberName="bar_textbox"
              virtualName="" explicitFocusOrder="0" pos="472 320 64 24" initialText="0"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="new label" id="27d9be3876fa4763" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="544 296 32 24" edTextCol="ff000000"
         edBkgCol="0" labelText="BPM" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="70729000d345ad44" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="472 296 40 24" edTextCol="ff000000"
         edBkgCol="0" labelText="bar #" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
