#pragma once
#include <JuceHeader.h>

enum class WaveType { Sine = 0, Saw, Square, Triangle };

class Oscillator
{
public:
    Oscillator();
    void prepareToPlay (double sampleRate);
    void setParameters (WaveType type, float gain, float tune, float detune, int unisonCount, float unisonDetune, float unisonBlend);
    void resetPhases();
    void processNextSample (double basePitch, float& sampleL, float& sampleR);

private:
    double currentSampleRate = 44100.0;
    juce::Random random;
    
    WaveType currentWave = WaveType::Saw;
    float masterGain = 0.2f;
    float oscTune = 0.0f;
    float oscDetune = 0.0f;
    int unisonCount = 1;
    float unisonDetune = 0.0f;
    float unisonBlend = 0.0f;

    double phases[7] = {0.0};
};