/*
  ==============================================================================

    KAPGain.cpp
    Created: 8 Jan 2021 12:46:22pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#include "KAPGain.h"
#include <JuceHeader.h>

KAPGain::KAPGain()
{
    
}

KAPGain::~KAPGain()
{
    
}

void KAPGain::process(float* inAudio, float inGain, float* outAudio, int inNumSamplesToRender)
{
    // clamp the feedback to -24dB to 24dB; 0.5 gain value = 0dB
    float gainMapped = juce::jmap(inGain, 0.f, 1.f, -24.f, 24.f);
    gainMapped = juce::Decibels::decibelsToGain(gainMapped, -24.f);
    
    for (int i = 0; i < inNumSamplesToRender; i++) {
        outAudio[i] = inAudio[i] * gainMapped;
    }
}
