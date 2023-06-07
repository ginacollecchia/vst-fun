/*
  ==============================================================================

    KAPGain.h
    Created: 8 Jan 2021 12:46:22pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#pragma once

#include "KAPAudioHelpers.h"

#include "JuceHeader.h"

class KAPGain
{
public:
    
    KAPGain();
    ~KAPGain();
    
    void process(float* inAudio, float inGain, float* outAudio, int inNumSamplesToRender);
    
private:
};
