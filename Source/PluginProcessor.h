#pragma once

#include <JuceHeader.h>
#include <atomic>

// 1. WAVEFORM SELECTION ENUM
enum class WaveType { Sine = 0, Saw, Square, Triangle };

// 2. SYNTH SOUND DEFINITION
struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

// 3. LFO GENERATOR
struct LFO
{
    double currentPhase = 0.0;
    double phaseIncrement = 0.0;
    double sampleRate = 44100.0;

    void setParameters (float frequency, double sr)
    {
        sampleRate = sr;
        phaseIncrement = (frequency / sampleRate) * juce::MathConstants<double>::twoPi;
    }

    float getNextSample()
    {
        currentPhase += phaseIncrement;
        if (currentPhase >= juce::MathConstants<double>::twoPi)
            currentPhase -= juce::MathConstants<double>::twoPi;

        return (float) std::sin (currentPhase);
    }
};

// 4. TPT STATE VARIABLE FILTER DEFINITION
struct StateVariableFilter
{
    float g = 0.0f;
    float k = 0.0f;
    float s1 = 0.0f;
    float s2 = 0.0f;

    void setParams (float cutoffHz, float Q, double sampleRate)
    {
        if (sampleRate <= 0.0) return;

        float maxCutoff = (float) (sampleRate * 0.49);
        float safeCutoff = juce::jlimit (20.0f, maxCutoff, cutoffHz);
        float safeQ = juce::jlimit (0.5f, 10.0f, Q);

        g = std::tan (juce::MathConstants<float>::pi * safeCutoff / (float) sampleRate);
        k = 1.0f / safeQ;
    }

    void reset() { s1 = 0.0f; s2 = 0.0f; }

    float processLowPass (float input)
    {
        float hp = (input - (g + k) * s1 - s2) / (1.0f + g * (g + k));
        float bp = g * hp + s1;
        s1 = g * hp + bp;
        float lp = g * bp + s2;
        s2 = g * bp + lp;
        return lp;
    }
};

// 5. POLYPHONIC VOICE DEFINITION
class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice() {}

    bool canPlaySound (juce::SynthesiserSound* sound) override { return dynamic_cast<SynthSound*> (sound) != nullptr; }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override
    {
        currentPitch = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        phase1 = 0.0;
        phase2 = 0.0;
        noteVelocity = velocity;
        filter.reset();
        adsr.noteOn();
    }

    void stopNote (float, bool allowTailOff) override
    {
        adsr.noteOff();
        if (! allowTailOff || ! adsr.isActive()) clearCurrentNote();
    }

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void prepareToPlay (double sampleRate, int)
    {
        adsr.setSampleRate (sampleRate);
        currentSampleRate = sampleRate;
        filter.reset();
    }

    void updateParameters (WaveType wave1, float gain1, float tune1, float detune1,
                           WaveType wave2, float gain2, float tune2, float detune2,
                           const juce::ADSR::Parameters& params, float cutoff, 
                           float resonance, float lfoRate, float lfoDepth)
    {
        currentWave1 = wave1;
        masterGain1 = gain1;
        osc1Tune = tune1;
        osc1Detune = detune1;
        
        currentWave2 = wave2;
        masterGain2 = gain2;
        osc2Tune = tune2;
        osc2Detune = detune2;

        adsr.setParameters (params);
        baseCutoff = cutoff;
        filterResonance = resonance;
        filterLFO.setParameters (lfoRate, currentSampleRate);
        modDepth = lfoDepth;
    }

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        if (! isVoiceActive()) return;

        while (--numSamples >= 0)
        {
            if (currentSampleRate <= 0.0) break;

            // OSC 1 PITCH & PHASE (Now with Tune/Detune)
            double pitch1 = currentPitch * std::pow (2.0, (osc1Tune + (osc1Detune / 100.0)) / 12.0);
            double phaseInc1 = (pitch1 / currentSampleRate) * juce::MathConstants<double>::twoPi;
            phase1 += phaseInc1;
            if (phase1 >= juce::MathConstants<double>::twoPi) phase1 -= juce::MathConstants<double>::twoPi;

            // OSC 2 PITCH & PHASE
            double pitch2 = currentPitch * std::pow (2.0, (osc2Tune + (osc2Detune / 100.0)) / 12.0);
            double phaseInc2 = (pitch2 / currentSampleRate) * juce::MathConstants<double>::twoPi;
            phase2 += phaseInc2;
            if (phase2 >= juce::MathConstants<double>::twoPi) phase2 -= juce::MathConstants<double>::twoPi;

            // GENERATE OSC 1
            float sample1 = 0.0f;
            switch (currentWave1)
            {
                case WaveType::Sine:     sample1 = (float) std::sin (phase1); break;
                case WaveType::Saw:      sample1 = (float) (1.0 - (phase1 / juce::MathConstants<double>::pi)); break;
                case WaveType::Square:   sample1 = (phase1 < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
                case WaveType::Triangle: sample1 = (float) (2.0 * std::abs (2.0 * (phase1 / juce::MathConstants<double>::twoPi) - 1.0) - 1.0); break;
            }

            // GENERATE OSC 2
            float sample2 = 0.0f;
            switch (currentWave2)
            {
                case WaveType::Sine:     sample2 = (float) std::sin (phase2); break;
                case WaveType::Saw:      sample2 = (float) (1.0 - (phase2 / juce::MathConstants<double>::pi)); break;
                case WaveType::Square:   sample2 = (phase2 < juce::MathConstants<double>::pi) ? 1.0f : -1.0f; break;
                case WaveType::Triangle: sample2 = (float) (2.0 * std::abs (2.0 * (phase2 / juce::MathConstants<double>::twoPi) - 1.0) - 1.0); break;
            }

            // MIX OSCILLATORS AND APPLY ENVELOPE
            float adsrValue = adsr.getNextSample();
            float mixedSample = ((sample1 * masterGain1) + (sample2 * masterGain2)) * noteVelocity * adsrValue;
            
            // FILTER MODULATION
            float lfoVal = filterLFO.getNextSample();
            float modulatedCutoff = baseCutoff + (lfoVal * modDepth * baseCutoff);
            filter.setParams (modulatedCutoff, filterResonance, currentSampleRate);
            
            float finalSample = filter.processLowPass (mixedSample);

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
    LFO filterLFO;
    
    double currentSampleRate = 44100.0;
    double currentPitch = 440.0;
    float noteVelocity = 0.0f;

    // OSC 1 Variables
    double phase1 = 0.0;
    float masterGain1 = 0.2f;
    WaveType currentWave1 = WaveType::Saw;
    float osc1Tune = 0.0f;
    float osc1Detune = 0.0f;

    // OSC 2 Variables
    double phase2 = 0.0;
    float masterGain2 = 0.0f;
    WaveType currentWave2 = WaveType::Sine;
    float osc2Tune = 0.0f;
    float osc2Detune = 0.0f;

    // Filter Variables
    float baseCutoff = 20000.0f;
    float filterResonance = 0.707f;
    float modDepth = 0.0f;
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
    
    // Thread-safe pointer to feed the oscilloscope
    std::atomic<juce::AudioVisualiserComponent*> visualizer { nullptr };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::Synthesiser synth;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessor)
};