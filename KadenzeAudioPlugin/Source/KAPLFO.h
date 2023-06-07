/*
  ==============================================================================

    KAPLFO.h
    Created: 8 Jan 2021 12:46:13pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#pragma once

#include "KAPAudioHelpers.h"

class KAPLfo {
public:
    
    KAPLfo();
    ~KAPLfo();
    
    void reset();
    
    void setSampleRate(double inSampleRate);
    
    void process(float inRate, float inDepth, int inNumSamplesToRender);
    
    float* getBuffer();
    
private:
    
    double mSampleRate;
    
    float mPhase;
    
    float mBuffer[maxBufferDelaySize];
    
};
