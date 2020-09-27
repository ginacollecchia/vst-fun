//------------------------------------------------------------------------------
// VST WahWah Plug-In
//
// Filename     : WahWah.cpp
// Author       : Regina Collecchia + music 424 staff
// Date         : 4/24/14 (whoa)
// Homework     : 3, problem 2
// Description  : Implements a wah-wah pedal guitar effect with the use of an LFO, frequency computer, and stereo side-chaining (only time that the "detectLevel" function is used).
// 
//------------------------------------------------------------------------------

#include "WahWah.h"
#include <math.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new WahWah (audioMaster);
}

//------------------------------------------------------------------------------
WahWah::WahWah (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumProgs, kNumParams)	// 1 program, 1 parameter only
{
	setNumInputs (kNumInputs);		// stereo in
	setNumOutputs (kNumOutputs);		// stereo out
	setUniqueID ('Dstn');	// identify
	canProcessReplacing ();	// supports replacing output


	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name

	
	GainValue = (float) 0.0;	// input filter gain, dB
	FcValue = (float) 1000.0;	// input filter center frequency, Hz
	QValue = (float) 5.0;	// input filter resonance, ratio
	RateValue = (float) 1.0;
	DepthValue = (float) 1.0;

	GainKnob = SmartKnob::value2knob(GainValue, GainLimits, GainTaper);
	FcKnob = SmartKnob::value2knob(FcValue, FcLimits, FcTaper);
	QKnob = SmartKnob::value2knob(QValue, QLimits, QTaper);
	RateKnob = (float) SmartKnob::value2knob(RateValue, RateLimits, RateTaper);	
	DepthKnob  = (float) SmartKnob::value2knob(DepthValue, DepthLimits, DepthTaper);

	drive = 1.0;
    // initialize the phase
    ph = 0.0;
    
	// signal processing parameter and state initialization
	fs = getSampleRate();	// sampling rate, Hz
    
	fcSlewer.setTau(.02, fs); // set the mod signal slewer
	gSlewer.setTau(.02, fs); // set the mod signal slewer	
	qSlewer.setTau(.02, fs); // set the mod signal slewer
	modSlewer.setTau(.02, fs); // set the mod signal slewer


	// place holder
	designResonantLowPass(lpCoefs, FcValue, QValue);
	lpFilter.setCoefs(lpCoefs);
}

//------------------------------------------------------------------------------
WahWah::~WahWah ()
{
	// nothing to do here
}

//------------------------------------------------------------------------------
void WahWah::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void WahWah::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void WahWah::setParameter(VstInt32 index, float value)
{

	// assign control and signal processing parameter values
	switch (index)
	{

	case kParamGain:
		// input filter gain, dB
		GainKnob = value;
		GainValue = SmartKnob::knob2value(GainKnob, GainLimits, GainTaper);
			
		// input gain, amplitude
		drive = dB2mag( GainValue );

		break;

	case kParamFc:
		// input filter center frequency, Hz
		FcKnob = value;
		FcValue = SmartKnob::knob2value(FcKnob, FcLimits, FcTaper);

		break;

	case kParamQ:
		// input filter resonance, ratio
		QKnob = value;
		QValue = SmartKnob::knob2value(QKnob, QLimits, QTaper);

		break;
	case kParamRate:
		// input filter resonance, ratio
		RateKnob = value;
		RateValue = SmartKnob::knob2value(RateKnob, RateLimits, RateTaper);
		break;
	case kParamDepth:
		// input filter resonance, ratio
		DepthKnob = value;
        DepthValue = SmartKnob::knob2value(DepthKnob, DepthLimits, DepthTaper);
			
		break;			
	default :
		break;
	}

}


//------------------------------------------------------------------------------
float WahWah::getParameter(VstInt32 index)
{
	// return knob position
	switch (index)
	{
	case kParamGain:
		// input filter gain, dB
		return GainKnob;
		break;

	case kParamFc:
		// input filter center frequency, Hz
		return FcKnob;
		break;

	case kParamQ:
		// input filter resonance, ratio
		return QKnob;
		break;
	case kParamRate:
		// input filter resonance, ratio
		return RateKnob;
		break;
	case kParamDepth:
		// input filter resonance, ratio
		return DepthKnob;
		break;
			
	default:
		return 0.0;
	}
}

