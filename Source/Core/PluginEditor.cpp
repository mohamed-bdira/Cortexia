#include "PluginProcessor.h"
#include "PluginEditor.h"

const juce::Colour colOsc (0xff38bdf8); // Cyan
const juce::Colour colFilt(0xfffb923c); // Orange
const juce::Colour colLfo (0xffc084fc); // Purple
const juce::Colour colEnv (0xff4ade80); // Green
const juce::Colour colMast(0xfff43f5e); // Rose for Master

CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    audioProcessor.visualizer.store (&oscilloscope);

    // MASTER
    setupKnob (masterVolSlider, masterVolLabel, "Master", colMast);
    masterVolAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "MASTER_VOL", masterVolSlider);

    // OSC 1
    waveSelector1.addItemList ({"Sine", "Sawtooth", "Square", "Triangle"}, 1);
    waveSelector1.setLookAndFeel (&customLookAndFeel);
    waveSelector1.onChange = [this] { repaint(); }; // Trigger repaint to draw new wave
    addAndMakeVisible (waveSelector1);
    waveAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC", waveSelector1);

    setupKnob (volumeSlider1, volumeLabel1, "Vol", colOsc);
    volumeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOLUME", volumeSlider1);
    setupKnob (tuneSlider1, tuneLabel1, "Tune", colOsc);
    tuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE1", tuneSlider1);
    setupKnob (detuneSlider1, detuneLabel1, "Detune", colOsc);
    detuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE1", detuneSlider1);
    setupKnob (unisonSlider1, unisonLabel1, "Voices", colOsc);
    unisonAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UNISON1", unisonSlider1);
    setupKnob (uDetuneSlider1, uDetuneLabel1, "U.Detune", colOsc);
    uDetuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UDETUNE1", uDetuneSlider1);
    setupKnob (uBlendSlider1, uBlendLabel1, "U.Blend", colOsc);
    uBlendAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UBLEND1", uBlendSlider1);

    // OSC 2
    waveSelector2.addItemList ({"Sine", "Sawtooth", "Square", "Triangle"}, 1);
    waveSelector2.setLookAndFeel (&customLookAndFeel);
    waveSelector2.onChange = [this] { repaint(); };
    addAndMakeVisible (waveSelector2);
    waveAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC2", waveSelector2);

    setupKnob (volumeSlider2, volumeLabel2, "Vol", colOsc);
    volumeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOL2", volumeSlider2);
    setupKnob (tuneSlider2, tuneLabel2, "Tune", colOsc);
    tuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE2", tuneSlider2);
    setupKnob (detuneSlider2, detuneLabel2, "Detune", colOsc);
    detuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE2", detuneSlider2);
    setupKnob (unisonSlider2, unisonLabel2, "Voices", colOsc);
    unisonAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UNISON2", unisonSlider2);
    setupKnob (uDetuneSlider2, uDetuneLabel2, "U.Detune", colOsc);
    uDetuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UDETUNE2", uDetuneSlider2);
    setupKnob (uBlendSlider2, uBlendLabel2, "U.Blend", colOsc);
    uBlendAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UBLEND2", uBlendSlider2);

    // FILTER
    setupKnob (cutoffSlider, cutoffLabel, "Cutoff", colFilt);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "CUTOFF", cutoffSlider);
    setupKnob (resonanceSlider, resonanceLabel, "Resonance", colFilt);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RESONANCE", resonanceSlider);

    // LFO
    lfoTargetSelector.addItemList ({"None", "Cutoff", "Pitch 1"}, 1);
    lfoTargetSelector.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (lfoTargetSelector);
    lfoTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "LFO_TARGET", lfoTargetSelector);
    lfoTargetLabel.setText ("TARGET", juce::dontSendNotification);
    lfoTargetLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    lfoTargetLabel.setJustificationType (juce::Justification::centred);
    lfoTargetLabel.setColour (juce::Label::textColourId, juce::Colour (0xff8892b0));
    addAndMakeVisible (lfoTargetLabel);

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

    // OSCILLOSCOPE (Now Top Right)
    addAndMakeVisible (oscilloscope);
    oscilloscope.setRepaintRate (30);
    oscilloscope.setBufferSize (256);
    oscilloscope.setColours (juce::Colour (0xff090a0c), colMast); 

    setSize (1000, 650);
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor()
{
    audioProcessor.visualizer.store (nullptr);
    masterVolSlider.setLookAndFeel (nullptr);
    waveSelector1.setLookAndFeel (nullptr); 
    volumeSlider1.setLookAndFeel (nullptr); tuneSlider1.setLookAndFeel (nullptr); detuneSlider1.setLookAndFeel (nullptr);
    unisonSlider1.setLookAndFeel (nullptr); uDetuneSlider1.setLookAndFeel (nullptr); uBlendSlider1.setLookAndFeel (nullptr);
    waveSelector2.setLookAndFeel (nullptr); 
    volumeSlider2.setLookAndFeel (nullptr); tuneSlider2.setLookAndFeel (nullptr); detuneSlider2.setLookAndFeel (nullptr);
    unisonSlider2.setLookAndFeel (nullptr); uDetuneSlider2.setLookAndFeel (nullptr); uBlendSlider2.setLookAndFeel (nullptr);
    cutoffSlider.setLookAndFeel (nullptr); resonanceSlider.setLookAndFeel (nullptr);
    lfoTargetSelector.setLookAndFeel (nullptr); lfoRateSlider.setLookAndFeel (nullptr); lfoDepthSlider.setLookAndFeel (nullptr);
    attackSlider.setLookAndFeel (nullptr); decaySlider.setLookAndFeel (nullptr); sustainSlider.setLookAndFeel (nullptr); releaseSlider.setLookAndFeel (nullptr);
}

void CortexiaAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accentColour)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 18);
    slider.setLookAndFeel (&customLookAndFeel); 
    slider.setColour (juce::Slider::rotarySliderFillColourId, accentColour); 
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0)); 
    addAndMakeVisible (label);
}

void CortexiaAudioProcessorEditor::drawWaveformPath (juce::Graphics& g, int type, juce::Rectangle<float> bounds, juce::Colour color)
{
    juce::Path p;
    float w = bounds.getWidth();
    float h = bounds.getHeight();
    float cx = bounds.getX();
    float cy = bounds.getY() + h * 0.5f;

    p.startNewSubPath (cx, cy);

    if (type == 0) // Sine
    {
        for (float x = 0; x <= w; x += 1.0f)
            p.lineTo (cx + x, cy - (std::sin (x / w * juce::MathConstants<float>::twoPi) * h * 0.45f));
    }
    else if (type == 1) // Saw
    {
        p.lineTo (cx, cy - h * 0.45f);
        p.lineTo (cx + w, cy + h * 0.45f);
        p.lineTo (cx + w, cy);
    }
    else if (type == 2) // Square
    {
        p.lineTo (cx, cy - h * 0.45f);
        p.lineTo (cx + w * 0.5f, cy - h * 0.45f);
        p.lineTo (cx + w * 0.5f, cy + h * 0.45f);
        p.lineTo (cx + w, cy + h * 0.45f);
        p.lineTo (cx + w, cy);
    }
    else if (type == 3) // Triangle
    {
        p.lineTo (cx + w * 0.25f, cy - h * 0.45f);
        p.lineTo (cx + w * 0.75f, cy + h * 0.45f);
        p.lineTo (cx + w, cy);
    }

    g.setColour (color);
    g.strokePath (p, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void CortexiaAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bgGradient(juce::Colour(0xff121418), 0.0f, 0.0f,
                                    juce::Colour(0xff090a0c), 0.0f, (float)getHeight(), false);
    g.setGradientFill(bgGradient);
    g.fillAll ();

    g.setColour (juce::Colour (0xffffffff).withAlpha(0.9f));
    g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 25, 20, 200, 40, juce::Justification::left);
    
    g.setColour (juce::Colour (0xff8892b0));
    g.setFont (juce::FontOptions (14.0f, juce::Font::italic));
    g.drawText ("VIRTUAL ANALOG", 205, 31, 200, 20, juce::Justification::left);

    auto drawPanel = [&](float x, float y, float w, float h, const juce::String& title, juce::Colour accent)
    {
        g.setColour (juce::Colours::black.withAlpha (0.4f));
        g.fillRoundedRectangle (x + 4.0f, y + 4.0f, w, h, 10.0f);
        g.setColour (juce::Colour (0xff1e2229));
        g.fillRoundedRectangle (x, y, w, h, 10.0f);
        g.setColour (juce::Colour (0xff2d333b));
        g.drawRoundedRectangle (x, y, w, h, 10.0f, 1.5f);
        g.setColour (accent);
        g.fillRoundedRectangle (x + 15.0f, y - 2.0f, w - 30.0f, 4.0f, 2.0f);
        g.setColour (juce::Colour (0xffe2e8f0));
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (title, (int)x, (int)y + 12, (int)w, 20, juce::Justification::centred);
    };

    drawPanel (20.0f,  80.0f,  340.0f, 270.0f, "OSCILLATOR 1", colOsc);
    drawPanel (20.0f,  365.0f, 340.0f, 270.0f, "OSCILLATOR 2", colOsc);
    drawPanel (380.0f, 80.0f,  180.0f, 270.0f, "FILTER",       colFilt);
    drawPanel (380.0f, 365.0f, 180.0f, 270.0f, "MODULATION",   colLfo);
    drawPanel (580.0f, 80.0f,  395.0f, 555.0f, "ENVELOPE",     colEnv);

    // Draw Top Right Oscilloscope Border
    g.setColour (juce::Colour (0xff2d333b));
    g.drawRoundedRectangle (580.0f, 20.0f, 290.0f, 45.0f, 5.0f, 2.0f);

    // Draw Serum-Style Waveform Visualizers
    int w1 = (int) audioProcessor.apvts.getRawParameterValue ("OSC")->load();
    int w2 = (int) audioProcessor.apvts.getRawParameterValue ("OSC2")->load();
    
    // Waveform Background screens
    g.setColour (juce::Colour (0xff090a0c));
    g.fillRoundedRectangle (100.0f, 115.0f, 180.0f, 60.0f, 5.0f);
    g.fillRoundedRectangle (100.0f, 400.0f, 180.0f, 60.0f, 5.0f);
    g.setColour (juce::Colour (0xff2d333b));
    g.drawRoundedRectangle (100.0f, 115.0f, 180.0f, 60.0f, 5.0f, 1.5f);
    g.drawRoundedRectangle (100.0f, 400.0f, 180.0f, 60.0f, 5.0f, 1.5f);

    drawWaveformPath (g, w1, juce::Rectangle<float> (110.0f, 120.0f, 160.0f, 50.0f), colOsc);
    drawWaveformPath (g, w2, juce::Rectangle<float> (110.0f, 405.0f, 160.0f, 50.0f), colOsc);
}

