#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// ==============================================================================
// CUSTOM LOOK AND FEEL (MODERN KNOBS & MENUS)
class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        // ComboBox Colors
        setColour (juce::ComboBox::backgroundColourId,             juce::Colour (0xff0a192f));
        setColour (juce::ComboBox::outlineColourId,                juce::Colour (0xff38bdf8));
        setColour (juce::ComboBox::arrowColourId,                  juce::Colour (0xff38bdf8));
        setColour (juce::ComboBox::textColourId,                   juce::Colour (0xffe0f2fe));
        
        // PopupMenu Colors
        setColour (juce::PopupMenu::backgroundColourId,            juce::Colour (0xff0a192f));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff38bdf8));
        setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colour (0xff0a192f));
        setColour (juce::PopupMenu::textColourId,                  juce::Colour (0xffe0f2fe));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override
    {
        auto radius = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        
        // 1. Draw Background Track
        g.setColour (juce::Colour (0xff0a192f));
        juce::Path backgroundArc;
        backgroundArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.strokePath (backgroundArc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // 2. Draw Active Fill Track
        if (slider.isEnabled())
        {
            g.setColour (juce::Colour (0xff38bdf8));
            juce::Path fillArc;
            fillArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
            g.strokePath (fillArc, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        
        // 3. Draw Thumb/Pointer
        juce::Path pointer;
        auto pointerLength = radius * 0.4f;
        auto pointerThickness = 3.0f;
        pointer.addRoundedRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength, 1.5f);
        pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));
        g.setColour (juce::Colour (0xffffffff));
        g.fillPath (pointer);
    }
};

// ==============================================================================
// OSCILLOSCOPE COMPONENT
class OscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
    OscilloscopeComponent (CortexiaAudioProcessor& p) : audioProcessor (p) 
    { 
        startTimerHz (30); 
    }

    void timerCallback() override
    {
        if (audioProcessor.nextFrameReady)
        {
            audioProcessor.nextFrameReady = false;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colour (0x40000000));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);

        g.setColour (juce::Colour (0xff38bdf8)); 
        
        auto width = getWidth();
        auto height = getHeight();
        juce::Path wavePath;

        for (size_t i = 0; i < audioProcessor.scopeData.size(); ++i)
        {
            auto x = juce::jmap (float (i), 0.0f, float (audioProcessor.scopeData.size() - 1), 0.0f, float (width));
            auto y = juce::jmap (audioProcessor.scopeData[i], -1.0f, 1.0f, float (height), 0.0f);

            if (i == 0)
                wavePath.startNewSubPath (x, y);
            else
                wavePath.lineTo (x, y);
        }

        g.strokePath (wavePath, juce::PathStrokeType (2.0f));
    }

private:
    CortexiaAudioProcessor& audioProcessor;
};

// ==============================================================================
class CortexiaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CortexiaAudioProcessorEditor (CortexiaAudioProcessor&);
    ~CortexiaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);

    CortexiaAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    // OSCILLATOR
    juce::ComboBox waveSelector;
    juce::Label waveLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    // FILTER
    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;
    juce::Slider resonanceSlider;
    juce::Label resonanceLabel;

    // ENVELOPE
    juce::Slider attackSlider;
    juce::Label attackLabel;
    juce::Slider decaySlider;
    juce::Label decayLabel;
    juce::Slider sustainSlider;
    juce::Label sustainLabel;
    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<ComboBoxAttachment> waveAttachment;
    std::unique_ptr<SliderAttachment> volumeAttachment;

    std::unique_ptr<SliderAttachment> cutoffAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;

    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    OscilloscopeComponent oscilloscope { audioProcessor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CortexiaAudioProcessorEditor)
};