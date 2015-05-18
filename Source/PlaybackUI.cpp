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

    addAndMakeVisible (loadButton = new TextButton ("record button"));
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


    //[UserPreSize]
    playStopButton->setVisible(false);        // invisible by default
    //[/UserPreSize]

    setSize (360, 640);


    //[Constructor] You can add your own custom stuff here..
    deviceManager.initialise(2, /* number of input channels */
                             2, /* number of output channels */
                             0, /* no XML settings*/
                             true, /* select default device on failure */
                             String::empty, /* preferred device name */
                             0 /* preferred setup options */);
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

    playStopButton->setBounds (104, 168, 150, 24);
    loadButton->setBounds (104, 112, 150, 24);
    resultLabel->setBounds (104, 256, 152, 224);
    title->setBounds (64, 24, 224, 40);
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
        inputSource= nullptr;
        // init the file input here
        FileChooser chooser (("Choose audio file to open"),File::nonexistent,"*",true);
        chooser.browseForFileToOpen();
        if(chooser.getResult().exists())
        {
            inputSource = new AudioInputSource(deviceManager,1);
            inputSource->setFile(chooser.getResult());
            resultLabel->setText("File choosed", dontSendNotification);
            playStopButton->setVisible(true);
        }
        //[/UserButtonCode_loadButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

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
              virtualName="" explicitFocusOrder="0" pos="104 168 150 24" buttonText="play / stop"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="record button" id="1d59d8ea76ceba03" memberName="loadButton"
              virtualName="" explicitFocusOrder="0" pos="104 112 150 24" buttonText="load"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="result label" id="f3671d4a4efe8c7b" memberName="resultLabel"
         virtualName="" explicitFocusOrder="0" pos="104 256 152 224" edTextCol="ff000000"
         edBkgCol="0" labelText="result" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="12"/>
  <LABEL name="title" id="8c8b576e0a1edba7" memberName="title" virtualName=""
         explicitFocusOrder="0" pos="64 24 224 40" edTextCol="ff000000"
         edBkgCol="0" labelText="Audio PoC 0.1" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Marion"
         fontsize="19.5" bold="1" italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
