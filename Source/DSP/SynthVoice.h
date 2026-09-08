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

    void updateParameters (WaveType wave1, float gain1, float tune1, float detune1, int uni1, float uDet1, float uBlnd1,
                           WaveType wave2, float gain2, float tune2, float detune2, int uni2, float uDet2, float uBlnd2,
                           const juce::ADSR::Parameters& params, float cutoff, float resonance, 
                           float lfoRate, float lfoDepth, int target, float mVol);

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

private:
    juce::ADSR adsr;
    StateVariableFilter filterL, filterR;
    LFO filterLFO;
    juce::Random random;
    
    double currentSampleRate = 44100.0;
    double currentPitch = 440.0;
    float noteVelocity = 0.0f;
    float masterVol = 1.0f;

    double phase1[7] = {0.0};
    float masterGain1 = 0.2f;
    WaveType currentWave1 = WaveType::Saw;
    float osc1Tune = 0.0f;
    float osc1Detune = 0.0f;
    int unisonCount1 = 1;
    float unisonDetune1 = 0.0f;
    float unisonBlend1 = 0.0f;

    double phase2[7] = {0.0};
    float masterGain2 = 0.0f;
    WaveType currentWave2 = WaveType::Sine;
    float osc2Tune = 0.0f;
    float osc2Detune = 0.0f;
    int unisonCount2 = 1;
    float unisonDetune2 = 0.0f;
    float unisonBlend2 = 0.0f;

    float baseCutoff = 20000.0f;
    float filterResonance = 0.707f;
    float modDepth = 0.0f;
    int currentLfoTarget = 1;
};