/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayAudioProcessor::DelayAudioProcessor()
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
    mCircularBuffer = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet", "Dry Wet", 0, 1, 0.5));
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));
    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delayTime", "Delay Time", 0, MAX_DELAY_TIME, 0.5));
    
    mDelayReadHead = 0.f;
    mDelayTimeInSamples = 0.f;
    mDelayTimeSmoothed = 0.f;
    mFeedback = 0.f;
}

DelayAudioProcessor::~DelayAudioProcessor()
{
    if (mCircularBuffer != nullptr) {
        delete [] mCircularBuffer;
        mCircularBuffer = nullptr;
    }
}

//==============================================================================
const juce::String DelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;

    if (mCircularBuffer == nullptr) {
        mCircularBuffer = new float[mCircularBufferLength];
    }
    
    // zero out the memory so that everything inside the circular buffer is silent
    juce::zeromem(mCircularBuffer, mCircularBufferLength * sizeof(float));
    
    mCircularBufferWriteHead = 0;
    
    mDelayTimeSmoothed = mDelayTimeParameter->get();
}

void DelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

float DelayAudioProcessor::lin_interp(float inSampleX, float inSampleY, float inFloatPhase)
{
    return (1 - inFloatPhase) * inSampleX  + inFloatPhase * inSampleY;
}

void DelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        // Feed delay buffer into channelData
        for (int i = 0; i < buffer.getNumSamples(); i++) {
            
            // smooth the time parameter
            mDelayTimeSmoothed = mDelayTimeSmoothed - 0.0001*(mDelayTimeSmoothed - *mDelayTimeParameter);
            mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed;

            // read the buffer data and add feedback
            mCircularBuffer[mCircularBufferWriteHead] = channelData[i] + mFeedback;
            
            // set delay readhead
            mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
            if (mDelayReadHead < 0) {
                mDelayReadHead += mCircularBufferLength;
            }
            
            // do interpolation: get floating point value of difference between float time and samples
            int readHead_x1 = (int)mDelayReadHead;
            int readHead_x2 = readHead_x1 + 1;
            float readHeadPhase = mDelayReadHead - readHead_x1;
            if (readHead_x2 >= mCircularBufferLength) {
                readHead_x2 -= mCircularBufferLength;
            }
            
            // Now we get the interpolated delay sample
            float delay_sample = lin_interp(mCircularBuffer[readHead_x1], mCircularBuffer[readHead_x2], readHeadPhase);
            // float delay_sample = mCircularBuffer[(int)mDelayReadHead];

            mFeedback = delay_sample * *mFeedbackParameter;
            
            // add in the delayed signal
            // buffer.addSample(channel, i, delay_sample);
            buffer.setSample(channel, i, buffer.getSample(channel, i) * (1.f - *mDryWetParameter) + delay_sample * *mDryWetParameter);
            
            // iterate the write head of the circular buffer
            mCircularBufferWriteHead++;
            if (mCircularBufferWriteHead >= mCircularBufferLength) {
                mCircularBufferWriteHead = 0;
            }
        }
    }
}

//==============================================================================
bool DelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayAudioProcessor::createEditor()
{
    return new DelayAudioProcessorEditor (*this);
}

//==============================================================================
void DelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayAudioProcessor();
}

