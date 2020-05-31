//------------------------------------------------------------------------------
// VST Effect Plug-in
//
// Filename     : Distortion.cpp
// Created by   : Regina Collecchia + music424 staff
// Company      : CCRMA - Stanford University
// Description  : Applies upsampling -> anti-imaging -> distortion ->
//                upsampling -> anti-aliasing
// Date         : 5/4/14
//------------------------------------------------------------------------------

#include "Distortion_1.h"
#include <stdlib.h>

//------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new Distortion (audioMaster);
}

//------------------------------------------------------------------------------
Distortion::Distortion (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, kNumProgs, kNumParams)	// 1 program, 1 parameter only
{
	setNumInputs (kNumInputs);		// stereo in
	setNumOutputs (kNumOutputs);		// stereo out
	setUniqueID ('Dstn');	// identify
	canProcessReplacing ();	// supports replacing output
    
    
	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
    
	// user interface parameter initialization
	DriveValue = (float) 0.0;	// input gain, dB
	DriveKnob = SmartKnob::value2knob(DriveValue, DriveLimits, DriveTaper);
	drive = dB2mag( DriveValue );
    
	LevelValue = (float) 0.0;	// input gain, dB
	LevelKnob = SmartKnob::value2knob(LevelValue, LevelLimits, LevelTaper);
	level = dB2mag( LevelValue );
    
	GainInValue = (float) 0.0;	// input filter gain, dB
	GainInKnob = SmartKnob::value2knob(GainInValue, GainInLimits, GainInTaper);
    
	FcInValue = (float) 1000.0;	// input filter center frequency, Hz
	FcInKnob = SmartKnob::value2knob(FcInValue, FcInLimits, FcInTaper);
    
	QInValue = (float) 5.0;	// input filter resonance, ratio
	QInKnob = SmartKnob::value2knob(QInValue, QInLimits, QInTaper);
    
	GainOutValue = (float) 0.0;	// input filter gain, dB
	GainOutKnob = SmartKnob::value2knob(GainOutValue, GainOutLimits, GainOutTaper);
    
	FcOutValue = (float) 1000.0;	// input filter center frequency, Hz
	FcOutKnob = SmartKnob::value2knob(FcOutValue, FcOutLimits, FcOutTaper);
    
	QOutValue = (float) 5.0;	// input filter resonance, ratio
	QOutKnob = SmartKnob::value2knob(QOutValue, QOutLimits, QOutTaper);
    
    
	// signal processing parameter and state initialization
	fs = getSampleRate();	// sampling rate, Hz
    
	designParametric(InCoefs, FcInValue, GainInValue, QInValue);
	InFilter.setCoefs(InCoefs);
    
	designParametric(OutCoefs, FcOutValue, GainOutValue, QOutValue);
	OutFilter.setCoefs(OutCoefs);
    
	for (int j = 0; j < kAAOrder; j++) {
		AIFilter[j].setCoefs((double *) AACoefs[j]);
		AAFilter[j].setCoefs((double *) AACoefs[j]);
	}
    
}

//------------------------------------------------------------------------------
Distortion::~Distortion ()
{
	// nothing to do here
}

//------------------------------------------------------------------------------
void Distortion::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void Distortion::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void Distortion::setParameter(VstInt32 index, float value)
{
    
	// assign control and signal processing parameter values
	switch (index)
	{
        case kParamDrive:
            // input gain, dB
            DriveKnob = value;
            DriveValue = SmartKnob::knob2value(DriveKnob, DriveLimits, DriveTaper);
            
            // input gain, amplitude
            drive = dB2mag( DriveValue );
            
            break;
            
        case kParamLevel:
            // output gain, dB
            LevelKnob = value;
            LevelValue = SmartKnob::knob2value(LevelKnob, LevelLimits, LevelTaper);
            
            // input gain, amplitude
            level = dB2mag( LevelValue );
            
            break;
            
        case kParamGainIn:
            // input filter gain, dB
            GainInKnob = value;
            GainInValue = SmartKnob::knob2value(GainInKnob, GainInLimits, GainInTaper);
            
            // design new input filter
            designParametric(InCoefs, FcInValue, GainInValue, QInValue);
            InFilter.setCoefs(InCoefs);
            
            break;
            
        case kParamFcIn:
            // input filter center frequency, Hz
            FcInKnob = value;
            FcInValue = SmartKnob::knob2value(FcInKnob, FcInLimits, FcInTaper);
            
            // design new input filter
            designParametric(InCoefs, FcInValue, GainInValue, QInValue);
            InFilter.setCoefs(InCoefs);
            
            break;
            
        case kParamQIn:
            // input filter resonance, ratio
            QInKnob = value;
            QInValue = SmartKnob::knob2value(QInKnob, QInLimits, QInTaper);
            
            // design new input filter
            designParametric(InCoefs, FcInValue, GainInValue, QInValue);
            InFilter.setCoefs(InCoefs);
            
            break;
            
        case kParamGainOut:
            // output filter gain, dB
            GainOutKnob = value;
            GainOutValue = SmartKnob::knob2value(GainOutKnob, GainOutLimits, GainOutTaper);
            
            // design new output filter
            designParametric(OutCoefs, FcOutValue, GainOutValue, QOutValue);
            OutFilter.setCoefs(OutCoefs);
            
            break;
            
        case kParamFcOut:
            // output filter center frequency, Hz
            FcOutKnob = value;
            FcOutValue = SmartKnob::knob2value(FcOutKnob, FcOutLimits, FcOutTaper);
            
            // design new output filter
            designParametric(OutCoefs, FcOutValue, GainOutValue, QOutValue);
            OutFilter.setCoefs(OutCoefs);
            
            break;
            
        case kParamQOut:
            // output filter resonance, ratio
            QOutKnob = value;
            QOutValue = SmartKnob::knob2value(QOutKnob, QOutLimits, QOutTaper);
            
            // design new output filter
            designParametric(OutCoefs, FcOutValue, GainOutValue, QOutValue);
            OutFilter.setCoefs(OutCoefs);
            
            break;
            
        default :
            break;
	}
    
}


