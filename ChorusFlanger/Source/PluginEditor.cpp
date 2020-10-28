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
    setSize (500, 524);
    
    auto& params = processor.getParameters();
    
    // Dry/wet slider
    juce::AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    int sliderDiameter = 65;
    
    mDryWetSlider.setBounds(65, 190, sliderDiameter, sliderDiameter);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    mDryWetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    mDryWetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
    mDryWetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    mDryWetSlider.setTransform(juce::AffineTransform::rotation(0.314).translated(mDryWetSlider.getBounds().getCentre()));
    mDryWetSlider.setCentrePosition(100, 230);
    mDryWetSlider.setRotaryParameters(4.189, 8.378, true); // 240 degrees to 480 degrees
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetSlider.onValueChange = [this, dryWetParameter] { *dryWetParameter = mDryWetSlider.getValue();};
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };
    
    // mDryWetLabel.setText("Dry/Wet", juce::dontSendNotification);
    // mDryWetLabel.attachToComponent(&mDryWetSlider, false);
    // addAndMakeVisible(mDryWetLabel);
    
    // Feedback slider
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    
    mFeedbackSlider.setBounds(220, 80, sliderDiameter, sliderDiameter);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    mFeedbackSlider.setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    mFeedbackSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
    mFeedbackSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    mFeedbackSlider.setTransform(juce::AffineTransform::rotation(1.571).translated(mFeedbackSlider.getBounds().getCentre()));
    mFeedbackSlider.setCentrePosition(244, 102);
    mFeedbackSlider.setRotaryParameters(4.189, 8.378, true); // 240 degrees to 480 degrees
    addAndMakeVisible(mFeedbackSlider);
    
    mFeedbackSlider.onValueChange = [this, feedbackParameter] { *feedbackParameter = mFeedbackSlider.getValue();};
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };
    
    // mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    // mFeedbackLabel.attachToComponent(&mFeedbackSlider, false);
    // addAndMakeVisible(mFeedbackLabel);

    // Depth slider
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    
    mDepthSlider.setBounds(370, 180, sliderDiameter, sliderDiameter);
    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mDepthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    mDepthSlider.setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    mDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
    mDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    mDepthSlider.setTransform(juce::AffineTransform::rotation(2.827).translated(mDepthSlider.getBounds().getCentre()));
    mDepthSlider.setCentrePosition(399, 217);
    mDepthSlider.setRotaryParameters(4.189, 8.378, true); // 240 degrees to 480 degrees
    addAndMakeVisible(mDepthSlider);
    
    mDepthSlider.onValueChange = [this, depthParameter] { *depthParameter = mDepthSlider.getValue();};
    mDepthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };
    
    // mDepthLabel.setText("Depth", juce::dontSendNotification);
    // mDepthLabel.attachToComponent(&mDepthSlider, false);
    // addAndMakeVisible(mDepthLabel);
    
    // Phase offset slider
    juce::AudioParameterFloat* phaseOffsetParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);
    
    mPhaseOffsetSlider.setBounds(120, 380, sliderDiameter, sliderDiameter);
    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mPhaseOffsetSlider.setRange(phaseOffsetParameter->range.start, phaseOffsetParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseOffsetParameter);
    mPhaseOffsetSlider.setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
    mPhaseOffsetSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    mPhaseOffsetSlider.setTransform(juce::AffineTransform::rotation(-0.942).translated(mPhaseOffsetSlider.getBounds().getCentre()));
    mPhaseOffsetSlider.setCentrePosition(152, 418);
    mPhaseOffsetSlider.setRotaryParameters(4.189, 8.378, true); // 240 degrees to 480 degrees
    addAndMakeVisible(mPhaseOffsetSlider);
    
    mPhaseOffsetSlider.onValueChange = [this, phaseOffsetParameter] { *phaseOffsetParameter = mPhaseOffsetSlider.getValue();};
    mPhaseOffsetSlider.onDragStart = [phaseOffsetParameter] { phaseOffsetParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseOffsetParameter] { phaseOffsetParameter->endChangeGesture(); };
    
    // mPhaseOffsetLabel.setText("Phase Offset", juce::dontSendNotification);
    // mPhaseOffsetLabel.attachToComponent(&mPhaseOffsetSlider, false);
    // addAndMakeVisible(mPhaseOffsetLabel);

    // Rate slider
    juce::AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    
    mRateSlider.setBounds(320, 380, sliderDiameter, sliderDiameter);
    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    mRateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    mRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::limegreen);
    mRateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::red);
    mRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::red);
    mRateSlider.setTransform(juce::AffineTransform::rotation(-2.15).translated(mRateSlider.getBounds().getCentre()));
    mRateSlider.setCentrePosition(361, 414);
    mRateSlider.setRotaryParameters(4.189, 8.378, true); // 240 degrees to 480 degrees
    addAndMakeVisible(mRateSlider);
    
    mRateSlider.onValueChange = [this, rateParameter] { *rateParameter = mRateSlider.getValue();};
    mRateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };
    
    // mRateLabel.setText("Rate", juce::dontSendNotification);
    // mRateLabel.attachToComponent(&mRateSlider, false);
    // addAndMakeVisible(mRateLabel);

    // Type combo box
    juce::AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);
    
    mTypeComboBox.setBounds(360, 15, 100, 30);
    mTypeComboBox.addItem("Chorus", 1);
    mTypeComboBox.addItem("Flanger", 2);
    addAndMakeVisible(mTypeComboBox);
    mTypeComboBox.onChange = [this, typeParameter] {
        typeParameter->beginChangeGesture();
        *typeParameter = mTypeComboBox.getSelectedItemIndex();
        typeParameter->endChangeGesture();
    };
    
    mTypeComboBox.setSelectedItemIndex(*typeParameter);
    
    mTypeLabel.setText("Effect:", juce::dontSendNotification);
    mTypeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    mTypeLabel.setFont(juce::Font("Comic Sans MS", "Regular", 20.f));
    mTypeLabel.attachToComponent(&mTypeComboBox, true);
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
    juce::Image background = juce::ImageCache::getFromMemory (BinaryData::chorusBackground_jpg, (size_t) BinaryData::chorusBackground_jpgSize);
    if (mTypeComboBox.getSelectedItemIndex() == 1) {
        background = juce::ImageCache::getFromMemory (BinaryData::chorusBackground_jpg, (size_t) BinaryData::chorusBackground_jpgSize);
    } else {
        background = juce::ImageCache::getFromMemory (BinaryData::flangerBackground_jpg, (size_t) BinaryData::flangerBackground_jpgSize);
    }
    g.drawImageAt (background, 0, 0);

    // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ChorusFlangerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
