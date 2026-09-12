#pragma once
#include <JuceHeader.h>
#include <atomic>
#include "DSP/CortexiaSynth.h"

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
    juce::MidiKeyboardState keyboardState;
    std::atomic<juce::AudioVisualiserComponent*> visualizer { nullptr };
    std::atomic<float> lastModWheel { 0.0f };
    std::atomic<float> lastExpression { 0.0f };

    void setModWheel01 (float value);

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    CortexiaSynthesiser synth;
    int lastPitchWheel = 8192;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessor)
};