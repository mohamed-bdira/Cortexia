#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    const juce::Colour colOsc  (0xff2ee6c8);
    const juce::Colour colOscB (0xff38bdf8);
    const juce::Colour colFilt (0xff2ee6c8);
    const juce::Colour colLfo  (0xffe879f9);
    const juce::Colour colEnv  (0xff4ade80);
    const juce::Colour colEnv2 (0xfff59e0b);
    const juce::Colour colMute (0xff7a8599);

    constexpr int kModStripTop = 58;
    constexpr int kModStripH = 130;
    constexpr int kMatrixPanelTop = 198;
    constexpr int kMatrixPanelH = 418;
    constexpr int kMatrixHeaderY = kMatrixPanelTop + 10;
    constexpr int kMatrixRowBaseY = 228;
    constexpr int kMatrixRowStep = 100;
}

CortexiaAudioProcessorEditor::CortexiaAudioProcessorEditor (CortexiaAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&customLookAndFeel);
    audioProcessor.visualizer.store (&oscilloscope);

    oscPageButton.setClickingTogglesState (true);
    matrixPageButton.setClickingTogglesState (true);
    oscPageButton.setRadioGroupId (1);
    matrixPageButton.setRadioGroupId (1);
    oscPageButton.setToggleState (true, juce::dontSendNotification);
    addAndMakeVisible (oscPageButton);
    addAndMakeVisible (matrixPageButton);
    oscPageButton.onClick = [this] { setEditorPage (0); };
    matrixPageButton.onClick = [this] { setEditorPage (1); };

    setupKnob (masterVolSlider, masterVolLabel, "MASTER", colOsc);
    masterVolAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "MASTER_VOL", masterVolSlider);
    for (int i = 0; i < 4; ++i)
    {
        const juce::String label = "M" + juce::String (i + 1);
        setupKnob (macroSliders[i], macroLabels[i], label, colOsc);
        macroAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, "MACRO" + juce::String (i + 1), macroSliders[i]);
    }
    setupKnob (bendRangeSlider, bendRangeLabel, "BEND", colOsc);
    bendRangeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "BEND_RANGE", bendRangeSlider);

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

    setupKnob (lfoRateSlider, lfoRateLabel, "RATE", colLfo);
    setupKnob (lfoDepthSlider, lfoDepthLabel, "DEPTH", colLfo);

    for (int i = 0; i < kNumLfos; ++i)
    {
        auto& b = lfoButtons[i];
        b.setButtonText (juce::String (i + 1));
        b.setClickingTogglesState (true);
        b.setRadioGroupId (3);
        b.setToggleState (i == 0, juce::dontSendNotification);
        b.onClick = [this, i] { setLfoPage (i); };
        addAndMakeVisible (b);
    }

    lfoShapeSelector.addItemList (lfoShapeNames(), 1);
    lfoShapeSelector.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (lfoShapeSelector);

    lfoDivSelector.addItemList (lfoSyncDivisionNames(), 1);
    lfoDivSelector.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (lfoDivSelector);
    lfoDivLabel.setText ("DIV", juce::dontSendNotification);
    lfoDivLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    lfoDivLabel.setJustificationType (juce::Justification::centred);
    lfoDivLabel.setColour (juce::Label::textColourId, colMute);
    addAndMakeVisible (lfoDivLabel);

    lfoRetrigButton.setClickingTogglesState (true);
    lfoSyncButton.setClickingTogglesState (true);
    addAndMakeVisible (lfoRetrigButton);
    addAndMakeVisible (lfoSyncButton);
    lfoSyncButton.onStateChange = [this] { updateLfoRateVisibility(); };

    setupKnob (attackSlider, attackLabel, "ATTACK", colEnv);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ATTACK", attackSlider);
    setupKnob (decaySlider, decayLabel, "DECAY", colEnv);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "DECAY", decaySlider);
    setupKnob (sustainSlider, sustainLabel, "SUSTAIN", colEnv);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "SUSTAIN", sustainSlider);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", colEnv);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "RELEASE", releaseSlider);

    setupKnob (env2AttackSlider, env2AttackLabel, "ATTACK", colEnv2);
    env2AttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ENV2_ATTACK", env2AttackSlider);
    setupKnob (env2DecaySlider, env2DecayLabel, "DECAY", colEnv2);
    env2DecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ENV2_DECAY", env2DecaySlider);
    setupKnob (env2SustainSlider, env2SustainLabel, "SUSTAIN", colEnv2);
    env2SustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ENV2_SUSTAIN", env2SustainSlider);
    setupKnob (env2ReleaseSlider, env2ReleaseLabel, "RELEASE", colEnv2);
    env2ReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "ENV2_RELEASE", env2ReleaseSlider);

    env1Button.setClickingTogglesState (true);
    env2Button.setClickingTogglesState (true);
    env1Button.setRadioGroupId (2);
    env2Button.setRadioGroupId (2);
    env1Button.setToggleState (true, juce::dontSendNotification);
    addAndMakeVisible (env1Button);
    addAndMakeVisible (env2Button);
    env1Button.onClick = [this] { setEnvPage (0); };
    env2Button.onClick = [this] { setEnvPage (1); };

    voiceModeLabel.setText ("MODE", juce::dontSendNotification);
    voiceModeLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    voiceModeLabel.setJustificationType (juce::Justification::centred);
    voiceModeLabel.setColour (juce::Label::textColourId, colMute);
    addAndMakeVisible (voiceModeLabel);
    voiceModeSelector.addItemList ({ "Poly", "Mono", "Legato" }, 1);
    voiceModeSelector.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (voiceModeSelector);
    voiceModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        audioProcessor.apvts, "VOICE_MODE", voiceModeSelector);

    setupKnob (portaSlider, portaLabel, "PORTA", colOsc);
    portaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (audioProcessor.apvts, "PORTA", portaSlider);

    alwaysGlideLabel.setText ("ALWAYS", juce::dontSendNotification);
    alwaysGlideLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    alwaysGlideLabel.setJustificationType (juce::Justification::centred);
    alwaysGlideLabel.setColour (juce::Label::textColourId, colMute);
    addAndMakeVisible (alwaysGlideLabel);
    alwaysGlideButton.setButtonText ({});
    alwaysGlideButton.setClickingTogglesState (true);
    addAndMakeVisible (alwaysGlideButton);
    alwaysGlideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        audioProcessor.apvts, "ALWAYS_GLIDE", alwaysGlideButton);

    setupKnob (modWheelSlider, modWheelLabel, "MW", colOsc);
    modWheelSlider.setRange (0.0, 1.0, 0.01);
    modWheelSlider.setValue (audioProcessor.lastModWheel.load(), juce::dontSendNotification);
    modWheelSlider.onValueChange = [this]
    {
        audioProcessor.setModWheel01 ((float) modWheelSlider.getValue());
    };

    const auto srcNames = modSourceNames();
    const auto dstNames = modDestNames();

    auto setupHeader = [this] (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        label.setJustificationType (juce::Justification::centredLeft);
        label.setColour (juce::Label::textColourId, colMute);
        addAndMakeVisible (label);
    };
    setupHeader (matrixSrcHeader, "SRC");
    setupHeader (matrixDstHeader, "DEST");
    setupHeader (matrixAmtHeader, "AMT");
    setupHeader (matrixBipHeader, "BIP");
    setupHeader (matrixSrcHeader2, "SRC");
    setupHeader (matrixDstHeader2, "DEST");
    setupHeader (matrixAmtHeader2, "AMT");
    setupHeader (matrixBipHeader2, "BIP");

    for (int i = 0; i < 8; ++i)
    {
        auto& row = matrixRows[i];
        const juce::String n = juce::String (i + 1);
        row.src.addItemList (srcNames, 1);
        row.dst.addItemList (dstNames, 1);
        row.src.setLookAndFeel (&customLookAndFeel);
        row.dst.setLookAndFeel (&customLookAndFeel);
        addAndMakeVisible (row.src);
        addAndMakeVisible (row.dst);
        row.srcAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            audioProcessor.apvts, "MTX" + n + "_SRC", row.src);
        row.dstAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
            audioProcessor.apvts, "MTX" + n + "_DST", row.dst);

        setupKnob (row.amt, row.amtLabel, {}, colOsc);
        row.amtLabel.setVisible (false);
        row.amt.setTextBoxStyle (juce::Slider::TextBoxRight, false, 36, 14);
        row.amtAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, "MTX" + n + "_AMT", row.amt);

        row.bip.setButtonText ({});
        row.bip.setClickingTogglesState (true);
        addAndMakeVisible (row.bip);
        row.bipAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            audioProcessor.apvts, "MTX" + n + "_BIP", row.bip);
    }

    waveDisplay1.setAccentColour (colOsc);
    waveDisplay2.setAccentColour (colOscB);
    addAndMakeVisible (waveDisplay1);
    addAndMakeVisible (waveDisplay2);
    addAndMakeVisible (filterDisplay);
    addAndMakeVisible (envelopeGraph);
    addAndMakeVisible (lfoGraph);

    modEnvGraph1.setAccentColour (colEnv);
    modEnvGraph2.setAccentColour (colEnv2);
    addAndMakeVisible (modEnvGraph1);
    addAndMakeVisible (modEnvGraph2);
    for (auto& g : modLfoGraphs)
        addAndMakeVisible (g);

    auto setupModLabel = [this] (juce::Label& label, const juce::String& text)
    {
        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        label.setJustificationType (juce::Justification::centredLeft);
        label.setColour (juce::Label::textColourId, colMute);
        addAndMakeVisible (label);
    };
    setupModLabel (modMacrosHeader, "MACROS");
    setupModLabel (modEnvLabel1, "ENV 1");
    setupModLabel (modEnvLabel2, "ENV 2");
    for (int i = 0; i < kNumLfos; ++i)
        setupModLabel (modLfoLabels[i], "LFO " + juce::String (i + 1));

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

    setLfoPage (0);
    refreshDisplays();
    startTimerHz (20);
    setSize (1280, 720);
    applyEditorPageVisibility();
}

