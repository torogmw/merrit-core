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

#ifndef __JUCE_HEADER_E670A1777E98D4A2__
#define __JUCE_HEADER_E670A1777E98D4A2__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
#include "AudioInputSource.h"
#include "AudioRecorder.h"
#include "MusicXmlParser.h"
#include <GUIDOEngine/GUIDOEngine.h>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class PlaybackUI  : public Component,
                    public ButtonListener
{
public:
    //==============================================================================
    PlaybackUI ();
    ~PlaybackUI();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void startRecording();
    void stopRecording();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    AudioDeviceManager deviceManager;
    juce::AudioDeviceManager::AudioDeviceSetup audioDeviceSetup;
    ScopedPointer<AudioInputSource> inputSource;
    ScopedPointer<AudioRecorder> recorder;
    ScopedPointer<MusicXmlParser> notation;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextButton> playStopButton;
    ScopedPointer<TextButton> loadButton;
    ScopedPointer<Label> resultLabel;
    ScopedPointer<Label> title;
    ScopedPointer<TextButton> recordButton;
    ScopedPointer<TextButton> xmlButton;
    ScopedPointer<WebBrowserComponent> webBrowserComponent;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackUI)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_E670A1777E98D4A2__
