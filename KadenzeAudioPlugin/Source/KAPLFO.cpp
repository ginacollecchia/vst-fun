/*
  ==============================================================================

    KAPLFO.cpp
    Created: 8 Jan 2021 12:46:13pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#include "KAPLFO.h"
#include <JuceHeader.h>

KAPLfo::KAPLfo()
{
    reset();
}

KAPLfo::~KAPLfo()
{
    
}

void KAPLfo::reset()
{
    mPhase = 0.f;
    juce::zeromem(mBuffer, sizeof(float)*maxBufferDelaySize);
}

void KAPLfo::setSampleRate(double inSampleRate)
{
    mSampleRate = inSampleRate;
}

void KAPLfo::process(float inRate, float inDepth, int inNumSamplesToRender)
{
    // map 0 to 1 to 0.01Hz to 10Hz
    const float rate = juce::jmap(inRate, 0.f, 1.f, 0.01f, 10.f);
    
    for (int i = 0; i < inNumSamplesToRender; i++) {
        mPhase += (rate / mSampleRate);

        if (mPhase > 1.f) {
            mPhase -= 1.f;
        }
        
        const float lfoPosition = sinf(mPhase * k2PI) * inDepth;
        mBuffer[i] = lfoPosition;
    }
    
}

float* KAPLfo::getBuffer()
{
    return mBuffer;
}
