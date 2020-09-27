/*
  ==============================================================================

    Biquad.h
    Created: 27 Sep 2020 4:40:02pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#pragma once

//------------------------------------------------------------------------------
//  biquad filter section
class Biquad {
    
protected:
    float    b0, b1, b2, a1, a2, z1, z2;
    
public:
    Biquad() {
        this->b0=1.0;
        this->b1=0.0;
        this->b2=0.0;
        this->a1=0.0;
        this->a2=0.0;
        reset();
    }
    
    void setCoefs(float* coefs) {
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
    
    void process (float input, float& output) {
        // process input sample, direct form II transposed
        output = z1 + input*b0;
        z1 = z2 + input*b1 - output*a1;
        z2 = input*b2 - output*a2;
    }
};
