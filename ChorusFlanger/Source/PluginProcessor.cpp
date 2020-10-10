/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusFlangerAudioProcessor::ChorusFlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet", "Dry Wet", 0, 1, 0.5));
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));
    addParameter(mDepthParameter = new juce::AudioParameterFloat("depth", "Depth", 0, 1.0, 0.5f));
    addParameter(mPhaseOffsetParameter = new juce::AudioParameterFloat("phaseOffset", "Phase Offset", 0, 1.f, 0.f));
    addParameter(mRateParameter = new juce::AudioParameterFloat("rate", "Rate", 0, 20.f, 10.f));
    addParameter(mType = new juce::AudioParameterInt("type", "Type", 0, 1, 0));

    mFeedbackLeft = 0.f;
    mFeedbackRight = 0.f;
    mLFOPhase = 0.f;

}

ChorusFlangerAudioProcessor::~ChorusFlangerAudioProcessor()
{
    if (mCircularBufferLeft != nullptr) {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    
    if (mCircularBufferRight != nullptr) {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }

}

//==============================================================================
const juce::String ChorusFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusFlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChorusFlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChorusFlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChorusFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusFlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ChorusFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChorusFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ChorusFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLength];
    }
 
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLength];
    }

    // zero out the memory so that everything inside the circular buffer is silent
    juce::zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
    juce::zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));

    mCircularBufferWriteHead = 0;
    
    mLFOPhase = 0;

}

void ChorusFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

float ChorusFlangerAudioProcessor::lin_interp(float inSampleX, float inSampleY, float inFloatPhase)
{
    return (1 - inFloatPhase) * inSampleX  + inFloatPhase * inSampleY;
}

void ChorusFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    float* channelDataLeft = buffer.getWritePointer(0);
    float* channelDataRight = buffer.getWritePointer(0);

    // Feed delay buffer into channelData
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        
        // create 2 LFOs, with the left channel modified by the LFO and the right channel modified by the phase offset
        float lfoOutLeft = sin(2 * M_PI * mLFOPhase);
        float lfoPhaseRight = mLFOPhase * *mPhaseOffsetParameter;
        if (lfoPhaseRight > 1) lfoPhaseRight -= 1;
        float lfoOutRight = sin(2 * M_PI * lfoPhaseRight);

        // scale LFOs by depth
        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;
        
        float lfoOutMappedLeft = 0;
        float lfoOutMappedRight = 0;
        
        if (*mType == 0) {
            // CHORUS EFFECT
            // map LFO phase to time
            lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
        } else {
            // FLANGER EFFECT: different delay times create different timbres
            lfoOutMappedLeft = juce::jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
            lfoOutMappedRight = juce::jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
        }
        
        float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
        
        // smooth the time parameter: matters for sawtooth or undifferentiable LFOs
        // mDelayTimeSmoothed -= 0.001 * (mDelayTimeSmoothed - lfoOutMapped);

        mLFOPhase += *mRateParameter / getSampleRate();
        if (mLFOPhase > 1) mLFOPhase -= 1;
        
        // read the buffer data and add feedback
        mCircularBufferLeft[mCircularBufferWriteHead] = channelDataLeft[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = channelDataRight[i] + mFeedbackRight;
        
        // set delay readheads
        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
        if (delayReadHeadLeft < 0) delayReadHeadLeft += mCircularBufferLength;
        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
        if (delayReadHeadRight < 0) delayReadHeadRight += mCircularBufferLength;

        // do interpolation: get floating point value of difference between float time and samples
        int readHeadLeft_x1 = (int)delayReadHeadLeft;
        int readHeadLeft_x2 = readHeadLeft_x1 + 1;
        if (readHeadLeft_x2 >= mCircularBufferLength) readHeadLeft_x2 -= mCircularBufferLength;
        float readHeadPhaseLeft = delayReadHeadLeft - readHeadLeft_x1;
        
        int readHeadRight_x1 = (int)delayReadHeadRight;
        int readHeadRight_x2 = readHeadRight_x1 + 1;
        if (readHeadRight_x2 >= mCircularBufferLength) readHeadRight_x2 -= mCircularBufferLength;
        float readHeadPhaseRight = delayReadHeadRight - readHeadRight_x1;

        // Now we get the interpolated delay sample
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x1], mCircularBufferLeft[readHeadLeft_x2], readHeadPhaseLeft);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x1], mCircularBufferRight[readHeadRight_x2], readHeadPhaseRight);

        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
        
        // add in the delayed signal
        // buffer.addSample(channel, i, delay_sample);
        buffer.setSample(0, i, buffer.getSample(0, i) * (1.f - *mDryWetParameter) + delay_sample_left * *mDryWetParameter);
        buffer.setSample(1, i, buffer.getSample(1, i) * (1.f - *mDryWetParameter) + delay_sample_right * *mDryWetParameter);

        // iterate the write head of the circular buffer
        mCircularBufferWriteHead++;
        if (mCircularBufferWriteHead >= mCircularBufferLength) mCircularBufferWriteHead = 0;
    }
}

//==============================================================================
bool ChorusFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ChorusFlangerAudioProcessor::createEditor()
{
    return new ChorusFlangerAudioProcessorEditor (*this);
}

//==============================================================================
void ChorusFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("ChorusFlanger"));
    xml->setAttribute("DryWet", *mDryWetParameter);
    xml->setAttribute("Depth", *mDepthParameter);
    xml->setAttribute("Rate", *mRateParameter);
    xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
    xml->setAttribute("Feedback", *mFeedbackParameter);
    xml->setAttribute("Type", *mType);
    
    copyXmlToBinary(*xml, destData);
}

void ChorusFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml.get() != nullptr && xml->hasTagName("ChorusFlanger")) {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mFeedbackParameter = xml->getDoubleAttribute("Feedback");
        *mType = xml->getIntAttribute("Type");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusFlangerAudioProcessor();
}
