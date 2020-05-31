//------------------------------------------------------------------------------
// VST Effect Plug-in
//
// Filename     : Reverb.cpp
// Created by   : music424 staff
// Company      : Stanford
// Description  : 
//
// Date         : 5/3/11
//------------------------------------------------------------------------------

#include "ReverbSolution.h"
#include <math.h>
#include <stdlib.h>

#define PRE_WARP true

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
	parametric[0].setCoefs(parametric_coefs);
	parametric[1].setCoefs(parametric_coefs);

    
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
        int i;
        case kParamT60low:
            T60LowKnob = value;
            T60LowValue = SmartKnob::knob2value(T60LowKnob, T60LowLimits, T60LowTaper);
            for (i=0; i<kNumDelays ; i++){
                designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);
                fbfilt[i].SetCoefs(coefs);
            }
            break;
        case kParamT60high:
            T60HighKnob = value;
            T60HighValue = SmartKnob::knob2value(T60HighKnob, T60HighLimits, T60HighTaper);
            for (i=0; i<kNumDelays ; i++){
                designShelf(pcoefs,dlens[i], TransitionValue, T60LowValue, T60HighValue);
                fbfilt[i].SetCoefs(coefs);
            }
            break;
        case kParamTransition:
            TransitionKnob = value;
            TransitionValue = SmartKnob::knob2value(TransitionKnob, TransitionLimits, TransitionTaper);
            for (i=0; i<kNumDelays ; i++){
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
            parametric[0].setCoefs(parametric_coefs);
            parametric[1].setCoefs(parametric_coefs);
            break;
        case kParamGamma:
            ParametricGammaKnob = value;
            ParametricGammaValue = dB2mag(SmartKnob::knob2value(ParametricGammaKnob, ParametricGammaLimits, ParametricGammaTaper));
            designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
            parametric[0].setCoefs(parametric_coefs);
            parametric[1].setCoefs(parametric_coefs);
            break;
        case kParamFc:
            ParametricFcKnob = value;
            ParametricFcValue = SmartKnob::knob2value(ParametricFcKnob, ParametricFcLimits, ParametricFcTaper);
            designParametric(parametric_coefs, ParametricFcValue, ParametricGammaValue, ParametricQValue);
            parametric[0].setCoefs(parametric_coefs);
            parametric[1].setCoefs(parametric_coefs);
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
	vst_strncpy (name, "ReverbSol", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool Reverb::getProductString (char* text)
{
	vst_strncpy (text, "ReverbSol", kVstMaxProductStrLen);
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
	double a1,b0,b1,norm,b0z,b1z,a1z;
	double roundTrip=((double)(theLength))/fs;		// get delay length in seconds
	double g0,g1;  //Temp shelf gains at DC and Nyquist
	//  Design Shelf filter here. Must produce 1-pole, 1-zero filter coefficients
	//
	//  b1s + b0
	//  ---------
	//  a1s + 1
	//
	//  where b0 = asymptotic shelf gain at low frequency,
	//  b1/a1 = asymptotic shelf gain at high frequency.
	//  These gains must be computed so that, for the
	//  given corresponding delay time T, the signal has
	//  been reduced by the factor exp(-T/tau), where tau
	//  is the natural time constant associated with the
	//  desired T60.
	//  It is ok either to put the pole of the filter at the
	//  transition frequency, or to make the transition
	//  frequency equal to the geometric mean of the pole
	//  and the zero. Explain what you are doing for full
	//  credit.
    

    transition *= 2*M_PI; // transition in radians
    
    // There are two options to match the continous and discrete transition 
    // frequency: a) pre-warping; b) modify the bilinear transform
    
#ifdef PRE_WARP
    transition = 2*fs*tan(transition/(2*fs));
#endif


    // analog shelf design
        
    // low frequency asymptote (determined by t60low)
    g0 = exp(roundTrip*log(0.001)/t60low);
    // high frequency asymptote (determined by t60high)
    g1 = exp(roundTrip*log(0.001)/t60high);
    
    
    // There are different options to define rho, which basically controls the
    // transition frequency
    
    // option 1) the pole frequency defines the transition (-3 dB)
    double rho = (transition);	    

//    // option 2) point of maximum phase
//	rho = (transition*sqrt(lpi/l0));
    
    
    // filter coefficients
    b0 = g0;
    a1 = 1.0f/rho;
    b1 = a1*g1;
    
#ifdef PRE_WARP
    // if we pre-warped, we use the unmodified bilinear transform
	norm=1+a1*(2*fs);			// do bilinear transform
	b0z=(b0+b1*(2*fs))/norm;	// (bilinear)
	b1z=(b0-b1*(2*fs))/norm;	// (bilinear)
	a1z=(1-a1*(2*fs))/norm;	// (bilinear)    
#else    
    // Bilinear transform matching the transition frequency between analog and 
    // digital domain
    double c = transition / tan( transition/(2*fs) );
    norm =  1 + a1 * c;
    b0z = (b0 + b1 * c) / norm;
    b1z = (b0 - b1 * c) / norm;
    a1z = ( 1 - a1 * c) / norm;
#endif
    
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
    
	for (int i = 0; i < sampleFrames; i++)
	{        
        // denormal was a problem in Pentium 4 CPUs. This is obsolete now, but 
        // we left it in the code for historical reasons
		int noise=rand()-16384;
		double inp0=(*in0)+(float)(noise)*6.103515623e-015;	// additive noise to prevent denormal
		double inp1=(*in1)+(float)(noise)*6.103515623e-015;	// problem on P4 (bad for reverbs).
        
        // TODO: connect the Parametric section for problem 2
        
//        // parametric section at the input
//        parametric[0].process(inp0, inp0);
//        parametric[1].process(inp1, inp1);
        
        
		double accum,OutL,OutR;
		int dlind,sumind;
        
		OutL=0.0;OutR=0.0;
        
		for (dlind=0; dlind<kNumDelays ; dlind++)
		{
			accum=0.0;
			for (sumind=0; sumind<kNumDelays ; sumind++)		
			{
				accum+=FB[dlind][sumind]*dl[sumind].Read();			// add up contributions from each delay line through orthonormal matrix
			}
			OutL+=OutVecL[dlind]*accum; OutR+=OutVecR[dlind]*accum;	// sum into L,R busses
			accum+=InVecL[dlind]*inp0+InVecR[dlind]*inp1;			// add in L,R contributions for current line
			fbfilt[dlind].Process(accum,accum);						// filter data with shelf
			dl[dlind].Write(accum);									// write back into current delay line
		}
		for (dlind=0; dlind<kNumDelays ; dlind++)
			dl[dlind].UpdatePointers();								// advance read, write pointers

        // parametric section at the output
        // parametric[0].process(OutL, OutL);
        // parametric[1].process(OutR, OutR);
        
        
		*out0++ = OutL*WetDryKnob + inp0*(1.0-WetDryKnob);			// compute wet/dry output
		*out1++ = OutR*WetDryKnob + inp1*(1.0-WetDryKnob);
		in0++;in1++;
	}
}



//------------------------------------------------------------------------------
void Reverb::bilinearTransform(double acoefs[], double dcoefs[])
{
	double b0, b1, b2, a0, a1, a2;		    //storage for continuous-time filter coefs
	double bz0, bz1, bz2, az0, az1, az2;	// coefs for discrete-time filter.
	
	// For easier looking code...unpack
	b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2]; 
    a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];
	
	
	// TODO: apply bilinear transform
	///////////////START//////////////////
    
    double T = 1/fs;
    double Tsq = T*T;
    
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
	
    
	// TODO: design analog filter based on input gain, center frequency and Q
    // Remeber to handle the two cases: boost and cut!
    ///////////////START//////////////////

    // in radians
    center *= 2*M_PI;
    
    // pre-warping
    center = 2*fs*tan(center/(2*fs));
    
    // take gain from dB to linear space
    gain = dB2mag( gain );
    
    
    // analog coeffs
    b0 = 1.0;
    b1 = (gain > 1.0) ? gain / ( center * qval ) : 1.0 / ( center * qval ) ;
    b2 = 1.0 / ( center * center ); 
    
    a0 = 1.0;
    a1 = (gain > 1.0) ? 1.0 / ( center * qval ) : 1.0 / ( center * gain * qval );
    a2 = 1.0 / ( center * center );
    
    ///////////////END//////////////////


	// pack the analog coeffs into an array and apply the bilinear tranform
	acoefs[0] = b0; acoefs[1] = b1; acoefs[2] = b2; 
    acoefs[3] = a0; acoefs[4] = a1; acoefs[5] = a2;
	
	// inputs the 6 analog coeffs, output the 5 digital coeffs
	bilinearTransform(acoefs, dcoefs);
	
}
