#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // OSCILLATOR CONTROLS
    waveSelector.addItem ("Sine", 1);
    waveSelector.addItem ("Sawtooth", 2);
    waveSelector.addItem ("Square", 3);
    waveSelector.addItem ("Triangle", 4);

    // Apply our custom styling to the dropdown
    waveSelector.setLookAndFeel (&customLookAndFeel);

    addAndMakeVisible (waveSelector);
    waveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC", waveSelector);

    waveLabel.setText ("Waveform", juce::dontSendNotification);
    waveLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    waveLabel.setJustificationType (juce::Justification::centred);
    waveLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe0f2fe));
    addAndMakeVisible (waveLabel);

    setupKnob (volumeSlider, volumeLabel, "Volume");
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOLUME", volumeSlider);

    // FILTER CONTROLS
    setupKnob (cutoffSlider, cutoffLabel, "Cutoff");
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "CUTOFF", cutoffSlider);

    setupKnob (resonanceSlider, resonanceLabel, "Resonance");
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RESONANCE", resonanceSlider);

    // ENVELOPE CONTROLS
    setupKnob (attackSlider, attackLabel, "Attack");
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ATTACK", attackSlider);

    setupKnob (decaySlider, decayLabel, "Decay");
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DECAY", decaySlider);

    setupKnob (sustainSlider, sustainLabel, "Sustain");
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SUSTAIN", sustainSlider);

    setupKnob (releaseSlider, releaseLabel, "Release");
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RELEASE", releaseSlider);

    addAndMakeVisible (oscilloscope);

    setSize (800, 450);
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor()
{
    // MUST clear the LookAndFeel for all components using it
    waveSelector.setLookAndFeel (nullptr); 
    volumeSlider.setLookAndFeel (nullptr);
    cutoffSlider.setLookAndFeel (nullptr);
    resonanceSlider.setLookAndFeel (nullptr);
    attackSlider.setLookAndFeel (nullptr);
    decaySlider.setLookAndFeel (nullptr);
    sustainSlider.setLookAndFeel (nullptr);
    releaseSlider.setLookAndFeel (nullptr);
}

//==============================================================================
void CortexiaAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
    slider.setLookAndFeel (&customLookAndFeel); 
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId,    juce::Colour (0xffe0f2fe)); 
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffe0f2fe));
    addAndMakeVisible (label);
}

//==============================================================================
void CortexiaAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient backgroundGradient (
        juce::Colour (0xff0B2F4D), 0.0f, 0.0f,
        juce::Colour (0xff5195A6), (float) getWidth(), (float) getHeight(),
        false
    );
    g.setGradientFill (backgroundGradient);
    g.fillAll();

    // Title
    g.setColour (juce::Colour (0xffffffff));
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 20, 15, 200, 30, juce::Justification::left);

    // Panel Backgrounds
    g.setColour (juce::Colour (0x40000000));
    g.fillRoundedRectangle (20, 60, 150, 230, 8.0f);   // Oscillator
    g.fillRoundedRectangle (185, 60, 180, 230, 8.0f);  // Filter
    g.fillRoundedRectangle (380, 60, 400, 230, 8.0f);  // Envelope

    // Panel Titles
    g.setColour (juce::Colour (0xff38bdf8));
    g.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    g.drawText ("OSCILLATOR", 30, 70, 130, 20, juce::Justification::centred);
    g.drawText ("FILTER", 195, 70, 160, 20, juce::Justification::centred);
    g.drawText ("ENVELOPE", 390, 70, 380, 20, juce::Justification::centred);
}

void CortexiaAudioProcessorEditor::resized()
{
    // Oscillator Panel (x=20, w=150)
    waveLabel.setBounds (30, 95, 130, 20);
    waveSelector.setBounds (30, 115, 130, 25);
    volumeLabel.setBounds (30, 160, 130, 20);
    volumeSlider.setBounds (45, 180, 100, 90);

    // Filter Panel (x=185, w=180)
    cutoffLabel.setBounds (195, 105, 75, 20);
    cutoffSlider.setBounds (195, 125, 75, 130);
    resonanceLabel.setBounds (280, 105, 75, 20);
    resonanceSlider.setBounds (280, 125, 75, 130);

    // Envelope Panel (x=380, w=400)
    int adsrY = 105;
    int knobWidth = 85;
    int knobHeight = 150;

    attackLabel.setBounds (395, adsrY, knobWidth, 20);
    attackSlider.setBounds (395, adsrY + 20, knobWidth, knobHeight);

    decayLabel.setBounds (490, adsrY, knobWidth, 20);
    decaySlider.setBounds (490, adsrY + 20, knobWidth, knobHeight);

    sustainLabel.setBounds (585, adsrY, knobWidth, 20);
    sustainSlider.setBounds (585, adsrY + 20, knobWidth, knobHeight);

    releaseLabel.setBounds (680, adsrY, knobWidth, 20);
    releaseSlider.setBounds (680, adsrY + 20, knobWidth, knobHeight);

    // Oscilloscope Bottom Panel
    oscilloscope.setBounds (20, 310, 760, 120); 
}