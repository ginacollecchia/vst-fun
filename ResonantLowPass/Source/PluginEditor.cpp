/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ResonantLowPassAudioProcessorEditor::ResonantLowPassAudioProcessorEditor (ResonantLowPassAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    
    auto& params = audioProcessor.getParameters();
    gainParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    fcParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    qParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    
    // GAIN KNOB==================================================================================
    addAndMakeVisible(mGainKnob);
    mGainKnob.setSliderStyle(juce::Slider::Rotary);
    mGainKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, mGainKnob.getTextBoxHeight());
    mGainKnob.setRange(gainParameter->range.start, gainParameter->range.end);
    mGainKnob.setValue(*gainParameter);
    mGainKnob.onValueChange = [this] {
        *gainParameter = mGainKnob.getValue();
    };
    mGainKnob.onDragStart = [this] {
        gainParameter->beginChangeGesture();
    };
    mGainKnob.onDragEnd = [this] {
        gainParameter->endChangeGesture();
    };
    addAndMakeVisible(mGainLabel);
    mGainLabel.setText("Gain", juce::dontSendNotification);
    mGainLabel.attachToComponent(&mGainKnob, true);

    // CUTOFF FREQUENCY KNOB=====================================================================
    addAndMakeVisible(mFcKnob);
    mFcKnob.setSliderStyle(juce::Slider::Rotary);
    mFcKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, mFcKnob.getTextBoxHeight());
    mFcKnob.setRange(fcParameter->range.start, fcParameter->range.end);
    mFcKnob.setValue(*fcParameter);
    mFcKnob.setTextValueSuffix("Hz");
    mFcKnob.onValueChange = [this] {
        *fcParameter = mFcKnob.getValue();
    };
    mFcKnob.onDragStart = [this] {
        fcParameter->beginChangeGesture();
    };
    mFcKnob.onDragEnd = [this] {
        fcParameter->endChangeGesture();
    };
    addAndMakeVisible(mFcLabel);
    mFcLabel.setText("Fc", juce::dontSendNotification);
    mFcLabel.attachToComponent(&mFcKnob, true);

    // Q KNOB====================================================================================
    addAndMakeVisible(mQKnob);
    mQKnob.setSliderStyle(juce::Slider::Rotary);
    mQKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, mQKnob.getTextBoxHeight());
    mQKnob.setRange(qParameter->range.start, qParameter->range.end);
    mQKnob.setValue(*qParameter);
    mQKnob.onValueChange = [this] {
        *qParameter = mQKnob.getValue();
    };
    mQKnob.onDragStart = [this] {
        qParameter->beginChangeGesture();
    };
    mQKnob.onDragEnd = [this] {
        qParameter->endChangeGesture();
    };
    addAndMakeVisible(mQLabel);
    mQLabel.setText("Q", juce::dontSendNotification);
    mQLabel.attachToComponent(&mQKnob, true);

}

ResonantLowPassAudioProcessorEditor::~ResonantLowPassAudioProcessorEditor()
{
}

//==============================================================================
void ResonantLowPassAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Resonant Low Pass Filter", getLocalBounds(), juce::Justification::centred, 1);
}

void ResonantLowPassAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    float knobLeft = 30;
    float offset = getWidth() / 3 - knobLeft - 10;
    mGainKnob.setBounds(knobLeft, knobLeft, offset, offset);
    mFcKnob.setBounds(knobLeft + offset, knobLeft, offset, offset);
    mQKnob.setBounds(knobLeft + 2*offset, knobLeft, offset, offset);
}
