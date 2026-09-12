#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    const juce::Colour colOsc  (0xff2ee6c8);
    const juce::Colour colOscB (0xff38bdf8);
    const juce::Colour colFilt (0xff2ee6c8);
    const juce::Colour colLfo  (0xffe879f9);
    const juce::Colour colEnv  (0xff4ade80);
    const juce::Colour colMute (0xff7a8599);
}

CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&customLookAndFeel);
    audioProcessor.visualizer.store (&oscilloscope);

    setupKnob (masterVolSlider, masterVolLabel, "MASTER", colOsc);
    masterVolAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "MASTER_VOL", masterVolSlider);

    auto setupWaveCombo = [this] (juce::ComboBox& box)
    {
        box.addItemList ({ "Sine", "Sawtooth", "Square", "Triangle" }, 1);
        box.setLookAndFeel (&customLookAndFeel);
        addAndMakeVisible (box);
    };

    setupWaveCombo (waveSelector1);
    waveSelector1.onChange = [this] { refreshDisplays(); };
    waveAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC", waveSelector1);

    setupKnob (volumeSlider1, volumeLabel1, "VOL", colOsc);
    volumeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOLUME", volumeSlider1);
    setupKnob (tuneSlider1, tuneLabel1, "TUNE", colOsc);
    tuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE1", tuneSlider1);
    setupKnob (detuneSlider1, detuneLabel1, "DETUNE", colOsc);
    detuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE1", detuneSlider1);
    setupKnob (wtPosSlider1, wtPosLabel1, "WT POS", colOsc);
    wtPosAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "WTPOS1", wtPosSlider1);
    setupKnob (unisonSlider1, unisonLabel1, "VOICES", colOsc);
    unisonAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UNISON1", unisonSlider1);
    setupKnob (uDetuneSlider1, uDetuneLabel1, "U.DET", colOsc);
    uDetuneAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UDETUNE1", uDetuneSlider1);
    setupKnob (uBlendSlider1, uBlendLabel1, "U.BLEND", colOsc);
    uBlendAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UBLEND1", uBlendSlider1);

    setupWaveCombo (waveSelector2);
    waveSelector2.onChange = [this] { refreshDisplays(); };
    waveAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "OSC2", waveSelector2);

    setupKnob (volumeSlider2, volumeLabel2, "VOL", colOscB);
    volumeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "VOL2", volumeSlider2);
    setupKnob (tuneSlider2, tuneLabel2, "TUNE", colOscB);
    tuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "TUNE2", tuneSlider2);
    setupKnob (detuneSlider2, detuneLabel2, "DETUNE", colOscB);
    detuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DETUNE2", detuneSlider2);
    setupKnob (wtPosSlider2, wtPosLabel2, "WT POS", colOscB);
    wtPosAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "WTPOS2", wtPosSlider2);
    setupKnob (unisonSlider2, unisonLabel2, "VOICES", colOscB);
    unisonAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UNISON2", unisonSlider2);
    setupKnob (uDetuneSlider2, uDetuneLabel2, "U.DET", colOscB);
    uDetuneAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UDETUNE2", uDetuneSlider2);
    setupKnob (uBlendSlider2, uBlendLabel2, "U.BLEND", colOscB);
    uBlendAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "UBLEND2", uBlendSlider2);

    setupKnob (cutoffSlider, cutoffLabel, "CUTOFF", colFilt);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "CUTOFF", cutoffSlider);
    setupKnob (resonanceSlider, resonanceLabel, "RES", colFilt);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RESONANCE", resonanceSlider);

    lfoTargetSelector.addItemList ({ "None", "Cutoff", "Pitch 1" }, 1);
    lfoTargetSelector.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (lfoTargetSelector);
    lfoTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (audioProcessor.apvts, "LFO_TARGET", lfoTargetSelector);
    lfoTargetLabel.setText ("TARGET", juce::dontSendNotification);
    lfoTargetLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    lfoTargetLabel.setJustificationType (juce::Justification::centred);
    lfoTargetLabel.setColour (juce::Label::textColourId, colMute);
    addAndMakeVisible (lfoTargetLabel);

    setupKnob (lfoRateSlider, lfoRateLabel, "RATE", colLfo);
    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO_RATE", lfoRateSlider);
    setupKnob (lfoDepthSlider, lfoDepthLabel, "DEPTH", colLfo);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "LFO_DEPTH", lfoDepthSlider);

    setupKnob (attackSlider, attackLabel, "ATTACK", colEnv);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ATTACK", attackSlider);
    setupKnob (decaySlider, decayLabel, "DECAY", colEnv);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DECAY", decaySlider);
    setupKnob (sustainSlider, sustainLabel, "SUSTAIN", colEnv);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SUSTAIN", sustainSlider);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", colEnv);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RELEASE", releaseSlider);

    waveDisplay1.setAccentColour (colOsc);
    waveDisplay2.setAccentColour (colOscB);
    addAndMakeVisible (waveDisplay1);
    addAndMakeVisible (waveDisplay2);
    addAndMakeVisible (filterDisplay);
    addAndMakeVisible (envelopeGraph);
    addAndMakeVisible (lfoGraph);

    addAndMakeVisible (oscilloscope);
    oscilloscope.setRepaintRate (30);
    oscilloscope.setBufferSize (256);
    oscilloscope.setColours (juce::Colour (0xff0a0e14), colOsc);

    keyboard.setLookAndFeel (&customLookAndFeel);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffe8edf4));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff121820));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff0a0e14));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, juce::Colour (0xff2ee6c8).withAlpha (0.28f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, juce::Colour (0xff2ee6c8).withAlpha (0.45f));
    keyboard.setAvailableRange (24, 96);
    keyboard.setKeyWidth (14.0f);
    addAndMakeVisible (keyboard);

    refreshDisplays();
    startTimerHz (20);
    setSize (1280, 720);
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor()
{
    stopTimer();
    audioProcessor.visualizer.store (nullptr);
    keyboard.setLookAndFeel (nullptr);
    waveSelector1.setLookAndFeel (nullptr);
    waveSelector2.setLookAndFeel (nullptr);
    lfoTargetSelector.setLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

void CortexiaAudioProcessorEditor::setupKnob (juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour accentColour)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 14);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accentColour);
    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, colMute);
    addAndMakeVisible (label);
}

