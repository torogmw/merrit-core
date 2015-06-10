/*
  ==============================================================================

    BeatTimer.cpp
    Created: 10 Jun 2015 3:45:58pm
    Author:  Ruofeng Chen

  ==============================================================================
*/

#include "MusicTimers.h"

BeatTimer::BeatTimer()
{
    intervalInMilliseconds = 0;
    num_beats_in_measure = 0;
    beat_counter = 0;
}

void BeatTimer::setTimer(int bpm, int num_beats_in_measure_)
{
    intervalInMilliseconds = 1000.0 / ((float)bpm / 60.0);
    num_beats_in_measure = num_beats_in_measure_;
    beat_counter = 0;
}

void BeatTimer::timerCallback()
{
    beat_counter++;
    if (beat_counter == num_beats_in_measure) {
        beat_counter = 0;
    }
}

void BeatTimer::startTimer()
{
    printf("%d\n", intervalInMilliseconds);
    juce::Timer::startTimer(intervalInMilliseconds);
}

BeatTimer::~BeatTimer()
{

}