//------------------------------------------------------------------------------
float Distortion::getParameter(VstInt32 index)
{
	// return knob position
	switch (index)
	{
        case kParamDrive:
            // input gain, dB
            return DriveKnob;
            break;
            
        case kParamLevel:
            // output gain, dB
            return LevelKnob;
            break;
            
        case kParamGainIn:
            // input filter gain, dB
            return GainInKnob;
            break;
            
        case kParamFcIn:
            // input filter center frequency, Hz
            return FcInKnob;
            break;
            
        case kParamQIn:
            // input filter resonance, ratio
            return QInKnob;
            break;
            
        case kParamGainOut:
            // output filter gain, dB
            return GainOutKnob;
            break;
            
        case kParamFcOut:
            // output filter center frequency, Hz
            return FcOutKnob;
            break;
            
        case kParamQOut:
            // output filter resonance, ratio
            return QOutKnob;
            break;
            
        default:
            return 0.0;
	}
}

//------------------------------------------------------------------------------
void Distortion::getParameterName(VstInt32 index, char *label)
{
	// return knob name
	switch (index)
	{
        case kParamDrive:
            // input gain, dB
            vst_strncpy(label, " Input Gain ", kVstMaxParamStrLen);
            break;
            
        case kParamLevel:
            // input gain, dB
            vst_strncpy(label, " Output Gain ", kVstMaxParamStrLen);
            break;
            
        case kParamGainIn:
            // input filter gain, dB
            vst_strncpy(label, " Input Filter Gain ", kVstMaxParamStrLen);
            break;
            
        case kParamFcIn:
            // input filter center frequency, Hz
            vst_strncpy(label, " Input Filter Fc ", kVstMaxParamStrLen);
            break;
            
        case kParamQIn:
            // input filter resonance, ratio
            vst_strncpy(label, " Input Filter Q ", kVstMaxParamStrLen);
            break;
            
        case kParamGainOut:
            // output filter gain, dB
            vst_strncpy(label, " Output Filter Gain ", kVstMaxParamStrLen);
            break;
            
        case kParamFcOut:
            // output filter center frequency, Hz
            vst_strncpy(label, " Output Filter Fc ", kVstMaxParamStrLen);
            break;
            
        case kParamQOut:
            // output filter resonance, ratio
            vst_strncpy(label, " Output Filter Q ", kVstMaxParamStrLen);
            break;
            
        default :
            *label = '\0';
            break;
	};
}

