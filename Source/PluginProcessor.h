#pragma once

#include <JuceHeader.h>

// 1. WAVEFORM SELECTION ENUM
enum class WaveType { Sine = 0, Saw, Square, Triangle };

// 2. SYNTH SOUND DEFINITION
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

// 3. TPT STATE VARIABLE FILTER DEFINITION
struct StateVariableFilter
{
    float g = 0.0f;
    float k = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;

    void setParams (float cutoffHz, float Q, double sampleRate)
    {
        if (sampleRate <= 0.0)
            return;

        float maxCutoff = (float) (sampleRate * 0.49);
        float safeCutoff = juce::jlimit (20.0f, maxCutoff, cutoffHz);
        float safeQ = juce::jlimit (0.5f, 10.0f, Q);

        g = std::tan (juce::MathConstants<float>::pi * safeCutoff / (float) sampleRate);
        k = 1.0f / safeQ;
    }

    void reset()
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    float processLowPass (float input)
    {
        // THE MATH IS FIXED HERE: Removed the rogue 2.0f multiplier
        float hp = (input - (g + k) * s1 - s2) / (1.0f + g * (g + k));
        float bp = g * hp + s1;
        s1 = g * hp + bp;
        float lp = g * bp + s2;
        s2 = g * bp + lp;
        return lp;
    }
};

// 4. POLYPHONIC VOICE DEFINITION
class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SynthSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        currentPitch = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        phase = 0.0;
        noteVelocity = velocity;
        filter.reset();
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (! allowTailOff || ! adsr.isActive())
            clearCurrentNote();
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void prepareToPlay (double sampleRate, int)
    {
        adsr.setSampleRate (sampleRate);
        currentSampleRate = sampleRate;
        filter.reset();
    }

    void updateParameters (WaveType wave, const juce::ADSR::Parameters& params, float gain, float cutoff, float resonance)
    {
        currentWave = wave;
        adsr.setParameters (params);
        masterGain = gain;
        filter.setParams (cutoff, resonance, currentSampleRate);
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (! isVoiceActive())
            return;

        while (--numSamples >= 0)
        {
            if (currentSampleRate <= 0.0)
                break;

            double phaseIncrement = (currentPitch / currentSampleRate) * juce::MathConstants<double>::twoPi;
            phase += phaseIncrement;
            if (phase >= juce::MathConstants<double>::twoPi)
                phase -= juce::MathConstants<double>::twoPi;

            float sample = 0.0f;

            switch (currentWave)
            {
                case WaveType::Sine:
                    sample = (float) std::sin (phase);
                    break;
                case WaveType::Saw:
                    sample = (float) (1.0 - (phase / juce::MathConstants<double>::pi));
                    break;
                case WaveType::Square:
                    sample = (phase < juce::MathConstants<double>::pi) ? 1.0f : -1.0f;
                    break;
                case WaveType::Triangle:
                    sample = (float) (2.0 * std::abs (2.0 * (phase / juce::MathConstants<double>::twoPi) - 1.0) - 1.0);
                    break;
            }

            float adsrValue = adsr.getNextSample();
            float rawSample = sample * noteVelocity * masterGain * adsrValue;
            
            float finalSample = filter.processLowPass (rawSample);

            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
                outputBuffer.addSample (channel, startSample, finalSample);

            startSample++;

            if (! adsr.isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    juce::ADSR adsr;
    StateVariableFilter filter;
    double currentSampleRate = 44100.0;
    double currentPitch = 440.0;
    double phase = 0.0;
    float noteVelocity = 0.0f;
    float masterGain = 0.2f;
    WaveType currentWave = WaveType::Saw;
};

//==============================================================================
class CortexiaAudioProcessor  : public juce::AudioProcessor
{
public:
    CortexiaAudioProcessor();
    ~CortexiaAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    std::array<float, 512> scopeData;
    std::atomic<bool> nextFrameReady { false };
    int scopePos = 0;

    void pushNextSampleIntoScope (float sample);

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessor)
};