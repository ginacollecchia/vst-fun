//------------------------------------------------------------------------------
// VST Effect Plug-in
//
// Filename     : Reverb.cpp
// Created by   : Regina Collecchia + music424 staff
// Company      : Stanford
// Description  : Implements a feedback delay network, length specified by fc.
//              Adds a parametric filter for EQ.
//
// Date         : 5/8/14
//------------------------------------------------------------------------------

#include "Reverb.h"
#include <math.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new Reverb (audioMaster);
}

//------------------------------------------------------------------------------
Reverb::Reverb (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, kNumParams)	// 1 program, 1 parameter only
{
	setNumInputs (kNumInputs);		// stereo in
	setNumOutputs (kNumOutputs);		// stereo out
	setUniqueID ('Rvrb');	// identify
	canProcessReplacing ();	// supports replacing output
    
    
	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
    
	// internal state var declaration and initialization
    
	fs = getSampleRate();
    
	pcoefs = coefs;									// pointer for filter coef assignment
    
        
    T60LowValue = 3.5;      // time, in seconds, for low freqs to decay 60dB
	T60LowKnob = SmartKnob::value2knob(T60LowValue, T60LowLimits, T60LowTaper);
    
	T60HighValue = 0.5;			// time, in seconds, for high freqs to decay 60dB
	T60HighKnob = SmartKnob::value2knob(T60HighValue, T60HighLimits, T60HighTaper);

	TransitionValue = 6700.0;			// transition freq, in Hz, between low and high freqs
	TransitionKnob = SmartKnob::value2knob(TransitionValue, TransitionLimits, TransitionTaper);
    
	WetDryKnob = 0.2;		// output (wet/dry) mix
    
	for(int i=0; i<kNumDelays; i++){		
		dl[i].SetDelay(dlens[i]);						// set reverb delay lengths
		designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);	// design filters for feedback loop
		fbfilt[i].SetCoefs(coefs);							// assign filter coefs
	}
    
    ParametricFcValue = 5000.0;
    ParametricFcKnob = SmartKnob::value2knob(ParametricFcValue, ParametricFcLimits, ParametricFcTaper);
    ParametricGammaValue = dB2mag(0.0);
    ParametricGammaKnob = SmartKnob::value2knob(ParametricGammaValue, ParametricGammaLimits, ParametricGammaTaper);
    ParametricQValue = 1;
    ParametricQKnob = SmartKnob::value2knob(ParametricQValue, ParametricQLimits, ParametricQTaper);
    
    designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
	parametricLeft.setCoefs(parametric_coefs);
	parametricRight.setCoefs(parametric_coefs);

    
}

//------------------------------------------------------------------------------
Reverb::~Reverb ()
{
	// nothing to do here
}

//------------------------------------------------------------------------------
void Reverb::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void Reverb::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//------------------------------------------------------------------------------
void Reverb::setParameter (VstInt32 index, float value)
{
	switch (index)
	{
        case kParamT60low:
            T60LowKnob = value;
            T60LowValue = SmartKnob::knob2value(T60LowKnob, T60LowLimits, T60LowTaper);
            for (int i = 0; i < kNumDelays; i++){
                designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);
                fbfilt[i].SetCoefs(coefs);
            }
            break;
        case kParamT60high:
            T60HighKnob = value;
            T60HighValue = SmartKnob::knob2value(T60HighKnob, T60HighLimits, T60HighTaper);
            for (int i=0; i<kNumDelays; i++){
                designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);
                fbfilt[i].SetCoefs(coefs);
            }
            break;
        case kParamTransition:
            TransitionKnob = value;
            TransitionValue = SmartKnob::knob2value(TransitionKnob, TransitionLimits, TransitionTaper);
            for (int i=0; i < kNumDelays; i++){
                designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);
                fbfilt[i].SetCoefs(coefs);
            }
            break;
        case kParamWetDry:
            WetDryKnob = value;
            break;
        case kParamQ:
            ParametricQKnob = value;
            ParametricQValue = SmartKnob::knob2value(ParametricQKnob, ParametricQLimits, ParametricQTaper);
            designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
            parametricLeft.setCoefs(parametric_coefs);
            parametricRight.setCoefs(parametric_coefs);
            break;
        case kParamGamma:
            ParametricGammaKnob = value;
            ParametricGammaValue = dB2mag(SmartKnob::knob2value(ParametricGammaKnob, ParametricGammaLimits, ParametricGammaTaper));
            designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
            parametricLeft.setCoefs(parametric_coefs);
            parametricRight.setCoefs(parametric_coefs);
            break;
        case kParamFc:
            ParametricFcKnob = value;
            ParametricFcValue = SmartKnob::knob2value(ParametricFcKnob, ParametricFcLimits, ParametricFcTaper);
            designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
            parametricLeft.setCoefs(parametric_coefs);
            parametricRight.setCoefs(parametric_coefs);
            break;
            
        default :
            break;
	};
}

