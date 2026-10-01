#include "LFO.h"

void LFO::setParameters (float frequency, double sr)
{
    sampleRate = sr;
    phaseIncrement = sampleRate > 0.0 ? (double) frequency / sampleRate : 0.0;
}

void LFO::reset()
{
    phase = 0.0;
    heldValue = random.nextFloat() * 2.0f - 1.0f;
}

float LFO::getNextSample()
{
    phase += phaseIncrement;
    if (phase >= 1.0)
    {
        phase -= std::floor (phase);
        heldValue = random.nextFloat() * 2.0f - 1.0f;
    }

    const float p = (float) phase;
    switch (shape)
    {
        case LfoShape::Triangle:   return p < 0.5f ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);
        case LfoShape::Saw:        return 2.0f * p - 1.0f;
        case LfoShape::Square:     return p < 0.5f ? 1.0f : -1.0f;
        case LfoShape::SampleHold: return heldValue;
        case LfoShape::Sine:
        default:                   return std::sin (p * juce::MathConstants<float>::twoPi);
    }
}