void CortexiaAudioProcessorEditor::layoutKnob (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
{
    label.setBounds (area.removeFromTop (14));
    slider.setBounds (area);
}

void CortexiaAudioProcessorEditor::refreshDisplays()
{
    auto& apvts = audioProcessor.apvts;
    waveDisplay1.setWaveType ((int) apvts.getRawParameterValue ("OSC")->load());
    waveDisplay2.setWaveType ((int) apvts.getRawParameterValue ("OSC2")->load());
    filterDisplay.setParams (apvts.getRawParameterValue ("CUTOFF")->load(),
                             apvts.getRawParameterValue ("RESONANCE")->load());
    envelopeGraph.setAdsr (apvts.getRawParameterValue ("ATTACK")->load(),
                           apvts.getRawParameterValue ("DECAY")->load(),
                           apvts.getRawParameterValue ("SUSTAIN")->load(),
                           apvts.getRawParameterValue ("RELEASE")->load());
    lfoGraph.setDepth (apvts.getRawParameterValue ("LFO_DEPTH")->load());
}

void CortexiaAudioProcessorEditor::timerCallback()
{
    refreshDisplays();
}

void CortexiaAudioProcessorEditor::drawPanel (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    g.setColour (juce::Colour (0xff121820));
    g.fillRoundedRectangle (bounds, 8.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (bounds, 8.0f, 1.0f);
}

void CortexiaAudioProcessorEditor::drawBadge (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, juce::Colour colour) const
{
    g.setColour (colour.withAlpha (0.12f));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (colour);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText (text, bounds, juce::Justification::centred);
}

void CortexiaAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (juce::Colour (0xff0a0e14), 0.0f, 0.0f,
                             juce::Colour (0xff0d1218), 0.0f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    g.setColour (juce::Colour (0xff7a8599));
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 16, 10, 120, 16, juce::Justification::left);

    g.setColour (juce::Colour (0xffe8edf4));
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("CORTEXIA", 0, 8, getWidth(), 24, juce::Justification::centred);
    g.setColour (colMute);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("SPECTRAL SYNTHESIZER", 0, 30, getWidth(), 16, juce::Justification::centred);

    auto oscA = juce::Rectangle<float> (12.0f, 58.0f, 412.0f, 340.0f);
    auto oscB = juce::Rectangle<float> (432.0f, 58.0f, 412.0f, 340.0f);
    auto filt = juce::Rectangle<float> (852.0f, 58.0f, 416.0f, 340.0f);
    auto env  = juce::Rectangle<float> (12.0f, 406.0f, 630.0f, 210.0f);
    auto lfo  = juce::Rectangle<float> (650.0f, 406.0f, 618.0f, 210.0f);

    drawPanel (g, oscA);
    drawPanel (g, oscB);
    drawPanel (g, filt);
    drawPanel (g, env);
    drawPanel (g, lfo);

    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.setColour (juce::Colour (0xffe8edf4));
    g.drawText ("OSC A", (int) oscA.getX() + 14, (int) oscA.getY() + 10, 70, 18, juce::Justification::left);
    g.drawText ("OSC B", (int) oscB.getX() + 14, (int) oscB.getY() + 10, 70, 18, juce::Justification::left);
    g.drawText ("FILTER", (int) filt.getX() + 14, (int) filt.getY() + 10, 90, 18, juce::Justification::left);
    g.drawText ("ENVELOPE", (int) env.getX() + 14, (int) env.getY() + 10, 110, 18, juce::Justification::left);
    g.drawText ("LFO", (int) lfo.getX() + 14, (int) lfo.getY() + 10, 70, 18, juce::Justification::left);

    drawBadge (g, { oscA.getX() + 78.0f, oscA.getY() + 11.0f, 78.0f, 16.0f }, "WAVETABLE", colOsc);
    drawBadge (g, { oscB.getX() + 78.0f, oscB.getY() + 11.0f, 78.0f, 16.0f }, "WAVETABLE", colOscB);
    drawBadge (g, { filt.getX() + 88.0f, filt.getY() + 11.0f, 42.0f, 16.0f }, "LP", colFilt);

    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (980.0f, 10.0f, 210.0f, 36.0f, 5.0f, 1.0f);
}

void CortexiaAudioProcessorEditor::resized()
{
    oscilloscope.setBounds (981, 11, 208, 34);
    layoutKnob (masterVolSlider, masterVolLabel, { 1200, 4, 64, 72 });

    waveSelector1.setBounds (270, 66, 140, 24);
    waveDisplay1.setBounds (24, 100, 388, 168);
    {
        auto knobs = juce::Rectangle<int> (18, 276, 400, 110);
        const int w = knobs.getWidth() / 7;
        layoutKnob (volumeSlider1,  volumeLabel1,  knobs.removeFromLeft (w));
        layoutKnob (tuneSlider1,    tuneLabel1,    knobs.removeFromLeft (w));
        layoutKnob (detuneSlider1,  detuneLabel1,  knobs.removeFromLeft (w));
        layoutKnob (wtPosSlider1,   wtPosLabel1,   knobs.removeFromLeft (w));
        layoutKnob (unisonSlider1,  unisonLabel1,  knobs.removeFromLeft (w));
        layoutKnob (uDetuneSlider1, uDetuneLabel1, knobs.removeFromLeft (w));
        layoutKnob (uBlendSlider1,  uBlendLabel1,  knobs);
    }

    waveSelector2.setBounds (690, 66, 140, 24);
    waveDisplay2.setBounds (444, 100, 388, 168);
    {
        auto knobs = juce::Rectangle<int> (438, 276, 400, 110);
        const int w = knobs.getWidth() / 7;
        layoutKnob (volumeSlider2,  volumeLabel2,  knobs.removeFromLeft (w));
        layoutKnob (tuneSlider2,    tuneLabel2,    knobs.removeFromLeft (w));
        layoutKnob (detuneSlider2,  detuneLabel2,  knobs.removeFromLeft (w));
        layoutKnob (wtPosSlider2,   wtPosLabel2,   knobs.removeFromLeft (w));
        layoutKnob (unisonSlider2,  unisonLabel2,  knobs.removeFromLeft (w));
        layoutKnob (uDetuneSlider2, uDetuneLabel2, knobs.removeFromLeft (w));
        layoutKnob (uBlendSlider2,  uBlendLabel2,  knobs);
    }

    filterDisplay.setBounds (864, 96, 392, 198);
    layoutKnob (cutoffSlider,    cutoffLabel,    { 940, 302, 90, 88 });
    layoutKnob (resonanceSlider, resonanceLabel, { 1088, 302, 90, 88 });

    envelopeGraph.setBounds (24, 436, 360, 164);
    {
        auto knobs = juce::Rectangle<int> (396, 448, 230, 150);
        const int w = knobs.getWidth() / 2;
        auto col1 = knobs.removeFromLeft (w);
        auto col2 = knobs;
        auto a = col1.removeFromTop (75);
        auto d = col1;
        auto s = col2.removeFromTop (75);
        auto r = col2;
        layoutKnob (attackSlider,  attackLabel,  a);
        layoutKnob (decaySlider,   decayLabel,   d);
        layoutKnob (sustainSlider, sustainLabel, s);
        layoutKnob (releaseSlider, releaseLabel, r);
    }

    lfoGraph.setBounds (662, 436, 330, 164);
    lfoTargetLabel.setBounds (1008, 436, 140, 14);
    lfoTargetSelector.setBounds (1008, 452, 140, 24);
    layoutKnob (lfoRateSlider,  lfoRateLabel,  { 1008, 488, 70, 112 });
    layoutKnob (lfoDepthSlider, lfoDepthLabel, { 1088, 488, 70, 112 });

    keyboard.setBounds (12, 628, 1256, 80);
}