CortexiaAudioProcessorEditor::~CortexiaAudioProcessorEditor()
{
    stopTimer();
    audioProcessor.visualizer.store (nullptr);
    keyboard.setLookAndFeel (nullptr);
    waveSelector1.setLookAndFeel (nullptr);
    waveSelector2.setLookAndFeel (nullptr);
    voiceModeSelector.setLookAndFeel (nullptr);
    lfoShapeSelector.setLookAndFeel (nullptr);
    lfoDivSelector.setLookAndFeel (nullptr);
    for (auto& row : matrixRows)
    {
        row.src.setLookAndFeel (nullptr);
        row.dst.setLookAndFeel (nullptr);
    }
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
    const juce::String envPrefix = envPage == 0 ? "" : "ENV2_";
    envelopeGraph.setAdsr (apvts.getRawParameterValue (envPrefix + "ATTACK")->load(),
                           apvts.getRawParameterValue (envPrefix + "DECAY")->load(),
                           apvts.getRawParameterValue (envPrefix + "SUSTAIN")->load(),
                           apvts.getRawParameterValue (envPrefix + "RELEASE")->load());
    lfoGraph.setDepth (apvts.getRawParameterValue (lfoParamId (lfoPage, "DEPTH"))->load());
    lfoGraph.setShape ((int) apvts.getRawParameterValue (lfoParamId (lfoPage, "SHAPE"))->load());

    modEnvGraph1.setAdsr (apvts.getRawParameterValue ("ATTACK")->load(),
                          apvts.getRawParameterValue ("DECAY")->load(),
                          apvts.getRawParameterValue ("SUSTAIN")->load(),
                          apvts.getRawParameterValue ("RELEASE")->load());
    modEnvGraph2.setAdsr (apvts.getRawParameterValue ("ENV2_ATTACK")->load(),
                          apvts.getRawParameterValue ("ENV2_DECAY")->load(),
                          apvts.getRawParameterValue ("ENV2_SUSTAIN")->load(),
                          apvts.getRawParameterValue ("ENV2_RELEASE")->load());
    for (int i = 0; i < kNumLfos; ++i)
    {
        modLfoGraphs[i].setDepth (apvts.getRawParameterValue (lfoParamId (i, "DEPTH"))->load());
        modLfoGraphs[i].setShape ((int) apvts.getRawParameterValue (lfoParamId (i, "SHAPE"))->load());
    }
}

