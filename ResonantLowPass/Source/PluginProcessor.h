/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Biquad.h"

#ifndef dB
// if below -100dB, set to -100dB to prevent taking log of zero
#define dB(x)       20.0 * ((x) > 0.00001 ? log10(x) : log10(0.00001))
#endif

#ifndef dB2mag
#define dB2mag(x)   pow( 10.0, (x) / 20.0 )
#endif


#define kMaxLen        32

#define pi          3.14159265358979
#define eps         2.220446049250313e-16

//==============================================================================
/**
*/

// input filter gain limits, dB; taper, exponent
const static float GainLimits[2] = {-24.0, 24.0};
const static float GainTaper = 1.0;

// input filter center frequency limits, hz; taper, exponent
const static float FcLimits[2] = {50.0, 5000.0};
const static float FcTaper = -1.0;

// input filter resonance limits, dB; taper, exponent
const static float QLimits[2] = {0.25, 32.0};
const static float QTaper = -1.0;


class ResonantLowPassAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ResonantLowPassAudioProcessor();
    ~ResonantLowPassAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //new things====================================================================
    void bilinearTransform(float acoefs[], float dcoefs[]);
    void designResonantLowPass(float* dcoefs, float center, float qval);
    

private:
    //==============================================================================
    juce::AudioParameterFloat* mFcParameter;
    juce::AudioParameterFloat* mQParameter;
    juce::AudioParameterFloat* mGainParameter;
        
    float lpCoefs[5] = {1.f, 0.f, 0.f, 0.f, 0.f}; // filter coefficients
    Biquad lpFilter;
    float mGainSmoothed;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantLowPassAudioProcessor)
};

