/*
  ==============================================================================

    KAPDelay.h
    Created: 8 Jan 2021 12:47:04pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#pragma once

#include "KAPAudioHelpers.h"

class KAPDelay {
public:
    KAPDelay();
    ~KAPDelay();
    
    void setSampleRate(double inSampleRate);
    
    void reset();
    
    void process(float* inAudio,
                 float inDelayTime, // called inTime in tutorial
                 float inFeedback,
                 float inWetDry,
                 float* inModulationBuffer,
                 float* outAudio,
                 int inNumSamplesToRender);
    
private:
    double getInterpolatedSample(float inDelayTimeInSamples);
    
    double mSampleRate;
    double mBuffer[maxBufferDelaySize];
    double mFeedbackSample;
    
    float mTimeSmoothed;
    
    int mDelayIndex;
};
