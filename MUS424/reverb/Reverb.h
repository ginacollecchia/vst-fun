//------------------------------------------------------------------------------
// VST Effect Plug-in
//
// Filename     : Reverb.h
// Created by   : Regina Collecchia + music424 staff
// Company      : Stanford
// Description  : Implements a feedback delay network, length specified by fc.
//              Adds a parametric filter for EQ.
//
// Date         : 5/8/14
//------------------------------------------------------------------------------

#ifndef __Reverb__
#define __Reverb__

#include "public.sdk/source/vst2.x/audioeffectx.h"
#include <math.h>

#ifndef max
#define max(a,b)			(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)			(((a) < (b)) ? (a) : (b))
#endif

#ifndef dB
// if below -100dB, set to -100dB to prevent taking log of zero
#define dB(x)               20.0 * ((x) > 0.00001 ? log10(x) : log10(0.00001))
#endif

#ifndef dB2mag
#define dB2mag(x)           pow( 10.0, (x) / 20.0 )
#endif

#define kMaxLen			32
#define pi              3.1415926536


//------------------------------------------------------------------------------
//  one-pole, one-zero shelf filter
struct HalfBiquad {			
    double	a1, b0, b1, z1;
    
    HalfBiquad()		{	this->a1=0.0; this->b0=1.0; this->b1=0.0; Reset();	}
    void	SetCoefs (double* coefs)		//pointer to array: [b0 b1 b2 a1 a2]
    {	this->a1=*(coefs+2); this->b0=*(coefs); this->b1=*(coefs+1);}
    void	Reset()	{	z1=0; }
    void	Process (double input, double& output)
    {
        //Transposed Direct II Form (PREFERRED)
        output = z1+input*b0;
        z1=input*b1-output*a1;
        
    }
};


//------------------------------------------------------------------------------
//  delay line
#define kMaxDelay 8192
struct DelayLine {			// delay line
    double	dly[kMaxDelay];							// double-precision delay line
    long	wp,rp,theDelay;							// read, write pointers and delay length
    DelayLine()		{
        int indx;
        for (indx=0; indx<kMaxDelay ; indx++){
            dly[indx]=0.0;							// clear delay line on instantiation
        }
        wp=0;rp=0;									// init write and read pointers to zero
    }
    void SetDelay(long aDelay)
    {	theDelay=aDelay;}
    void Write(double data)
    {	dly[wp]=data;}								// write data into line
    double Read()
    {	return dly[rp];}							// read data from line
    void UpdatePointers()							// advance read, write pointers
    {	wp--;
        if(wp<0)
            wp=kMaxDelay-1;
        rp=wp+theDelay;
        if(rp>kMaxDelay-1)
            rp-=kMaxDelay;
        if(rp>kMaxDelay-1)
            rp=kMaxDelay-1;
        if(rp<0)
            rp=0;
    }
};



//------------------------------------------------------------------------------
// signal processing functions
struct Biquad {
    //  biquad filter section
    double	b0, b1, b2, a1, a2, z1, z2;
    
    Biquad() {
        this->b0=1.0;
        this->b1=0.0;
        this->b2=0.0;
        this->a1=0.0;
        this->a2=0.0;
        reset();
    }
    void setCoefs(double* coefs) {
        // set filter coefficients [b0 b1 b2 a1 a2]
        this->b0=*(coefs);
        this->b1=*(coefs+1);
        this->b2=*(coefs+2);
        this->a1=*(coefs+3);
        this->a2=*(coefs+4);
    }
    void reset() {
        // reset filter state
        z1=0;
        z2=0;
    }
    void process (double input, double& output) {
        // process input sample, direct form II transposed
        output = z1 + input*b0;
        z1 = z2 + input*b1 - output*a1;
        z2 = input*b2 - output*a2;
    }
};


//------------------------------------------------------------------------------
class Reverb : public AudioEffectX
{
public:
	Reverb (audioMasterCallback audioMaster);
	~Reverb ();
    
	// Processing
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
    
	// Program
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);
    
	// Parameters
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);
    
	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();
    
	void designShelf(double* pcofs, long theLength, double transition, double T60low, double T60high);
    void bilinearTransform(double acoefs[], double dcoefs[]);
    void designParametric(double* dcoefs, double center, double gain, double qval);

    
    
protected:
	// param IDs
	enum {
		kParamT60low	= 0,
		kParamT60high,
		kParamTransition,
		kParamWetDry,
        kParamQ,
        kParamGamma,
        kParamFc,
		kNumParams
	};
    
    
	// knob vars
	float T60LowKnob, T60LowValue;
	float T60HighKnob, T60HighValue;
	float TransitionKnob, TransitionValue;
	float WetDryKnob; // since the mix in the [0,1] range, we can use the knob value directly
    float ParametricQKnob, ParametricQValue;
    float ParametricGammaKnob, ParametricGammaValue;
    float ParametricFcKnob, ParametricFcValue;
	
	// config
	enum { 
		kNumProgs	= 1,
		kNumInputs	= 2,
		kNumOutputs	= 2
	};
    
	char	programName[kVstMaxProgNameLen + 1];
        
    
	// internal state var declaration and initialization
	double fs;
