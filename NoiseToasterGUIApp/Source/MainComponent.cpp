#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (800, 829);
    
    dial1.setSliderStyle(juce::Slider::Rotary);
    dial1.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(dial1);
    
    dial2.setSliderStyle(juce::Slider::Rotary);
    dial2.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(dial2);
    
    dial3.setSliderStyle(juce::Slider::Rotary);
    dial3.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(dial3);

    
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (36.0f));
    g.setColour (juce::Colours::white);
    juce::Image background = juce::ImageCache::getFromMemory (BinaryData::noiseToasterFrontPlateFunSmaller_jpg, (size_t) BinaryData::noiseToasterFrontPlateFunSmaller_jpgSize);
    g.drawImageAt (background, 0, 0);
    g.drawText ("NOISE TOASTER", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    auto area = getLocalBounds();
    
    // auto dialArea = area.removeFromTop(area.getHeight() / 7.8);
    // dial1.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 2).reduced(8));
    dial1.setBounds(38, 90, 92, 92);
    dial2.setBounds(267, 90, 92, 92);
    dial3.setBounds(420, 90, 92, 92);
}
