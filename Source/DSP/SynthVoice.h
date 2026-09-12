#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"
#include "LFO.h"
#include "StateVariableFilter.h"

struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice();

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote (float, bool allowTailOff) override;
    void pitchWheelMoved (int) override;
    void controllerMoved (int, int) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock);

    void updateParameters (WaveType wave1, float gain1, float tune1, float detune1, int uni1, float uDet1, float uBlnd1, float wtPos1,
                           WaveType wave2, float gain2, float tune2, float detune2, int uni2, float uDet2, float uBlnd2, float wtPos2,
                           const juce::ADSR::Parameters& params, float cutoff, float resonance,
                           float lfoRate, float lfoDepth, int target, float mVol);

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    juce::ADSR adsr;
    StateVariableFilter filterL, filterR;
    LFO filterLFO;
    Oscillator osc1, osc2;

    double currentSampleRate = 44100.0;
    double currentPitch = 440.0;
    float noteVelocity = 0.0f;
    float masterVol = 1.0f;

    float baseCutoff = 20000.0f;
    float filterResonance = 0.707f;
    float modDepth = 0.0f;
    int currentLfoTarget = 1;
};