void CortexiaAudioProcessorEditor::timerCallback()
{
    refreshDisplays();
    if (! modWheelSlider.isMouseButtonDown())
        modWheelSlider.setValue (audioProcessor.lastModWheel.load(), juce::dontSendNotification);
}

void CortexiaAudioProcessorEditor::setEditorPage (int page)
{
    editorPage = page;
    applyEditorPageVisibility();
    resized();
    repaint();
}

void CortexiaAudioProcessorEditor::setEnvPage (int page)
{
    envPage = page;
    envelopeGraph.setAccentColour (envPage == 0 ? colEnv : colEnv2);
    applyEditorPageVisibility();
    refreshDisplays();
    repaint();
}

void CortexiaAudioProcessorEditor::setLfoPage (int page)
{
    lfoPage = juce::jlimit (0, kNumLfos - 1, page);
    auto& apvts = audioProcessor.apvts;
    using Apvts = juce::AudioProcessorValueTreeState;

    lfoRateAttachment.reset();
    lfoDepthAttachment.reset();
    lfoShapeAttachment.reset();
    lfoDivAttachment.reset();
    lfoRetrigAttachment.reset();
    lfoSyncAttachment.reset();

    lfoRateAttachment = std::make_unique<Apvts::SliderAttachment> (apvts, lfoParamId (lfoPage, "RATE"), lfoRateSlider);
    lfoDepthAttachment = std::make_unique<Apvts::SliderAttachment> (apvts, lfoParamId (lfoPage, "DEPTH"), lfoDepthSlider);
    lfoShapeAttachment = std::make_unique<Apvts::ComboBoxAttachment> (apvts, lfoParamId (lfoPage, "SHAPE"), lfoShapeSelector);
    lfoDivAttachment = std::make_unique<Apvts::ComboBoxAttachment> (apvts, lfoParamId (lfoPage, "DIV"), lfoDivSelector);
    lfoRetrigAttachment = std::make_unique<Apvts::ButtonAttachment> (apvts, lfoParamId (lfoPage, "RETRIG"), lfoRetrigButton);
    lfoSyncAttachment = std::make_unique<Apvts::ButtonAttachment> (apvts, lfoParamId (lfoPage, "SYNC"), lfoSyncButton);

    updateLfoRateVisibility();
    refreshDisplays();
    repaint();
}