//------------------------------------------------------------------------------
void WahWah::getParameterName(VstInt32 index, char *label)
{
	// return knob name
	switch (index)
	{
	case kParamGain:
		// input filter gain, dB
		vst_strncpy(label, " Gain ", kVstMaxParamStrLen);
		break;

	case kParamFc:
		// input filter center frequency, Hz
		vst_strncpy(label, " Fc ", kVstMaxParamStrLen);
		break;

	case kParamQ:
		// input filter resonance, ratio
		vst_strncpy(label, " Q ", kVstMaxParamStrLen);
		break;
	case kParamRate:
		// input filter resonance, ratio
		vst_strncpy(label, " Rate ", kVstMaxParamStrLen);
		break;
	case kParamDepth:
		// input filter resonance, ratio
		vst_strncpy(label, " Depth ", kVstMaxParamStrLen);
		break;
	default :
		*label = '\0';
		break;
	};
}

//------------------------------------------------------------------------------
void WahWah::getParameterDisplay(VstInt32 index, char *text)
{
	// return parameter value as text
	switch (index)
	{
	case kParamGain:
		// input filter gain, dB
		float2string(GainValue, text, kVstMaxParamStrLen);
		break;

	case kParamFc:
		// input filter center frequency, Hz
		float2string(FcValue, text, kVstMaxParamStrLen);
		break;

	case kParamQ:
		// input filter resonance, ratio
		float2string(QValue, text, kVstMaxParamStrLen);
		break;
			
	case kParamRate:
		// input filter resonance, ratio
		float2string(RateValue, text, kVstMaxParamStrLen);
		break;
			
	case kParamDepth:
		// input filter resonance, ratio
		float2string(DepthValue, text, kVstMaxParamStrLen);
		break;
	default :
		*text = '\0';
		break;
	};
}

//------------------------------------------------------------------------------
void WahWah::getParameterLabel(VstInt32 index, char *label)
{
	switch (index)
	{
	case kParamGain:
		// input filter gain, dB
		vst_strncpy(label, " dB ", kVstMaxParamStrLen);
		break;

	case kParamFc:
		// input filter center frequency, Hz
		vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
		break;

	case kParamQ:
		// input filter resonance, ratio
		vst_strncpy(label, " Q ", kVstMaxParamStrLen);
		break;
			
	case kParamRate:
		// input filter resonance, ratio
		vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
		break;
	case kParamDepth:
		// input filter resonance, ratio
		vst_strncpy(label, " Ratio ", kVstMaxParamStrLen);
		break;

	default :
		*label = '\0';
		break;
	};
}

