#pragma once
#include <JuceHeader.h>

struct LFO
{
    double currentPhase = 0.0;
    double phaseIncrement = 0.0;
    double sampleRate = 44100.0;

    void setParameters (float frequency, double sr);
    float getNextSample();
};