//------------------------------------------------------------------------------
float Reverb::getParameter (VstInt32 index)
{
	switch (index)
	{
        case kParamT60low:
            return T60LowKnob;
            break;
        case kParamT60high:
            return T60HighKnob;
            break;
        case kParamTransition:
            return TransitionKnob;
            break;
        case kParamWetDry:
            return WetDryKnob;
            break;
        case kParamQ:
            return ParametricQKnob;
            break;
        case kParamGamma:
            return ParametricGammaKnob;
            break;
        case kParamFc:
            return ParametricFcKnob;
            break;
        default :
            return 0.0;
	};
}

//------------------------------------------------------------------------------
void Reverb::getParameterName (VstInt32 index, char* label)
{
	switch (index)
	{
        case kParamT60low:
            vst_strncpy(label, " T60Low ", kVstMaxParamStrLen);
            break;
        case kParamT60high:
            vst_strncpy(label, " T60High ", kVstMaxParamStrLen);
            break;
        case kParamTransition:
            vst_strncpy(label, " Transition ", kVstMaxParamStrLen);
            break;
        case kParamWetDry:
            vst_strncpy(label, " Wet/Dry ", kVstMaxParamStrLen);
            break;
        case kParamQ:
            vst_strncpy(label, " Q ", kVstMaxParamStrLen);
            break;
        case kParamGamma:
            vst_strncpy(label, " Gain ", kVstMaxParamStrLen);
            break;
        case kParamFc:
            vst_strncpy(label, " Fc ", kVstMaxParamStrLen);
            break;
        default :
            *label = '\0';
            break;
	};
}

//------------------------------------------------------------------------------
void Reverb::getParameterDisplay (VstInt32 index, char* text)
{
	switch (index)
	{
        case kParamT60low:
            float2string(T60LowValue, text, kVstMaxParamStrLen);
            break;
        case kParamT60high:
            float2string(T60HighValue, text, kVstMaxParamStrLen);
            break;
        case kParamTransition:
            float2string(TransitionValue, text, kVstMaxParamStrLen);
            break;
        case kParamWetDry:
            float2string(100.0*WetDryKnob, text, kVstMaxParamStrLen);
            break;
        case kParamQ:
            float2string(ParametricQValue, text, kVstMaxParamStrLen);
            break;
        case kParamGamma:
            float2string(dB(ParametricGammaValue), text, kVstMaxParamStrLen);
            break;
        case kParamFc:
            float2string(ParametricFcValue, text, kVstMaxParamStrLen);
            break;
        default :
            *text = '\0';
            break;
	};
}

//------------------------------------------------------------------------------
void Reverb::getParameterLabel (VstInt32 index, char* label)
{
	switch (index)
	{
        case kParamT60low:
            vst_strncpy(label, " s ", kVstMaxParamStrLen);
            break;
        case kParamT60high:
            vst_strncpy(label, " s ", kVstMaxParamStrLen);
            break;
        case kParamTransition:
            vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
            break;
        case kParamWetDry:
            vst_strncpy(label, " % ", kVstMaxParamStrLen);
            break;
        case kParamQ:
            vst_strncpy(label, " ", kVstMaxParamStrLen);
            break;
        case kParamGamma:
            vst_strncpy(label, " dB ", kVstMaxParamStrLen);
            break;
        case kParamFc:
            vst_strncpy(label, " Hz ", kVstMaxParamStrLen);
            break;
        default :
            *label = '\0';
            break;
	};
}

