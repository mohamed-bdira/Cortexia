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

// 3. POLYPHONIC VOICE DEFINITION
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
    }

    void updateParameters (WaveType wave, const juce::ADSR::Parameters& params, float gain)
    {
        currentWave = wave;
        adsr.setParameters (params);
        masterGain = gain;
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

            // Generate Waveform
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
            float finalSample = sample * noteVelocity * masterGain * adsrValue;

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
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override {}
    void setStateInformation (const void* data, int sizeInBytes) override {}

    // Public Synth Parameters
    float synthGain = 0.2f;
    WaveType currentWaveType = WaveType::Saw;
    juce::ADSR::Parameters adsrParams { 0.1f, 0.1f, 0.8f, 0.4f }; // A, D, S, R defaults

private:
    juce::Synthesiser synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessor)
};