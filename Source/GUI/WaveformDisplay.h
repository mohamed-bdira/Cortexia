#pragma once
#include <JuceHeader.h>
#include "DSP/Wavetable.h"

class WaveformDisplay : public juce::Component
{
public:
    WaveformDisplay();

    void paint (juce::Graphics&) override;
    void setWaveType (int type);
    void setAccentColour (juce::Colour colour);

private:
    int currentWaveType = 1;
    juce::Colour accent { 0xff2ee6c8 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformDisplay)
};

class FilterResponseDisplay : public juce::Component
{
public:
    void setParams (float cutoffHz, float q);
    void paint (juce::Graphics&) override;

private:
    float cutoff = 20000.0f;
    float resonance = 0.707f;
};

class EnvelopeGraph : public juce::Component
{
public:
    void setAdsr (float attack, float decay, float sustain, float release);
    void paint (juce::Graphics&) override;

private:
    float attack = 0.1f, decay = 0.1f, sustain = 0.8f, release = 0.4f;
};

class LfoGraph : public juce::Component
{
public:
    void setDepth (float depth);
    void paint (juce::Graphics&) override;

private:
    float depth = 0.0f;
};
