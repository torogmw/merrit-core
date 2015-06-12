/*
  ==============================================================================

    BeatTimer.h
    Created: 10 Jun 2015 3:45:40pm
    Author:  Ruofeng Chen

  ==============================================================================
*/

#ifndef MUSICTIMER_H_INCLUDED
#define MUSICTIMER_H_INCLUDED

#include "JuceHeader.h"
#include "PlaybackUI.h"

class PlaybackUI;

class BeatTimer : public Timer
{
public:
    BeatTimer(PlaybackUI *playbackUI);
    ~BeatTimer();
    void setTimer(int bpm, int num_beats_in_measure);
    void timerCallback();
    void startTimer();
private:
    void executeForMeasure();
    int intervalInMilliseconds;
    int num_beats_in_measure;
    int beat_counter;
    PlaybackUI *playbackUI;
};


#endif  // MUSICTIMER_H_INCLUDED
