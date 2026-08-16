#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// ==============================================================================
// CUSTOM MODERN LOOK & FEEL
// ==============================================================================
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff171a1f));
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff2d333b));
        setColour (juce::ComboBox::textColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::ComboBox::arrowColourId, juce::Colour (0xff8892b0));
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff171a1f));
        setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe2e8f0));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff38bdf8).withAlpha(0.2f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, const float rotaryStartAngle,
                           const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (4.0f);
        auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = 4.0f;
        auto arcRadius = radius - lineW * 0.5f;

        // Draw Background Track
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                     arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff2d333b));
        g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Draw Value Arc (Dynamic Color)
        auto fillColour = slider.findColour (juce::Slider::rotarySliderFillColourId);
        if (slider.isEnabled())
        {
            juce::Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                    arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
            g.setColour (fillColour);
            g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Draw Inner Knob Body
        g.setColour (juce::Colour (0xff171a1f));
        g.fillEllipse (bounds.getCentreX() - arcRadius + lineW, bounds.getCentreY() - arcRadius + lineW,
                       (arcRadius - lineW) * 2.0f, (arcRadius - lineW) * 2.0f);

        // Draw Pointer/Indicator
        juce::Path pointer;
        auto pointerLength = radius * 0.6f;
        auto pointerThickness = 2.5f;
        pointer.addRectangle (-pointerThickness * 0.5f, -radius + lineW + 3.0f, pointerThickness, pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
        g.setColour (juce::Colour (0xffe2e8f0));
        g.fillPath (pointer);
    }
};

// ==============================================================================
// PLUGIN EDITOR CLASS
// ==============================================================================
class CortexiaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CortexiaAudioProcessorEditor (CortexiaAudioProcessor&);
    ~CortexiaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accentColour);

    CortexiaAudioProcessor& audioProcessor;
    ModernLookAndFeel customLookAndFeel; // Applied our new modern look

    // OSCILLATOR 1
    juce::ComboBox waveSelector1;
    juce::Label waveLabel1;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment1;
    juce::Slider volumeSlider1; juce::Label volumeLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment1;
    juce::Slider tuneSlider1;   juce::Label tuneLabel1;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuneAttachment1;
    juce::Slider detuneSlider1; juce::Label detuneLabel1; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment1;

    // OSCILLATOR 2
    juce::ComboBox waveSelector2;
    juce::Label waveLabel2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment2;
    juce::Slider volumeSlider2; juce::Label volumeLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment2;
    juce::Slider tuneSlider2;   juce::Label tuneLabel2;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuneAttachment2;
    juce::Slider detuneSlider2; juce::Label detuneLabel2; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment2;

    // FILTER
    juce::Slider cutoffSlider;    juce::Label cutoffLabel;    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    juce::Slider resonanceSlider; juce::Label resonanceLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;

    // LFO
    juce::Slider lfoRateSlider;   juce::Label lfoRateLabel;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttachment;
    juce::Slider lfoDepthSlider;  juce::Label lfoDepthLabel;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;

    // ENVELOPE
    juce::Slider attackSlider;  juce::Label attackLabel;  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    juce::Slider decaySlider;   juce::Label decayLabel;   std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    juce::Slider sustainSlider; juce::Label sustainLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    juce::Slider releaseSlider; juce::Label releaseLabel; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    // OSCILLOSCOPE
    juce::AudioVisualiserComponent oscilloscope { 2 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessorEditor)
};