void CortexiaAudioProcessorEditor::updateLfoRateVisibility()
{
    const bool osc = editorPage == 0;
    const bool sync = lfoSyncButton.getToggleState();
    lfoRateSlider.setVisible (osc && ! sync);
    lfoRateLabel.setVisible (osc && ! sync);
    lfoDivSelector.setVisible (osc && sync);
    lfoDivLabel.setVisible (osc && sync);
}

void CortexiaAudioProcessorEditor::applyEditorPageVisibility()
{
    const bool osc = editorPage == 0;
    const bool mtx = ! osc;

    auto visOsc = [osc] (juce::Component& c) { c.setVisible (osc); };
    visOsc (waveSelector1); visOsc (waveSelector2);
    visOsc (volumeSlider1); visOsc (volumeLabel1);
    visOsc (tuneSlider1); visOsc (tuneLabel1);
    visOsc (detuneSlider1); visOsc (detuneLabel1);
    visOsc (unisonSlider1); visOsc (unisonLabel1);
    visOsc (uDetuneSlider1); visOsc (uDetuneLabel1);
    visOsc (uBlendSlider1); visOsc (uBlendLabel1);
    visOsc (wtPosSlider1); visOsc (wtPosLabel1);
    visOsc (volumeSlider2); visOsc (volumeLabel2);
    visOsc (tuneSlider2); visOsc (tuneLabel2);
    visOsc (detuneSlider2); visOsc (detuneLabel2);
    visOsc (unisonSlider2); visOsc (unisonLabel2);
    visOsc (uDetuneSlider2); visOsc (uDetuneLabel2);
    visOsc (uBlendSlider2); visOsc (uBlendLabel2);
    visOsc (wtPosSlider2); visOsc (wtPosLabel2);
    visOsc (cutoffSlider); visOsc (cutoffLabel);
    visOsc (resonanceSlider); visOsc (resonanceLabel);
    visOsc (lfoDepthSlider); visOsc (lfoDepthLabel);
    visOsc (lfoShapeSelector); visOsc (lfoRetrigButton); visOsc (lfoSyncButton);
    for (auto& b : lfoButtons)
        visOsc (b);
    updateLfoRateVisibility();
    visOsc (env1Button); visOsc (env2Button);

    const bool env1 = osc && envPage == 0;
    const bool env2 = osc && envPage == 1;
    for (auto* c : std::initializer_list<juce::Component*> { &attackSlider, &attackLabel, &decaySlider, &decayLabel,
                                                              &sustainSlider, &sustainLabel, &releaseSlider, &releaseLabel })
        c->setVisible (env1);
    for (auto* c : std::initializer_list<juce::Component*> { &env2AttackSlider, &env2AttackLabel, &env2DecaySlider, &env2DecayLabel,
                                                              &env2SustainSlider, &env2SustainLabel, &env2ReleaseSlider, &env2ReleaseLabel })
        c->setVisible (env2);
    visOsc (waveDisplay1); visOsc (waveDisplay2);
    visOsc (filterDisplay); visOsc (envelopeGraph); visOsc (lfoGraph);

    modEnvGraph1.setVisible (mtx);
    modEnvGraph2.setVisible (mtx);
    modMacrosHeader.setVisible (mtx);
    modEnvLabel1.setVisible (mtx);
    modEnvLabel2.setVisible (mtx);
    for (auto& g : modLfoGraphs)
        g.setVisible (mtx);
    for (auto& l : modLfoLabels)
        l.setVisible (mtx);

    matrixSrcHeader.setVisible (mtx);
    matrixDstHeader.setVisible (mtx);
    matrixAmtHeader.setVisible (mtx);
    matrixBipHeader.setVisible (mtx);
    matrixSrcHeader2.setVisible (mtx);
    matrixDstHeader2.setVisible (mtx);
    matrixAmtHeader2.setVisible (mtx);
    matrixBipHeader2.setVisible (mtx);
    for (auto& row : matrixRows)
    {
        row.src.setVisible (mtx);
        row.dst.setVisible (mtx);
        row.amt.setVisible (mtx);
        row.bip.setVisible (mtx);
    }
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

    if (editorPage == 0)
    {
        g.setColour (colMute);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText ("MACROS", 608, 44, 48, 14, juce::Justification::centredRight);
    }

    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (960.0f, 10.0f, 168.0f, 36.0f, 5.0f, 1.0f);

    if (editorPage == 0)
    {
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
        g.drawText (envPage == 0 ? "ENV 1" : "ENV 2", (int) env.getX() + 14, (int) env.getY() + 10, 60, 18, juce::Justification::left);
        g.drawText ("LFO " + juce::String (lfoPage + 1), (int) lfo.getX() + 14, (int) lfo.getY() + 10, 60, 18, juce::Justification::left);

        drawBadge (g, { oscA.getX() + 78.0f, oscA.getY() + 11.0f, 78.0f, 16.0f }, "WAVETABLE", colOsc);
        drawBadge (g, { oscB.getX() + 78.0f, oscB.getY() + 11.0f, 78.0f, 16.0f }, "WAVETABLE", colOscB);
        drawBadge (g, { filt.getX() + 88.0f, filt.getY() + 11.0f, 42.0f, 16.0f }, "LP", colFilt);
        drawBadge (g, { env.getX() + 66.0f, env.getY() + 11.0f, 42.0f, 16.0f },
                   envPage == 0 ? "AMP" : "MOD", envPage == 0 ? colEnv : colEnv2);
    }
    else
    {
        auto modStrip = juce::Rectangle<float> (12.0f, (float) kModStripTop, 1256.0f, (float) kModStripH);
        auto mtx = juce::Rectangle<float> (12.0f, (float) kMatrixPanelTop, 1256.0f, (float) kMatrixPanelH);
        drawPanel (g, modStrip);
        drawPanel (g, mtx);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xffe8edf4));
        g.drawText ("MODULATORS", (int) modStrip.getX() + 14, (int) modStrip.getY() + 10, 120, 18, juce::Justification::left);
        g.drawText ("MATRIX", (int) mtx.getX() + 14, (int) mtx.getY() + 10, 110, 18, juce::Justification::left);
        drawBadge (g, { modStrip.getX() + 300.0f, modStrip.getY() + 11.0f, 36.0f, 16.0f }, "AMP", colEnv);
        drawBadge (g, { modStrip.getX() + 500.0f, modStrip.getY() + 11.0f, 36.0f, 16.0f }, "MOD", colEnv2);
        drawBadge (g, { mtx.getX() + 88.0f, mtx.getY() + 11.0f, 42.0f, 16.0f }, "8", colOsc);
    }
}

