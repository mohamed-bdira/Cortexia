#include "PluginProcessor.h"
#include "PluginEditor.h"

// Define our section color theme
const juce::Colour colOsc (0xff38bdf8); // Cyan
const juce::Colour colFilt(0xfffb923c); // Orange
const juce::Colour colLfo (0xffc084fc); // Purple
const juce::Colour colEnv (0xff4ade80); // Green

//==============================================================================
CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    audioProcessor.visualizer.store (&oscilloscope);

    // OSC 1
    waveSelector1.addItemList ({"Sine", "Sawtooth", "Square", "Triangle"}, 1);
    waveSelector1.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (waveSelector1);
    waveAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC", waveSelector1);
    
    waveLabel1.setText ("WAVEFORM", juce::dontSendNotification);
    waveLabel1.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    waveLabel1.setJustificationType (juce::Justification::centred);
    waveLabel1.setColour (juce::Label::textColourId, juce::Colour (0xff8892b0));
    addAndMakeVisible (waveLabel1);

    setupKnob (volumeSlider1, volumeLabel1, "Volume", colOsc);
    volumeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOLUME", volumeSlider1);
    setupKnob (tuneSlider1, tuneLabel1, "Tune", colOsc);
    tuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE1", tuneSlider1);
    setupKnob (detuneSlider1, detuneLabel1, "Detune", colOsc);
    detuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE1", detuneSlider1);

    // OSC 2
    waveSelector2.addItemList ({"Sine", "Sawtooth", "Square", "Triangle"}, 1);
    waveSelector2.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (waveSelector2);
    waveAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC2", waveSelector2);

    waveLabel2.setText ("WAVEFORM", juce::dontSendNotification);
    waveLabel2.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    waveLabel2.setJustificationType (juce::Justification::centred);
    waveLabel2.setColour (juce::Label::textColourId, juce::Colour (0xff8892b0));
    addAndMakeVisible (waveLabel2);

    setupKnob (volumeSlider2, volumeLabel2, "Volume", colOsc);
    volumeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOL2", volumeSlider2);
    setupKnob (tuneSlider2, tuneLabel2, "Tune", colOsc);
    tuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE2", tuneSlider2);
    setupKnob (detuneSlider2, detuneLabel2, "Detune", colOsc);
    detuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE2", detuneSlider2);

    // FILTER
    setupKnob (cutoffSlider, cutoffLabel, "Cutoff", colFilt);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "CUTOFF", cutoffSlider);
    setupKnob (resonanceSlider, resonanceLabel, "Resonance", colFilt);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RESONANCE", resonanceSlider);

    // LFO
    setupKnob (lfoRateSlider, lfoRateLabel, "Rate", colLfo);
    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO_RATE", lfoRateSlider);
    setupKnob (lfoDepthSlider, lfoDepthLabel, "Depth", colLfo);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO_DEPTH", lfoDepthSlider);

    // ENVELOPE
    setupKnob (attackSlider, attackLabel, "Attack", colEnv);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ATTACK", attackSlider);
    setupKnob (decaySlider, decayLabel, "Decay", colEnv);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DECAY", decaySlider);
    setupKnob (sustainSlider, sustainLabel, "Sustain", colEnv);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SUSTAIN", sustainSlider);
    setupKnob (releaseSlider, releaseLabel, "Release", colEnv);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RELEASE", releaseSlider);

    // OSCILLOSCOPE
    addAndMakeVisible (oscilloscope);
    oscilloscope.setRepaintRate (30);
    oscilloscope.setBufferSize (256);
    oscilloscope.setColours (juce::Colour (0xff090a0c), colOsc); // Super dark background, glowing cyan wave

    setSize (850, 600);
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor()
{
    audioProcessor.visualizer.store (nullptr);
    waveSelector1.setLookAndFeel (nullptr); 
    volumeSlider1.setLookAndFeel (nullptr);
    tuneSlider1.setLookAndFeel (nullptr);
    detuneSlider1.setLookAndFeel (nullptr);
    waveSelector2.setLookAndFeel (nullptr);
    volumeSlider2.setLookAndFeel (nullptr);
    tuneSlider2.setLookAndFeel (nullptr);
    detuneSlider2.setLookAndFeel (nullptr);
    cutoffSlider.setLookAndFeel (nullptr);
    resonanceSlider.setLookAndFeel (nullptr);
    lfoRateSlider.setLookAndFeel (nullptr);
    lfoDepthSlider.setLookAndFeel (nullptr);
    attackSlider.setLookAndFeel (nullptr);
    decaySlider.setLookAndFeel (nullptr);
    sustainSlider.setLookAndFeel (nullptr);
    releaseSlider.setLookAndFeel (nullptr);
}

void CortexiaAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accentColour)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
    slider.setLookAndFeel (&customLookAndFeel); 
    slider.setColour (juce::Slider::rotarySliderFillColourId, accentColour); // Feed color to LookAndFeel
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0)); // Soft white
    addAndMakeVisible (label);
}

void CortexiaAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Deep Charcoal/Blue Gradient Background
    juce::ColourGradient bgGradient(juce::Colour(0xff121418), 0.0f, 0.0f,
                                    juce::Colour(0xff090a0c), 0.0f, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll ();

    // Modern Typography Header
    g.setColour (juce::Colour (0xffffffff).withAlpha(0.9f));
    g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 25, 12, 200, 40, juce::Justification::left);
    
    g.setColour (juce::Colour (0xff8892b0));
    g.setFont (juce::FontOptions (14.0f, juce::Font::italic));
    g.drawText ("VIRTUAL ANALOG", 205, 23, 200, 20, juce::Justification::left);

    // Helper to draw modern layered panels (Updated parameters to float to clear up build warnings)
    auto drawPanel = [&](float x, float y, float w, float h, const juce::String& title, juce::Colour accent)
    {
        // Drop shadow
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (x + 4.0f, y + 4.0f, w, h, 10.0f);
        
        // Main panel body
        g.setColour (juce::Colour (0xff1e2229));
        g.fillRoundedRectangle (x, y, w, h, 10.0f);
        
        // Border stroke
        g.setColour (juce::Colour (0xff2d333b));
        g.drawRoundedRectangle (x, y, w, h, 10.0f, 1.5f);
        
        // Floating neon accent line
        g.setColour (accent);
        g.fillRoundedRectangle (x + 15.0f, y - 2.0f, w - 30.0f, 4.0f, 2.0f);

        // Section Title
        g.setColour (juce::Colour (0xffe2e8f0));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (title, (int)x, (int)y + 12, (int)w, 20, juce::Justification::centred);
    };

    drawPanel (20.0f,  70.0f,  260.0f, 180.0f, "OSCILLATOR 1", colOsc);
    drawPanel (20.0f,  265.0f, 260.0f, 180.0f, "OSCILLATOR 2", colOsc);
    drawPanel (290.0f, 70.0f,  180.0f, 180.0f, "FILTER",       colFilt);
    drawPanel (290.0f, 265.0f, 180.0f, 180.0f, "MODULATION",   colLfo);
    drawPanel (480.0f, 70.0f,  350.0f, 375.0f, "ENVELOPE",     colEnv);
    
    // Sub-panel style for Oscilloscope
    g.setColour (juce::Colour (0xff2d333b));
    g.drawRoundedRectangle (20.0f, 460.0f, 810.0f, 120.0f, 10.0f, 2.0f);
}

void CortexiaAudioProcessorEditor::resized()
{
    // OSC 1
    waveLabel1.setBounds    (85, 105, 130, 20);
    waveSelector1.setBounds (85, 125, 130, 25);
    volumeLabel1.setBounds  (35, 155, 60, 20);
    volumeSlider1.setBounds (35, 175, 60, 65);
    tuneLabel1.setBounds    (100, 155, 60, 20);
    tuneSlider1.setBounds   (100, 175, 60, 65);
    detuneLabel1.setBounds  (165, 155, 60, 20);
    detuneSlider1.setBounds (165, 175, 60, 65);

    // OSC 2
    waveLabel2.setBounds    (85, 300, 130, 20);
    waveSelector2.setBounds (85, 320, 130, 25);
    volumeLabel2.setBounds  (35, 350, 60, 20);
    volumeSlider2.setBounds (35, 370, 60, 65);
    tuneLabel2.setBounds    (100, 350, 60, 20);
    tuneSlider2.setBounds   (100, 370, 60, 65);
    detuneLabel2.setBounds  (165, 350, 60, 20);
    detuneSlider2.setBounds (165, 370, 60, 65);

    // FILTER
    cutoffLabel.setBounds     (300, 115, 75, 20);
    cutoffSlider.setBounds    (300, 135, 75, 95);
    resonanceLabel.setBounds  (385, 115, 75, 20);
    resonanceSlider.setBounds (385, 135, 75, 95);

    // LFO
    lfoRateLabel.setBounds    (300, 310, 75, 20);
    lfoRateSlider.setBounds   (300, 330, 75, 95);
    lfoDepthLabel.setBounds   (385, 310, 75, 20);
    lfoDepthSlider.setBounds  (385, 330, 75, 95);

    // ENVELOPE
    int envY = 180;
    attackLabel.setBounds    (500, envY, 70, 20);
    attackSlider.setBounds   (500, envY + 20, 70, 130);
    decayLabel.setBounds     (580, envY, 70, 20);
    decaySlider.setBounds    (580, envY + 20, 70, 130);
    sustainLabel.setBounds   (660, envY, 70, 20);
    sustainSlider.setBounds  (660, envY + 20, 70, 130);
    releaseLabel.setBounds   (740, envY, 70, 20);
    releaseSlider.setBounds  (740, envY + 20, 70, 130);

    // OSCILLOSCOPE
    oscilloscope.setBounds (22, 462, 806, 116); // Fit inside the stroked border
}