void CortexiaAudioProcessorEditor::resized()
{
    // TOP RIGHT
    oscilloscope.setBounds    (582, 22, 286, 41);
    masterVolLabel.setBounds  (895, 10, 60, 20);
    masterVolSlider.setBounds (895, 30, 60, 60);

    // OSC 1
    waveSelector1.setBounds (100, 115, 180, 60); // Transparent overlay on screen
    int o1y = 190;
    volumeLabel1.setBounds  (40, o1y, 60, 20); volumeSlider1.setBounds (40, o1y+20, 60, 65);
    tuneLabel1.setBounds    (100, o1y, 60, 20); tuneSlider1.setBounds   (100, o1y+20, 60, 65);
    detuneLabel1.setBounds  (160, o1y, 60, 20); detuneSlider1.setBounds (160, o1y+20, 60, 65);
    unisonLabel1.setBounds  (220, o1y, 60, 20); unisonSlider1.setBounds (220, o1y+20, 60, 65);
    uDetuneLabel1.setBounds (280, o1y, 60, 20); uDetuneSlider1.setBounds(280, o1y+20, 60, 65);
    uBlendLabel1.setBounds  (280, 110, 60, 20); uBlendSlider1.setBounds (280, 130, 60, 65); // Tucked next to wave

    // OSC 2
    waveSelector2.setBounds (100, 400, 180, 60);
    int o2y = 475;
    volumeLabel2.setBounds  (40, o2y, 60, 20); volumeSlider2.setBounds (40, o2y+20, 60, 65);
    tuneLabel2.setBounds    (100, o2y, 60, 20); tuneSlider2.setBounds   (100, o2y+20, 60, 65);
    detuneLabel2.setBounds  (160, o2y, 60, 20); detuneSlider2.setBounds (160, o2y+20, 60, 65);
    unisonLabel2.setBounds  (220, o2y, 60, 20); unisonSlider2.setBounds (220, o2y+20, 60, 65);
    uDetuneLabel2.setBounds (280, o2y, 60, 20); uDetuneSlider2.setBounds(280, o2y+20, 60, 65);
    uBlendLabel2.setBounds  (280, 395, 60, 20); uBlendSlider2.setBounds (280, 415, 60, 65);

    // FILTER
    cutoffLabel.setBounds     (430, 115, 75, 20); cutoffSlider.setBounds    (430, 135, 75, 95);
    resonanceLabel.setBounds  (430, 235, 75, 20); resonanceSlider.setBounds (430, 255, 75, 95);

    // LFO
    lfoTargetLabel.setBounds    (405, 400, 130, 20); lfoTargetSelector.setBounds (405, 420, 130, 25);
    lfoRateLabel.setBounds      (390, 460, 75, 20);  lfoRateSlider.setBounds     (390, 480, 75, 95);
    lfoDepthLabel.setBounds     (475, 460, 75, 20);  lfoDepthSlider.setBounds    (475, 480, 75, 95);

    // ENVELOPE (Centered vertically)
    int envY = 250;
    attackLabel.setBounds    (630, envY, 70, 20); attackSlider.setBounds   (630, envY + 20, 70, 130);
    decayLabel.setBounds     (710, envY, 70, 20); decaySlider.setBounds    (710, envY + 20, 70, 130);
    sustainLabel.setBounds   (790, envY, 70, 20); sustainSlider.setBounds  (790, envY + 20, 70, 130);
    releaseLabel.setBounds   (870, envY, 70, 20); releaseSlider.setBounds  (870, envY + 20, 70, 130);
}