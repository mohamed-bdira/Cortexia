#include "LFO.h"

void LFO::setParameters (float frequency, double sr)
{
    sampleRate = sr;
    phaseIncrement = (frequency / sampleRate) * juce::MathConstants<double>::twoPi;
}

float LFO::getNextSample()
{
    currentPhase += phaseIncrement;
    if (currentPhase >= juce::MathConstants<double>::twoPi)
        currentPhase -= juce::MathConstants<double>::twoPi;

    return (float) std::sin (currentPhase);
}