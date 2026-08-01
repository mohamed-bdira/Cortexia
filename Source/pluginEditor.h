#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CortexiaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CortexiaAudioProcessorEditor (CortexiaAudioProcessor&);
    ~CortexiaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CortexiaAudioProcessor& audioProcessor;

    // UI Controls
    juce::ComboBox waveSelector;
    juce::Label    waveLabel;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label  attackLabel, decayLabel, sustainLabel, releaseLabel;

    juce::Slider volumeSlider;
    juce::Label  volumeLabel;

    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessorEditor)
};