//------------------------------------------------------------------------
bool Reverb::getEffectName (char* name)
{
	vst_strncpy (name, "Reverb", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool Reverb::getProductString (char* text)
{
	vst_strncpy (text, "Reverb", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool Reverb::getVendorString (char* text)
{
	vst_strncpy (text, "Stanford/CCRMA MUS424", kVstMaxVendorStrLen);
	return true;
}

//------------------------------------------------------------------------------
VstInt32 Reverb::getVendorVersion ()
{ 
	return 1000; 
}


//------------------------------------------------------------------------------
void Reverb::designShelf(double* pcofs, long theLength, double transition, double t60low, double t60high)
{
	double a1,b0,b1,norm,b0z,b1z,a1z,c,Td,omegad;
	double roundTrip=((double)(theLength))/fs;		// get delay length in seconds
	double g0,g1;  //Temp shelf gains at DC and Nyquist
	//  Design Shelf filter here. Must produce 1-pole, 1-zero filter coefficients
	//
	//  b1s + b0
	//  ---------
	//  a1s + 1
	//
	//  These gains must be computed so that, the signal has
	//  been reduced by the factor exp(-t/tau) after a duration
	//  t, where tau is the natural time constant associated 
	//  with the desired T60.
	//  Make the pole of the filter at the
	//  transition frequency (Alternatively, you could try making
	//  the transition frequency equal to the geometric mean of the pole
	//  and the zero and compare). Explain what you are doing for full
	//  credit.
  
  // MODIFY
    g0 = powf(1000, -roundTrip/t60low);
    g1 = powf(1000, -roundTrip/t60high);
    
    // frequency-warping stuff
    omegad = 2 * pi * transition/fs; // ermahgerd
    Td = 2/(2 * pi * transition) * tan(omegad/2);
    c  = 2/Td;
    
//	b0 = 0.9;	// Dummy bypass filter, gain = 0.9
//	b1 = 0.9;	// at all frequencies
//	a1 = 1.0;
    
    b0   = g0;
    b1   = g1/transition;
    norm = 1;
    a1   = 1/transition;
    
    
	// Bilinear transform to make discrete-time filter.
	// This version of the bilinear transform uses the
	// low-frequency approximation to map 's' onto 'z'.
	// Wrap the frequencies so that the
	// transform which will match continuous- and
	// discrete- time frequencies at the transition frequency
	// of the filter.
  
  // MODIFY
	norm = 1+c*a1;			// do bilinear transform
	b0z  = (b1*c+b0)/norm;	// (bilinear)
	b1z  = (b0-b1*c)/norm;	// (bilinear)
	a1z  = (1-a1*c)/norm;	// (bilinear)
    
	*(pcoefs) = b0z;						// return coefs
	*(pcoefs+1) = b1z;
	*(pcoefs+2) = a1z;
    
}



//------------------------------------------------------------------------------
void Reverb::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
	float*	in0		= inputs[0];
	float*  in1     = inputs[1];
	float*	out0	= outputs[0];
	float*  out1    = outputs[1];
    
    double OutL, OutR, outLeft, outRight;
    
	for (int i = 0; i < sampleFrames; i++)
	{        
        // denormal was a problem in Pentium 4 CPUs. This is obsolete now, but 
        // we left it in the code for historical reasons
		int noise=rand()-16384;
		double inp0=(*in0)+(float)(noise)*6.103515623e-015;	// additive noise to prevent denormal
		double inp1=(*in1)+(float)(noise)*6.103515623e-015;	// problem on P4 (bad for reverbs).
        
        // TODO: connect the Parametric section for problem 2
        
		double accum;
		int dlind,sumind;
        
		OutL=0.0;OutR=0.0; // set by paramteric filter when commented out
        
		for (dlind=0; dlind<kNumDelays ; dlind++)
		{
			accum=0.0;
			for (sumind=0; sumind<kNumDelays ; sumind++)		
			{
                // mixing matrix
				// accum+=FB[dlind][sumind]*dl[sumind].Read();			// identity matrix
                accum+=ON[dlind][sumind]*dl[sumind].Read();             // orthonormal matrix
			}
			OutL+=OutVecL[dlind]*accum; OutR+=OutVecR[dlind]*accum;	// sum into L,R busses
			accum+=InVecL[dlind]*inp0+InVecR[dlind]*inp1;			// add in L,R contributions for current line
			fbfilt[dlind].Process(accum,accum);						// filter data with shelf
			dl[dlind].Write(accum);									// write back into current delay line
		}
		for (dlind=0; dlind<kNumDelays ; dlind++)
			dl[dlind].UpdatePointers();								// advance read, write pointers
        
        // apply input gain, input filter
        parametricLeft.process(OutL, outLeft);
        parametricRight.process(OutR, outRight);
        
        
		// *out0++ = OutL*WetDryKnob + inp0*(1.0-WetDryKnob);		// compute wet/dry output: OutL/R is set by reverb
		// *out1++ = OutR*WetDryKnob + inp1*(1.0-WetDryKnob);
        *out0++ = outLeft*WetDryKnob + inp0*(1.0-WetDryKnob);       // compute wet/dry output: OutL/R is set by parametric EQ
		*out1++ = outRight*WetDryKnob + inp1*(1.0-WetDryKnob);
		in0++; in1++;
	}
}



//------------------------------------------------------------------------------
void Reverb::bilinearTransform(double acoefs[], double dcoefs[])
{
	float b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	float bz0, bz1, bz2, az0, az1, az2;	// coefs for discrete-time filter.
	
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
    
	// return coefficients to the output
	dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2;
    dcoefs[3] = az1; dcoefs[4] = az2;
    
}


//------------------------------------------------------------------------------
void Reverb::designParametric(double* dcoefs, double center, double gain, double qval)
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
    a0 = 1.0; a1 = 0.0; a2 = 0.0;
	////////////////END/////////////////////
    
    center *= 2*pi;
    
    // pre-warping
    center = 2*fs*tan(center/(2*fs));
    
    
	b0 = 1.0;
    b1 = 0.0;
    b2 = 0.0;
    
    
    a0 = 1.0;
    a1 = 1.0 / ( center * qval );
    a2 = 1.0 / ( center * center );
	
	////////////////END/////////////////////
	// pack the analog coeffs into an array and apply the bilinear tranform
	acoefs[0] = b0; acoefs[1] = b1; acoefs[2] = b2;
    acoefs[3] = a0; acoefs[4] = a1; acoefs[5] = a2;
	
	// inputs the 6 analog coeffs, output the 5 digital coeffs
	bilinearTransform(acoefs, dcoefs);
	
}
