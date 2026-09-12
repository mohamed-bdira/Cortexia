#pragma once
#include <JuceHeader.h>
#include "Wavetable.h"

class Oscillator
{
public:
    Oscillator();
    void prepareToPlay (double sampleRate);
    void setParameters (WaveType type, float gain, float tune, float detune,
                        int unisonCount, float unisonDetune, float unisonBlend,
                        float wtPosition);
    void resetPhases();
    void processNextSample (double basePitch, float extraSemitones, float& sampleL, float& sampleR);

private:
    int pickMip (double freqHz, const Wavetable::Frame& frame) const;
    float cubicAt (const std::array<float, Wavetable::kMipStride>& mip, double phase) const;
    float lookup (double phase, double freqHz) const;
    float downsample2x (int voice, float s0, float s1);

    double currentSampleRate = 44100.0;
    juce::Random random;

    const Wavetable* wavetable = nullptr;
    WaveType currentWave = WaveType::Saw;
    float masterGain = 0.2f;
    float oscTune = 0.0f;
    float oscDetune = 0.0f;
    int unisonCount = 1;
    float unisonDetune = 0.0f;
    float unisonBlend = 0.0f;
    float wtPosition = 0.0f;

    double phases[7] = { 0.0 };
    float osHist[7][5] = {};
};
