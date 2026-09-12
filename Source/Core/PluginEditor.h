#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ModernLookAndFeel.h"
#include "GUI/WaveformDisplay.h"

class CortexiaAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    CortexiaAudioProcessorEditor (CortexiaAudioProcessor&);
    ~CortexiaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accentColour);
    void layoutKnob (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area);
    void refreshDisplays();
    void drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawBadge (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, juce::Colour colour) const;

    CortexiaAudioProcessor& audioProcessor;
    ModernLookAndFeel customLookAndFeel;

    juce::Slider masterVolSlider;
    juce::Label masterVolLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolAttachment;
    juce::Slider bendRangeSlider;
    juce::Label bendRangeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bendRangeAttachment;

    juce::ComboBox waveSelector1;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment1;
    juce::Slider volumeSlider1; juce::Label volumeLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment1;
    juce::Slider tuneSlider1;   juce::Label tuneLabel1;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuneAttachment1;
    juce::Slider detuneSlider1; juce::Label detuneLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment1;
    juce::Slider unisonSlider1; juce::Label unisonLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonAttachment1;
    juce::Slider uDetuneSlider1;juce::Label uDetuneLabel1;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> uDetuneAttachment1;
    juce::Slider uBlendSlider1; juce::Label uBlendLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> uBlendAttachment1;
    juce::Slider wtPosSlider1;  juce::Label wtPosLabel1;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wtPosAttachment1;

    juce::ComboBox waveSelector2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment2;
    juce::Slider volumeSlider2; juce::Label volumeLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment2;
    juce::Slider tuneSlider2;   juce::Label tuneLabel2;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuneAttachment2;
    juce::Slider detuneSlider2; juce::Label detuneLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment2;
    juce::Slider unisonSlider2; juce::Label unisonLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonAttachment2;
    juce::Slider uDetuneSlider2;juce::Label uDetuneLabel2;std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> uDetuneAttachment2;
    juce::Slider uBlendSlider2; juce::Label uBlendLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> uBlendAttachment2;
    juce::Slider wtPosSlider2;  juce::Label wtPosLabel2;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> wtPosAttachment2;

    juce::Slider cutoffSlider;    juce::Label cutoffLabel;    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    juce::Slider resonanceSlider; juce::Label resonanceLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    juce::ComboBox lfoTargetSelector; juce::Label lfoTargetLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoTargetAttachment;
    juce::Slider lfoRateSlider;       juce::Label lfoRateLabel;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttachment;
    juce::Slider lfoDepthSlider;      juce::Label lfoDepthLabel;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;

    juce::Slider attackSlider;  juce::Label attackLabel;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    juce::Slider decaySlider;   juce::Label decayLabel;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    juce::Slider sustainSlider; juce::Label sustainLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    juce::Slider releaseSlider; juce::Label releaseLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    juce::ComboBox voiceModeSelector;
    juce::Label voiceModeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> voiceModeAttachment;
    juce::Slider portaSlider;
    juce::Label portaLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> portaAttachment;
    juce::ToggleButton alwaysGlideButton;
    juce::Label alwaysGlideLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> alwaysGlideAttachment;

    juce::Slider modWheelSlider;
    juce::Label modWheelLabel;

    WaveformDisplay waveDisplay1, waveDisplay2;
    FilterResponseDisplay filterDisplay;
    EnvelopeGraph envelopeGraph;
    LfoGraph lfoGraph;
    juce::AudioVisualiserComponent oscilloscope { 1 };
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessorEditor)
};
