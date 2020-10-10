/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusFlangerAudioProcessorEditor::ChorusFlangerAudioProcessorEditor (ChorusFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (320, 320);
    
    auto& params = processor.getParameters();
    
    // Dry/wet slider
    juce::AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    
    mDryWetSlider.setBounds(10, 90, 100, 100);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetSlider.onValueChange = [this, dryWetParameter] { *dryWetParameter = mDryWetSlider.getValue();};
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };
    
    mDryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    mDryWetLabel.attachToComponent(&mDryWetSlider, false);
    addAndMakeVisible(mDryWetLabel);
    
    // Feedback slider
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    
    mFeedbackSlider.setBounds(110, 90, 100, 100);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);
    
    mFeedbackSlider.onValueChange = [this, feedbackParameter] { *feedbackParameter = mFeedbackSlider.getValue();};
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };
    
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, false);
    addAndMakeVisible(mFeedbackLabel);

    // Depth slider
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    
    mDepthSlider.setBounds(210, 90, 100, 100);
    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mDepthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    addAndMakeVisible(mDepthSlider);
    
    mDepthSlider.onValueChange = [this, depthParameter] { *depthParameter = mDepthSlider.getValue();};
    mDepthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };
    
    mDepthLabel.setText("Depth", juce::dontSendNotification);
    mDepthLabel.attachToComponent(&mDepthSlider, false);
    addAndMakeVisible(mDepthLabel);
    
    // Phase offset slider
    juce::AudioParameterFloat* phaseOffsetParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);
    
    mPhaseOffsetSlider.setBounds(10, 210, 100, 100);
    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mPhaseOffsetSlider.setRange(phaseOffsetParameter->range.start, phaseOffsetParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseOffsetParameter);
    addAndMakeVisible(mPhaseOffsetSlider);
    
    mPhaseOffsetSlider.onValueChange = [this, phaseOffsetParameter] { *phaseOffsetParameter = mPhaseOffsetSlider.getValue();};
    mPhaseOffsetSlider.onDragStart = [phaseOffsetParameter] { phaseOffsetParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseOffsetParameter] { phaseOffsetParameter->endChangeGesture(); };
    
    mPhaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    mPhaseOffsetLabel.attachToComponent(&mPhaseOffsetSlider, false);
    addAndMakeVisible(mPhaseOffsetLabel);

    // Rate slider
    juce::AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    
    mRateSlider.setBounds(110, 210, 100, 100);
    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mRateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    addAndMakeVisible(mRateSlider);
    
    mRateSlider.onValueChange = [this, rateParameter] { *rateParameter = mRateSlider.getValue();};
    mRateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };
    
    mRateLabel.setText("Rate", juce::dontSendNotification);
    mRateLabel.attachToComponent(&mRateSlider, false);
    addAndMakeVisible(mRateLabel);

    // Type combo box
    juce::AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);
    
    mTypeComboBox.setBounds(120, 15, 100, 30);
    mTypeComboBox.addItem("Chorus", 1);
    mTypeComboBox.addItem("Flanger", 2);
    addAndMakeVisible(mTypeComboBox);
    mTypeComboBox.onChange = [this, typeParameter] {
        typeParameter->beginChangeGesture();
        *typeParameter = mTypeComboBox.getSelectedItemIndex();
        typeParameter->endChangeGesture();
    };
    
    mTypeComboBox.setSelectedItemIndex(*typeParameter);
    
    mTypeLabel.setText("Effect: ", juce::dontSendNotification);
    mTypeLabel.attachToComponent(&mTypeComboBox, true);
    addAndMakeVisible(mTypeLabel);
}

ChorusFlangerAudioProcessorEditor::~ChorusFlangerAudioProcessorEditor()
{
}

//==============================================================================
void ChorusFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ChorusFlangerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
