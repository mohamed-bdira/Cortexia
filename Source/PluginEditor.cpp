#include "PluginProcessor.h"
#include "PluginEditor.h"

CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // 1. OSCILLATOR SELECTOR (Styled for Ocean / Cyan theme)
    waveSelector.addItem ("Sine", 1);
    waveSelector.addItem ("Sawtooth", 2);
    waveSelector.addItem ("Square", 3);
    waveSelector.addItem ("Triangle", 4);
    waveSelector.setSelectedId ((int) audioProcessor.currentWaveType + 1);

    waveSelector.setColour (juce::ComboBox::backgroundColourId,           juce::Colour (0xff0a192f));
    waveSelector.setColour (juce::ComboBox::outlineColourId,              juce::Colour (0xff38bdf8));
    waveSelector.setColour (juce::ComboBox::arrowColourId,                juce::Colour (0xff38bdf8));
    waveSelector.setColour (juce::ComboBox::textColourId,                 juce::Colour (0xffe0f2fe));
    waveSelector.setColour (juce::PopupMenu::backgroundColourId,          juce::Colour (0xff0a192f));
    waveSelector.setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff38bdf8));
    waveSelector.setColour (juce::PopupMenu::highlightedTextColourId,       juce::Colour (0xff0a192f));

    waveSelector.onChange = [this]
    {
        audioProcessor.currentWaveType = (WaveType) (waveSelector.getSelectedId() - 1);
    };
    addAndMakeVisible (waveSelector);

    waveLabel.setText ("Oscillator Wave", juce::dontSendNotification);
    waveLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    waveLabel.setJustificationType (juce::Justification::centred);
    waveLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe0f2fe));
    addAndMakeVisible (waveLabel);

    // 2. ADSR KNOBS
    setupKnob (attackSlider, attackLabel, "Attack");
    attackSlider.setRange (0.01, 5.0, 0.01);
    attackSlider.setValue (audioProcessor.adsrParams.attack);
    attackSlider.onValueChange = [this] { audioProcessor.adsrParams.attack = (float) attackSlider.getValue(); };

    setupKnob (decaySlider, decayLabel, "Decay");
    decaySlider.setRange (0.01, 5.0, 0.01);
    decaySlider.setValue (audioProcessor.adsrParams.decay);
    decaySlider.onValueChange = [this] { audioProcessor.adsrParams.decay = (float) decaySlider.getValue(); };

    setupKnob (sustainSlider, sustainLabel, "Sustain");
    sustainSlider.setRange (0.0, 1.0, 0.01);
    sustainSlider.setValue (audioProcessor.adsrParams.sustain);
    sustainSlider.onValueChange = [this] { audioProcessor.adsrParams.sustain = (float) sustainSlider.getValue(); };

    setupKnob (releaseSlider, releaseLabel, "Release");
    releaseSlider.setRange (0.01, 5.0, 0.01);
    releaseSlider.setValue (audioProcessor.adsrParams.release);
    releaseSlider.onValueChange = [this] { audioProcessor.adsrParams.release = (float) releaseSlider.getValue(); };

    // 3. MASTER VOLUME
    setupKnob (volumeSlider, volumeLabel, "Volume");
    volumeSlider.setRange (0.0, 1.0, 0.01);
    volumeSlider.setValue (audioProcessor.synthGain);
    volumeSlider.onValueChange = [this] { audioProcessor.synthGain = (float) volumeSlider.getValue(); };

    setSize (620, 320);
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor() {}

void CortexiaAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
    
    // Electric Cyan Styling
    slider.setColour (juce::Slider::rotarySliderFillColourId,    juce::Colour (0xff38bdf8)); // Electric Cyan
    slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff0a192f)); // Deep Navy Track
    slider.setColour (juce::Slider::thumbColourId,               juce::Colour (0xffffffff)); // Pure White Dot
    slider.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId,         juce::Colour (0xffe0f2fe)); // Light Ice Text
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffe0f2fe));
    addAndMakeVisible (label);
}

void CortexiaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. GRADIENT BACKGROUND (#0B2F4D -> #5195A6)
    juce::ColourGradient backgroundGradient (
        juce::Colour (0xff0B2F4D), 0.0f, 0.0f,
        juce::Colour (0xff5195A6), (float) getWidth(), (float) getHeight(),
        false
    );
    g.setGradientFill (backgroundGradient);
    g.fillAll();

    // 2. TITLE HEADER
    g.setColour (juce::Colour (0xffffffff));
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 20, 15, 200, 30, juce::Justification::left);

    // 3. SECTION CARDS (Dark translucent panels)
    g.setColour (juce::Colour (0x40000000));
    g.fillRoundedRectangle (20, 60, 160, 230, 8.0f);  // Oscillator Panel
    g.fillRoundedRectangle (195, 60, 405, 230, 8.0f); // Envelope Panel

    // 4. PANEL HEADERS (Electric Cyan Accent)
    g.setColour (juce::Colour (0xff38bdf8));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("OSCILLATOR", 30, 70, 140, 20, juce::Justification::centred);
    g.drawText ("ENVELOPE", 210, 70, 375, 20, juce::Justification::centred);
}

void CortexiaAudioProcessorEditor::resized()
{
    // Oscillator Controls
    waveLabel.setBounds (30, 95, 140, 20);
    waveSelector.setBounds (30, 120, 140, 25);

    volumeLabel.setBounds (30, 165, 140, 20);
    volumeSlider.setBounds (50, 185, 100, 90);

    // ADSR Controls
    int adsrY = 105;
    int knobWidth = 85;
    int knobHeight = 150;

    attackLabel.setBounds (210, adsrY, knobWidth, 20);
    attackSlider.setBounds (210, adsrY + 20, knobWidth, knobHeight);

    decayLabel.setBounds (305, adsrY, knobWidth, 20);
    decaySlider.setBounds (305, adsrY + 20, knobWidth, knobHeight);

    sustainLabel.setBounds (400, adsrY, knobWidth, 20);
    sustainSlider.setBounds (400, adsrY + 20, knobWidth, knobHeight);

    releaseLabel.setBounds (495, adsrY, knobWidth, 20);
    releaseSlider.setBounds (495, adsrY + 20, knobWidth, knobHeight);
}