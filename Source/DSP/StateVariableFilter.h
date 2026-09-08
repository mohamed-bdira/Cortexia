#pragma once
#include <JuceHeader.h>

struct StateVariableFilter
{
    float g = 0.0f;
    float k = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;

    void setParams (float cutoffHz, float Q, double sampleRate);
    void reset();
    float processLowPass (float input);
};