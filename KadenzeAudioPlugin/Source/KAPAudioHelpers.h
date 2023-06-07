/*
  ==============================================================================

    KAPAudioHelpers.h
    Created: 8 Jan 2021 12:45:54pm
    Author:  Gina Collecchia

  ==============================================================================
*/

#pragma once

#define kParameterSmoothingCoeff_Generic 0.04
#define kParameterSmoothingCoeff_Fine 0.002

// windows doesn't have these numbers as mac does (M_PI)
const static double kPI = 3.14159265359;
const static double k2PI = 6.28318530718;
const int maxBufferDelaySize = 192000;

inline float kap_linear_interp(float v0, float v1, float t)
{
    return ((1.f - t) * v0) + (t * v1);
}
