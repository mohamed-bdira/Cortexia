#include "Oscillator.h"

Oscillator::Oscillator() {}

void Oscillator::prepareToPlay (double sampleRate)
{
    currentSampleRate = sampleRate;
}

void Oscillator::resetPhases()
{
    for (int i = 0; i < 7; ++i)
        phases[i] = random.nextDouble() * juce::MathConstants<double>::twoPi;
}

void Oscillator::setParameters (WaveType type, float gain, float tune, float detune, int uCount, float uDetune, float uBlend)
{
    currentWave = type;
    masterGain = gain;
    oscTune = tune;
    oscDetune = detune;
    unisonCount = uCount;
    unisonDetune = uDetune;
    unisonBlend = uBlend;
}

void Oscillator::processNextSample (double basePitch, float& sampleL, float& sampleR)
{
    sampleL = 0.0f;
    sampleR = 0.0f;

    for (int v = 0; v < unisonCount; ++v)
    {
        float vDetune = 0.0f; float vPan = 0.5f;
        float vGain = (v == 0) ? 1.0f : unisonBlend * (1.0f / std::sqrt(unisonCount - 1.0f));

        if (unisonCount > 1 && v > 0)
        {
            float spread = 1.0f / (unisonCount / 2);
            int pair = (v + 1) / 2;
            float side = (v % 2 == 1) ? -1.0f : 1.0f;
            vPan = 0.5f + (side * 0.5f * (pair * spread));
            vDetune = side * pair * spread * unisonDetune * 50.0f;
        }

        double pitch = basePitch * std::pow (2.0, (oscTune + (oscDetune / 100.0) + (vDetune / 100.0)) / 12.0);
        double phaseInc = (pitch / currentSampleRate) * juce::MathConstants<double>::twoPi;
        
        phases[v] += phaseInc;
        if (phases[v] >= juce::MathConstants<double>::twoPi) phases[v] -= juce::MathConstants<double>::twoPi;

        float waveSample = 0.0f;
        switch (currentWave)
        {
            case WaveType::Sine:     waveSample = (float) std::sin (phases[v]); break;
            case WaveType::Saw:      waveSample = (float) (1.0 - (phases[v] / juce::MathConstants<double>::pi)); break;
            case WaveType::Square:   waveSample = (phases[v] < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
            case WaveType::Triangle: waveSample = (float) (2.0 * std::abs (2.0 * (phases[v] / juce::MathConstants<double>::twoPi) - 1.0) - 1.0); break;
        }
        
        sampleL += waveSample * masterGain * vGain * (1.0f - vPan);
        sampleR += waveSample * masterGain * vGain * vPan;
    }
}