#define kNumDelays	12
	HalfBiquad fbfilt[kNumDelays];
	DelayLine dl[kNumDelays];
	double coefs[3];
	double*	pcoefs;
    
    // parametric section
    double parametric_coefs[5];
    Biquad parametricLeft;
    Biquad parametricRight;
    
    
};

const static float InVecL[kNumDelays]={1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0};
const static float InVecR[kNumDelays]={0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0};
const static float OutVecL[kNumDelays]={1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0};
const static float OutVecR[kNumDelays]={0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0,0.0,1.0};
const static float dlens[kNumDelays]={2023,2153,2291,2438,2595,2761,2939,3127,3328,3542,3769,4011};


// UI controls limits and tapers
const static float T60LowLimits[2] = {0.01, 10.0};
const static float T60LowTaper = -1;

const static float T60HighLimits[2] = {0.01, 10.0};
const static float T60HighTaper = -1;

const static float TransitionLimits[2] = {50.0, 16000.0};
const static float TransitionTaper = -1;

const static float ParametricGammaLimits[2] = {-24.0, 24.0};
const static float ParametricGammaTaper = 1.0;

const static float ParametricFcLimits[2] = {50.0, 16000.0};
const static float ParametricFcTaper = -1.0;

const static float ParametricQLimits[2] = {0.25, 32.0};
const static float ParametricQTaper = -1.0;


// TODO: replace the identity "mixing" matrix with a matrix that actaully mixes! 
// You can generate the Hadamard matrix or you can create a different
// one using the orthonorm.m matlab script provided.
const static double FB[kNumDelays][kNumDelays]={	
    //Identity Mixing Matrix (no mixing, actually)
    {+1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, +0.00000, },
    {+0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +0.00000, +1.00000, },
   

};

const static double ON[kNumDelays][kNumDelays]={
    // matrix generated from the script orthonorm.m
    {-0.04847, -0.17288, -0.23635, -0.52946, -0.23560, +0.16995, +0.29591, +0.03690, +0.42629, +0.15084, +0.36397, +0.34829, },
    {-0.44432, -0.38392, +0.44898, +0.44657, -0.01030, -0.04918, +0.22425, -0.10791, +0.19304, -0.06870, -0.02037, +0.38383, },
    {-0.16270, -0.38060, -0.37445, +0.12547, -0.00376, +0.18013, +0.26726, +0.07370, +0.30189, -0.21492, -0.27043, -0.59409, },
    {-0.17087, -0.29272, +0.22427, -0.41500, +0.32503, -0.14665, +0.32731, +0.18663, -0.52384, -0.18442, +0.23249, -0.17612, },
    {-0.38613, -0.03649, -0.69953, +0.19442, -0.01122, -0.06413, -0.07095, +0.03154, -0.42670, -0.04673, +0.01413, +0.35737, },
    {+0.55569, -0.29000, -0.04017, +0.24640, -0.40354, +0.05674, +0.40270, -0.23051, -0.37491, +0.14574, +0.03057, +0.02614, },
    {-0.08730, +0.23644, -0.10955, +0.25807, +0.50241, +0.14388, +0.28903, -0.31895, +0.05450, +0.46202, +0.38453, -0.18759, },
    {+0.12821, -0.35224, -0.10105, +0.18830, -0.02110, -0.62189, -0.27787, +0.28836, +0.17454, +0.27222, +0.36841, -0.16113, },
    {+0.01103, -0.28736, -0.06791, -0.33320, +0.25180, -0.28840, -0.06267, -0.42858, +0.01936, +0.38264, -0.55599, +0.10761, },
    {+0.19522, +0.11562, -0.01982, +0.11691, +0.25636, +0.03176, +0.34268, +0.69524, +0.04381, +0.27715, -0.35905, +0.24351, },
    {+0.44237, -0.03685, -0.18095, +0.07652, +0.46317, -0.18330, +0.08495, -0.17141, +0.22201, -0.58825, +0.08792, +0.27671, },
    {+0.16938, -0.47903, +0.03365, +0.03423, +0.28150, +0.61820, -0.47650, +0.10633, -0.08841, +0.10988, +0.09718, +0.10209, },

};



//------------------------------------------------------------------------------
// "static" class to faciliate the knob handling
class SmartKnob {
public:
    // convert knob on [0,1] to value in [limits[0],limits[1]] according to taper
    static float knob2value(float knob, const float *limits, float taper)
    {
        float value;
        if (taper > 0.0) {  // algebraic taper
            value = limits[0] + (limits[1] - limits[0]) * pow(knob, taper);
        } else {            // exponential taper
            value = limits[0] * exp(log(limits[1]/limits[0]) * knob);
        }
        return value;
    };
    
    // convert value in [limits[0],limits[1]] to knob on [0,1] according to taper
    static float value2knob(float value, const float *limits, float taper)
    {
        float knob;
        if (taper > 0.0) {  // algebraic taper
            knob = pow((value - limits[0])/(limits[1] - limits[0]), 1.0/taper);
        } else {            // exponential taper
            knob = log(value/limits[0])/log(limits[1]/limits[0]);
        }
        return knob;
    };
    
};



#endif	// __Reverb__
