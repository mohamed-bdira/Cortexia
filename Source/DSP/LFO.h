#pragma once
#include <JuceHeader.h>

// Choice indices are saved in host state: append new entries, never reorder.
enum class LfoShape : int { Sine = 0, Triangle, Saw, Square, SampleHold };

constexpr int kNumLfos = 3;

inline juce::StringArray lfoShapeNames()
{
    return { "Sine", "Triangle", "Saw", "Square", "S&H" };
}

inline juce::StringArray lfoSyncDivisionNames()
{
    return { "4/1", "2/1", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T", "1/4D", "1/8D" };
}

// Length of one LFO cycle in quarter notes, indexed like lfoSyncDivisionNames().
inline double lfoSyncDivisionBeats (int index)
{
    static constexpr double beats[] = { 16.0, 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.125,
                                        2.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0, 1.5, 0.75 };
    return beats[juce::jlimit (0, (int) std::size (beats) - 1, index)];
}

// LFO 1 keeps the original LFO_RATE / LFO_DEPTH IDs so older sessions still load.
inline juce::String lfoParamId (int lfoIndex, const juce::String& suffix)
{
    if (lfoIndex == 0 && (suffix == "RATE" || suffix == "DEPTH"))
        return "LFO_" + suffix;
    return "LFO" + juce::String (lfoIndex + 1) + "_" + suffix;
}

struct LfoSettings
{
    float rateHz = 2.0f;
    float depth = 0.0f;
    LfoShape shape = LfoShape::Sine;
    bool retrigger = true;
};

struct LFO
{
    double phase = 0.0;
    double phaseIncrement = 0.0;
    double sampleRate = 44100.0;
    LfoShape shape = LfoShape::Sine;
    float heldValue = 0.0f;
    juce::Random random;

    void setParameters (float frequency, double sr);
    void setShape (LfoShape newShape) { shape = newShape; }
    void reset();
    float getNextSample();
};
