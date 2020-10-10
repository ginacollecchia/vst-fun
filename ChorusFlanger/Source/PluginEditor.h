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
class ChorusFlangerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ChorusFlangerAudioProcessorEditor (ChorusFlangerAudioProcessor&);
    ~ChorusFlangerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChorusFlangerAudioProcessor& audioProcessor;
    
    juce::Slider mDryWetSlider;
    juce::Label mDryWetLabel;
    juce::Slider mFeedbackSlider;
    juce::Label mFeedbackLabel;
    juce::Slider mDepthSlider;
    juce::Label mDepthLabel;
    juce::Slider mPhaseOffsetSlider;
    juce::Label mPhaseOffsetLabel;
    juce::Slider mRateSlider;
    juce::Label mRateLabel;
    juce::ComboBox mTypeComboBox;
    juce::Label mTypeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusFlangerAudioProcessorEditor)
};
