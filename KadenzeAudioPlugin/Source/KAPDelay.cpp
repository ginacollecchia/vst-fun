/*
  ==============================================================================

    KAPDelay.cpp
    Created: 8 Jan 2021 12:47:04pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#include "KAPDelay.h"
#include <JuceHeader.h>

KAPDelay::KAPDelay()
:   mSampleRate(-1),
    mFeedbackSample(0.0),
    mTimeSmoothed(0),
    mDelayIndex(0)
{
    
}

KAPDelay::~KAPDelay()
{
    
}

void KAPDelay::setSampleRate(double inSampleRate)
{
    mSampleRate = inSampleRate;
}

void KAPDelay::reset()
{
    mTimeSmoothed = 0.f;
    juce::zeromem(mBuffer, (sizeof(double) * maxBufferDelaySize));
}

void KAPDelay::process(float* inAudio, float inDelayTime, float inFeedback, float inWetDry, float* inModulationBuffer, float* outAudio, int inNumSamplesToRender)
{
    const float wet = inWetDry;
    const float dry = 1.f - wet;
    // clamp the feedback to always be less than 1
    const float feedbackMapped = juce::jmap(inFeedback, 0.f, 1.f, 0.f, 0.95f);
    
    mTimeSmoothed -= kParameterSmoothingCoeff_Generic*(mTimeSmoothed - inDelayTime);
    
    for (int i = 0; i < inNumSamplesToRender; i++) {
        
        const double delayTimeModulation = (0.003 * (0.002 * inModulationBuffer[i]));
        
        const double delayTimeInSamples = (mTimeSmoothed * delayTimeModulation * mSampleRate);
        
        // circular buffer to avoid clicks n pops:
        const double sample = getInterpolatedSample(delayTimeInSamples);
        
        mBuffer[mDelayIndex] = inAudio[i] + (mFeedbackSample * feedbackMapped);
        
        mFeedbackSample = sample;
        
        outAudio[i] = (inAudio[i]*dry + sample*wet);
        
        mDelayIndex++;
        
        while (mDelayIndex >= maxBufferDelaySize) {
            mDelayIndex -= maxBufferDelaySize;
        }
    }
}

double KAPDelay::getInterpolatedSample(float inDelayTimeInSamples)
{
    double readPosition = (double)mDelayIndex - inDelayTimeInSamples;
    
    while (readPosition < 0.f) {
        readPosition += maxBufferDelaySize;
    }
    
    int index_y0 = (int)readPosition - 1;
    
    while (index_y0 <= 0) {
        index_y0 += maxBufferDelaySize;
    }
    
    int index_y1 = readPosition;
    
    while (index_y1 >= maxBufferDelaySize) {
        index_y1 -= maxBufferDelaySize;
    }

    const float sample_y0 = mBuffer[index_y0];
    const float sample_y1 = mBuffer[index_y1];
    const float t = readPosition - (int)readPosition;
    
    double outSample = kap_linear_interp(sample_y0, sample_y1, t);
    
    return outSample;
}