void CortexiaAudioProcessorEditor::resized()
{
    oscPageButton.setBounds (772, 12, 72, 26);
    matrixPageButton.setBounds (848, 12, 56, 26);

    oscilloscope.setBounds (961, 11, 166, 34);
    for (int i = 0; i < 4; ++i)
    {
        if (editorPage == 0)
            layoutKnob (macroSliders[i], macroLabels[i], { 648 + i * 72, 44, 56, 72 });
        else
            layoutKnob (macroSliders[i], macroLabels[i], { 24 + i * 60, 86, 52, 68 });
    }
    layoutKnob (bendRangeSlider, bendRangeLabel, { 1136, 4, 64, 72 });
    layoutKnob (masterVolSlider, masterVolLabel, { 1208, 4, 64, 72 });

    if (editorPage == 0)
    {
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
            layoutKnob (env2AttackSlider,  env2AttackLabel,  a);
            layoutKnob (env2DecaySlider,   env2DecayLabel,   d);
            layoutKnob (env2SustainSlider, env2SustainLabel, s);
            layoutKnob (env2ReleaseSlider, env2ReleaseLabel, r);
        }
        env1Button.setBounds (130, 414, 60, 22);
        env2Button.setBounds (194, 414, 60, 22);

        for (int i = 0; i < kNumLfos; ++i)
            lfoButtons[i].setBounds (724 + i * 32, 414, 28, 22);
        lfoShapeSelector.setBounds (830, 413, 110, 24);
        lfoRetrigButton.setBounds (950, 414, 64, 22);
        lfoSyncButton.setBounds (1020, 414, 56, 22);

        lfoGraph.setBounds (662, 442, 430, 160);
        layoutKnob (lfoRateSlider,  lfoRateLabel,  { 1108, 448, 70, 150 });
        lfoDivLabel.setBounds (1108, 448, 70, 14);
        lfoDivSelector.setBounds (1108, 500, 70, 24);
        layoutKnob (lfoDepthSlider, lfoDepthLabel, { 1188, 448, 70, 150 });
    }
    else
    {
        modMacrosHeader.setBounds (24, kModStripTop + 12, 56, 14);
        modEnvLabel1.setBounds (300, kModStripTop + 12, 48, 14);
        modEnvLabel2.setBounds (500, kModStripTop + 12, 48, 14);
        modEnvGraph1.setBounds (300, 78, 190, 96);
        modEnvGraph2.setBounds (500, 78, 190, 96);
        for (int i = 0; i < kNumLfos; ++i)
        {
            modLfoLabels[i].setBounds (720 + i * 175, kModStripTop + 12, 48, 14);
            modLfoGraphs[i].setBounds (720 + i * 175, 78, 165, 96);
        }

        matrixSrcHeader.setBounds (40, kMatrixHeaderY, 180, 14);
        matrixDstHeader.setBounds (230, kMatrixHeaderY, 200, 14);
        matrixAmtHeader.setBounds (440, kMatrixHeaderY, 100, 14);
        matrixBipHeader.setBounds (550, kMatrixHeaderY, 50, 14);
        matrixSrcHeader2.setBounds (660, kMatrixHeaderY, 180, 14);
        matrixDstHeader2.setBounds (850, kMatrixHeaderY, 200, 14);
        matrixAmtHeader2.setBounds (1060, kMatrixHeaderY, 100, 14);
        matrixBipHeader2.setBounds (1170, kMatrixHeaderY, 50, 14);

        for (int i = 0; i < 8; ++i)
        {
            const int col = i / 4;
            const int row = i % 4;
            const int x = 40 + col * 620;
            const int y = kMatrixRowBaseY + row * kMatrixRowStep;
            auto& r = matrixRows[i];
            r.src.setBounds (x, y + 36, 180, 28);
            r.dst.setBounds (x + 190, y + 36, 200, 28);
            r.amt.setBounds (x + 400, y + 8, 100, 88);
            r.bip.setBounds (x + 510, y + 36, 50, 28);
        }
    }

    voiceModeLabel.setBounds (16, 632, 88, 14);
    voiceModeSelector.setBounds (16, 648, 88, 24);
    layoutKnob (portaSlider, portaLabel, { 112, 628, 64, 80 });
    alwaysGlideLabel.setBounds (180, 632, 56, 14);
    alwaysGlideButton.setBounds (180, 648, 56, 24);

    keyboard.setBounds (252, 628, 944, 80);
    layoutKnob (modWheelSlider, modWheelLabel, { 1208, 628, 60, 80 });
}
