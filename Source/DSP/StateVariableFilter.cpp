#include "StateVariableFilter.h"

void StateVariableFilter::setParams (float cutoffHz, float Q, double sampleRate)
{
    if (sampleRate <= 0.0) return;

    float maxCutoff = (float) (sampleRate * 0.49);
    float safeCutoff = juce::jlimit (20.0f, maxCutoff, cutoffHz);
    float safeQ = juce::jlimit (0.5f, 10.0f, Q);

    g = std::tan (juce::MathConstants<float>::pi * safeCutoff / (float) sampleRate);
    k = 1.0f / safeQ;
}

void StateVariableFilter::reset()
{
    s1 = 0.0f;
    s2 = 0.0f;
}

float StateVariableFilter::processLowPass (float input)
{
    float hp = (input - (g + k) * s1 - s2) / (1.0f + g * (g + k));
    float bp = g * hp + s1;
    s1 = g * hp + bp;
    float lp = g * bp + s2;
    s2 = g * bp + lp;
    return lp;
}