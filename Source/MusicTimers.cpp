/*
  ==============================================================================

    BeatTimer.cpp
    Created: 10 Jun 2015 3:45:58pm
    Author:  Ruofeng Chen

  ==============================================================================
*/

#include "MusicTimers.h"

BeatTimer::BeatTimer(PlaybackUI *playbackUI_)
{
    intervalInMilliseconds = 0;
    num_beats_in_measure = 0;
    beat_counter = 0;
    playbackUI = playbackUI_;
}

void BeatTimer::setTimer(int bpm, int num_beats_in_measure_)
{
    intervalInMilliseconds = 1000.0 / ((float)bpm / 60.0);
    num_beats_in_measure = num_beats_in_measure_;
    beat_counter = 0;
}

void BeatTimer::executeForMeasure()
{
    playbackUI->getEverythingReadyForMeasure(playbackUI->getCurrentMeasure());
    playbackUI->progressToNextMeasure();
}

void BeatTimer::timerCallback()
{
    if (beat_counter == 0) {
        executeForMeasure();
    }
    
    beat_counter++;
    if (beat_counter == num_beats_in_measure) {
        beat_counter = 0;
    }

}

void BeatTimer::startTimer()
{
    juce::Timer::startTimer(intervalInMilliseconds);
}

BeatTimer::~BeatTimer()
{

}