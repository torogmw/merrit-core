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

class BeatTimer : public Timer
{
public:
    BeatTimer();
    ~BeatTimer();
    void setTimer(int bpm, int num_beats_in_measure);
    void timerCallback();
    void startTimer();
private:
    int intervalInMilliseconds;
    int num_beats_in_measure;
    int beat_counter;
};


#endif  // MUSICTIMER_H_INCLUDED