//------------------------------------------------------------------------------
void Distortion::getParameterDisplay(VstInt32 index, char *text)
{
	// return parameter value as text
	switch (index)
	{
        case kParamDrive:
            // input gain, dB
            float2string(DriveValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamLevel:
            // output gain, dB
            float2string(LevelValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamGainIn:
            // input filter gain, dB
            float2string(GainInValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamFcIn:
            // input filter center frequency, Hz
            float2string(FcInValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamQIn:
            // input filter resonance, ratio
            float2string(QInValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamGainOut:
            // output filter gain, dB
            float2string(GainOutValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamFcOut:
            // output filter center frequency, Hz
            float2string(FcOutValue, text, kVstMaxParamStrLen);
            break;
            
        case kParamQOut:
            // output filter resonance, ratio
            float2string(QOutValue, text, kVstMaxParamStrLen);
            break;
            
        default :
            *text = '\0';
            break;
	};
}

//------------------------------------------------------------------------------
void Distortion::getParameterLabel(VstInt32 index, char *label)
{
	switch (index)
	{
        case kParamDrive:
            // input gain, dB
            vst_strncpy(label, " dB ", kVstMaxParamStrLen);
            break;
            
        case kParamLevel:
            // output gain, dB
            vst_strncpy(label, " dB ", kVstMaxParamStrLen);
            break;
            
        case kParamGainIn:
            // input filter gain, dB
            vst_strncpy(label, " dB ", kVstMaxParamStrLen);
            break;
            
        case kParamFcIn:
            // input filter center frequency, Hz
            vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
            break;
            
        case kParamQIn:
            // input filter resonance, ratio
            vst_strncpy(label, "    ", kVstMaxParamStrLen);
            break;
            
        case kParamGainOut:
            // output filter gain, dB
            vst_strncpy(label, " dB ", kVstMaxParamStrLen);
            break;
            
        case kParamFcOut:
            // output filter center frequency, Hz
            vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
            break;
            
        case kParamQOut:
            // output filter resonance, ratio
            vst_strncpy(label, "    ", kVstMaxParamStrLen);
            break;
            
        default :
            *label = '\0';
            break;
	};
}

//------------------------------------------------------------------------------
bool Distortion::getEffectName (char* name)
{
	vst_strncpy (name, "Distortion", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------------
bool Distortion::getProductString (char* text)
{
	vst_strncpy (text, "Distortion", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------------
bool Distortion::getVendorString (char* text)
{
	vst_strncpy (text, "Stanford/CCRMA MUS424", kVstMaxVendorStrLen);
	return true;
}

//------------------------------------------------------------------------------
VstInt32 Distortion::getVendorVersion ()
{ 
	return 1000; 
}



//------------------------------------------------------------------------------
void Distortion::processReplacing(float **inputs, float **outputs, 
                                  VstInt32 sampleFrames)
// overwrite output
{
    
	float*	in0 = inputs[0];
	float*  in1 = inputs[1];
	float*	out0 = outputs[0];
	float*  out1 = outputs[1];
    
	double isignal, fsignal, osignal, usignal, dsignal;
    
    double gain = 9.274340825277485e-11;
    
	int i, j, k;
    
	for (i = 0; i < sampleFrames; i++)
	{        
		// assign input
		double inp0 = (*in0);
		double inp1 = (*in1);
        
		isignal = (inp0 + inp1)/2;
        
        
		// apply input gain, input filter
		// InFilter.process(drive*isignal, fsignal);
		
        
		// upsample, apply distortion, downsample
		for (k = 0; k < kUSRatio; k++) {
			// upsample (insert zeros) and apply upsampling gain
			// usignal = (k == kUSRatio - 1) ? kUSRatio*fsignal : 0.0;
            usignal = (k == kUSRatio - 1) ? kUSRatio*isignal : 0.0;
            
			// apply antiimaging filter
			for (j = 0; j < kAAOrder; j++) {
				AIFilter[j].process(usignal,usignal);
			}
            
            usignal = gain*usignal;
            
			// apply distortion
			// note: x / (1+|x|) gives a soft saturation
			// where as min(1, max(-1, x)) gives a hard clipping
			// dsignal = usignal / (1.0 + fabs(usignal));
            dsignal = fmin(1.0, fmax(-1.0, usignal));
            
			// apply antialiasing filter
			for (j = 0; j < kAAOrder; j++) {
				AAFilter[j].process(dsignal,dsignal);
			}
            
            dsignal = gain*dsignal;
            
		}
        
        
        
        
		// apply output gain, output filter
		// OutFilter.process(level*dsignal, osignal);
        
        
		// apply gain, assign output
//		*out0++ = osignal;
//		*out1++ = osignal;
        *out0++ = level*dsignal;
        *out1++ = level*dsignal;
        
		// update input pointers
		in0++;in1++;
	}
}


//------------------------------------------------------------------------------
void Distortion::bilinearTransform(double acoefs[], double dcoefs[])
{
	double b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	double bz0, bz1, bz2, az0, az1, az2;	// coefs for discrete-time filter.
	double c, alpha, beta;
	
	// For easier looking code...unpack
	b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2]; 
    a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];
	
	
	// TODO: apply bilinear transform
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
void Distortion::designParametric(double* dcoefs, double center, double gain, double qval)
// design parametric filter based on input center frequency, gain, Q and sampling rate
{
	double b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	double acoefs[6];
    
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
	bilinearTransform(acoefs, dcoefs);
	
}

