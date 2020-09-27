/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ResonantLowPassAudioProcessor::ResonantLowPassAudioProcessor()
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
    addParameter(mGainParameter = new juce::AudioParameterFloat("gain", "Gain", 0.f, 1.f, 1.f));
    addParameter(mQParameter = new juce::AudioParameterFloat("q", "Q", 0.25f, 32.f, 5.f));
    addParameter(mFcParameter = new juce::AudioParameterFloat("fc", "Fc", 50.f, 5000.f, 1000.f));
    
    fs = getSampleRate();
    
    // TODO: this filter should change every time that Fc or Q are changed
    designResonantLowPass(lpCoefs, *mFcParameter, *mQParameter);
    lpFilter.setCoefs(lpCoefs);
    
    mGainSmoothed = mGainParameter->get();
}

ResonantLowPassAudioProcessor::~ResonantLowPassAudioProcessor()
{
}

//==============================================================================
const juce::String ResonantLowPassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ResonantLowPassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ResonantLowPassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ResonantLowPassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ResonantLowPassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ResonantLowPassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ResonantLowPassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ResonantLowPassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ResonantLowPassAudioProcessor::getProgramName (int index)
{
    return {};
}

void ResonantLowPassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ResonantLowPassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ResonantLowPassAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ResonantLowPassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ResonantLowPassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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
    auto* channelLeft = buffer.getWritePointer(0);
    auto* channelRight = buffer.getWritePointer(1);
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        lpFilter.process(channelLeft[i], channelLeft[i]);
        lpFilter.process(channelRight[i], channelRight[i]);
        mGainSmoothed -= 0.004*(mGainSmoothed-*mGainParameter);
        channelLeft[i] *= mGainSmoothed;
        channelRight[i] *= mGainSmoothed;
    }

}

//==============================================================================
bool ResonantLowPassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ResonantLowPassAudioProcessor::createEditor()
{
    return new ResonantLowPassAudioProcessorEditor (*this);
}

//==============================================================================
void ResonantLowPassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ResonantLowPassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ResonantLowPassAudioProcessor();
}

//------------------------------------------------------------------------------
void ResonantLowPassAudioProcessor::bilinearTransform(float acoefs[], float dcoefs[])
{
    float b0, b1, b2, a0, a1, a2;        //storage for continuous-time filter coefs
    float bz0, bz1, bz2, az0, az1, az2;    // coefs for discrete-time filter.
    
    // For easier looking code...unpack
    b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2];
    a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];
    
    
    // TODO: apply bilinear transform
    ///////////////START//////////////////
    bz0 = 1.0; bz1 = 0.0; bz2 = 0.0;
    az0 = 1.0; az1 = 0.0; az2 = 0.0;
    ////////////////END/////////////////////
    
    float T = 1/fs;
    float Tsq = T*T;
    
    // we need to normalize because the biquad struct assumes az0 = 1
    
    az0 = ( a0*Tsq + 2*a1*T + 4*a2 );
    az1 = ( 2*a0*Tsq - 8*a2 ) / az0;
    az2 = ( a0*Tsq - 2*a1*T + 4*a2 ) / az0;
    
    bz0 = ( b0*Tsq + 2*b1*T + 4*b2 ) / az0;
    bz1 = ( 2*b0*Tsq - 8*b2 ) / az0;
    bz2 = ( b0*Tsq - 2*b1*T + 4*b2 ) / az0;
    
    az0 = 1;
    
    ////////////////END/////////////////////
    
    // return coefficients to the output
    dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
    dcoefs[3] = az1; dcoefs[4] = az2;
    
}

//------------------------------------------------------------------------------
void ResonantLowPassAudioProcessor::designResonantLowPass(float* dcoefs, float center, float qval)
// design parametric filter based on input center frequency, gain, Q and sampling rate
{
    float b0, b1, b2, a0, a1, a2;        //storage for continuous-time filter coefs
    float acoefs[6];
    
    // Filter of the form
    //
    //    2
    // b2s  + b1s + b0
    // ---------------
    //    2
    // a2s  + a1s + a0
    //
    // Parameters are center frequency in Hz, gain in dB, and Q.
    
    
    b0 = 1.0; b1 = 0.0; b2 = 0.0;
    a0 = 1.0; a1 = 0.0; a2 = 0.0;
    
    center *= 2*pi;
    
    // pre-warping
    center = 2*fs*tan(center/(2*fs));
    
    
    b0 = 1.0;
    b1 = 0.0;
    b2 = 0.0;
    
    
    a0 = 1.0;
    a1 = 1.0 / ( center * qval );
    a2 = 1.0 / ( center * center );
    
    // pack the analog coeffs into an array and apply the bilinear tranform
    acoefs[0] = b0; acoefs[1] = b1; acoefs[2] = b2;
    acoefs[3] = a0; acoefs[4] = a1; acoefs[5] = a2;
    
    // inputs the 6 analog coeffs, output the 5 digital coeffs
    bilinearTransform(acoefs, dcoefs);
}

