/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GainAudioProcessorEditor::GainAudioProcessorEditor (GainAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (200, 230);
    
    auto& params = audioProcessor.getParameters();
    gainParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    
    addAndMakeVisible(mGainControlSlider);
    mGainControlSlider.setSliderStyle(juce::Slider::Rotary);
    mGainControlSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, mGainControlSlider.getTextBoxHeight());
    mGainControlSlider.setRange(gainParameter->range.start, gainParameter->range.end);
    mGainControlSlider.setValue(0.8f); // could be set to *gainParameter instead, and use onValueChange, e.g.:
    /*
     mGainControlSlider.onValueChange = [this, gainParameter] {
        *gainParameter = mGainControlSlider.getValue();
     };
     mGainControlSlider.onDragStart = [gainParameter] {
        gainParameter->beginChangeGesture();
     };
     mGainControlSlider.onDragEnd = [gainParameter] {
        gainParameter->endChangeGesture();
     };
     */
    mGainControlSlider.addListener(this);
    
    addAndMakeVisible(mGainLabel);
    mGainLabel.setText("Gain", juce::dontSendNotification);
    mGainLabel.attachToComponent(&mGainControlSlider, true);
 }

GainAudioProcessorEditor::~GainAudioProcessorEditor()
{
}

//==============================================================================
void GainAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void GainAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto sliderLeft = 60;
    mGainControlSlider.setBounds(sliderLeft, 40, getWidth() / 2, getWidth() / 2);
}

void GainAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    
    if (slider == &mGainControlSlider) {
        *gainParameter = mGainControlSlider.getValue();
    }
}