//------------------------------------------------------------------------------
bool WahWah::getEffectName (char* name)
{
	vst_strncpy (name, "WahWah", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------------
bool WahWah::getProductString (char* text)
{
	vst_strncpy (text, "WahWah", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------------
bool WahWah::getVendorString (char* text)
{
	vst_strncpy (text, "Stanford/CCRMA MUS424", kVstMaxVendorStrLen);
	return true;
}

//------------------------------------------------------------------------------
VstInt32 WahWah::getVendorVersion ()
{ 
	return 1000; 
}

float WahWah::frequencyComputer(float fc, float rate, float depth)
{
    // PROBLEM 2C //
	float floSignal = pow(depth, .5*LFO(rate));
	
	// Slew the modulation signal as well
	modSlewer.process(floSignal, floSignal);
	
	return fc*floSignal;
	
}


//------------------------------------------------------------------------------
void WahWah::processReplacing(float **inputs, float **outputs, 
                              VstInt32 sampleFrames)
// overwrite output
{

	float*	in0 = inputs[0];
	float*  in1 = inputs[1];
	float*	out0 = outputs[0];
	float*  out1 = outputs[1];
    
    float level_estimate, cutoff_freq;
    float tot;

	for (int i = 0; i < sampleFrames; i++)
	{
		// assign input
		float inp0 = (*in0);
		float inp1 = (*in1);
		float signal = (inp0 + inp1)/2;

        // PROBLEM 2D //
        // detect the level of the right channel to compute the cutoff frequency: uses peak detection (not RMS)
		lpFilter.detectLevel(inp1,level_estimate,fs);
        // stereo side-chain: wc:=wc*e^(level*log10(rho))
        cutoff_freq = FcValue * exp(level_estimate * log10(DepthValue));
		
        // slew control signals
        // UNCOMMENT THIS UP TO PROBLEM 2D //
		// fcSlewer.process(FcValue, FcValue);
        fcSlewer.process(cutoff_freq, cutoff_freq);
		gSlewer.process(GainValue, GainValue);
		qSlewer.process(QValue, QValue);
		
		// modulation signal
		float modulationSignal  = frequencyComputer(FcValue, RateValue, DepthValue);
		
		// design new input filter
		designResonantLowPass(lpCoefs, modulationSignal, QValue);
		
		// set the filter stucture coefs
		lpFilter.setCoefs(lpCoefs);
        
		// apply input gain, input filter
		lpFilter.process(signal, signal);
		
		// apply gain, assign output
		*out0++ = signal*drive;
		*out1++ = signal*drive;
		
		// update input pointers
		in0++;in1++;
		
	}
}


//------------------------------------------------------------------------------
float WahWah::LFO(float w0)
{
    // PROBLEM 2B //
	///////////////START//////////////////
    // double omega0 = w0;
    ph = fmodf(ph + w0/fs, 2*pi);
    
	return sin(ph);
	////////////////END/////////////////////
	
}


//------------------------------------------------------------------------------
void WahWah::bilinearTranform(float acoefs[], float dcoefs[])
{
	double b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	double bz0, bz1, bz2, az0, az1, az2;	// coefs for discrete-time filter.
    double c, alpha, beta;
	
	// For easier looking code...unpack
	b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2]; 
    a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];
	
	
	// TODO: apply bilinear transform
    // FROM PROBLEM 1B //
	///////////////START//////////////////
	c = 2 * fs;
    // normalization stuff
    alpha = (c * c)*a2 + c*a1 + a0;
    beta = (c * c)*b2 + c*b1 + b0;
    // coefficients
    az0 = ((c * c)*a2 + c*a1 + a0)/alpha; // 4 * fs * fs * a2 + 2 * fs * a1 + a0
    az1 = 2*(-(c * c) * a2 + a0)/alpha; // -8 * fs * fs * a2 + 2 * a0
    az2 = ((c * c)*a2 - c * a1 + a0)/alpha; // 4 * fs * fs * a2 - 2 * fs * a1 + a0
    bz0 = ((c * c)*b2 + c*b1 + b0)/alpha; // 4 * fs * fs * b2
    bz1 = 2*(-(c * c)*b2 + b0)/alpha; // -8 * fs * fs * b2 + 2 * b0
    bz2 = ((c * c)*b2 - c * b1 + b0)/alpha; // 4 * fs * fs * b2 - c * fs * b1 + b0
    ////////////////END/////////////////////
	
    
	// return coefficients to the output
	dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2; 
    dcoefs[3] = az1; dcoefs[4] = az2;

}


//------------------------------------------------------------------------------
void WahWah::designResonantLowPass(float* dcoefs, float center, float qval)
// design parametric filter based on input center frequency, gain, Q and sampling rate
{
	float b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	float acoefs[6];

	//Design parametric filter here. Filter should be of the form
	//
	//    2
	// b2s  + b1s + b0
	// ---------------
	//    2
	// a2s  + a1s + a0
	//
	// Parameters are center frequency in Hz, gain in dB, and Q.
	
    
	//TODO: design analog filter based on input gain, center frequency and Q	
 	///////////////START//////////////////
	b0 = 1.0; b1 = 0.0; b2 = 0.0;
    a0 = 1.0; a1 = 1.0/(center*qval); a2 = 1.0/powf(center,2);
    ////////////////END/////////////////////
    
    
	// pack the analog coeffs into an array and apply the bilinear tranform
	acoefs[0] = b0; acoefs[1] = b1; acoefs[2] = b2; 
    acoefs[3] = a0; acoefs[4] = a1; acoefs[5] = a2;
	
	// inputs the 6 analog coeffs, output the 5 digital coeffs
	bilinearTranform(acoefs, dcoefs);
	
}




