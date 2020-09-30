/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ResonantLowPassAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ResonantLowPassAudioProcessorEditor (ResonantLowPassAudioProcessor&);
    ~ResonantLowPassAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ResonantLowPassAudioProcessor& audioProcessor;
    
    juce::Slider mGainKnob;
    juce::Label mGainLabel;
    
    juce::Slider mFcKnob;
    juce::Label mFcLabel;
    
    juce::Slider mQKnob;
    juce::Label mQLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResonantLowPassAudioProcessorEditor)
};
