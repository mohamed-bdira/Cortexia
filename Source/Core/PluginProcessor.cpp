#include "PluginProcessor.h"
#include "PluginEditor.h"

CortexiaAudioProcessor::CortexiaAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    AnalogWavetableBank::getInstance();
    synth.addSound (new SynthSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new SynthVoice());
}

CortexiaAudioProcessor::~CortexiaAudioProcessor() {}

void CortexiaAudioProcessor::setModWheel01 (float value)
{
    lastModWheel.store (juce::jlimit (0.0f, 1.0f, value));
}

juce::AudioProcessorValueTreeState::ParameterLayout CortexiaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // MASTER
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("MASTER_VOL", "Master Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterInt> ("BEND_RANGE", "Bend Range", 0, 24, 2));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("VOICE_MODE", "Voice Mode",
                                                                   juce::StringArray { "Poly", "Mono", "Legato" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("PORTA", "Portamento",
                                                                  juce::NormalisableRange<float> (0.0f, 2.0f, 0.001f, 0.4f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("ALWAYS_GLIDE", "Always Glide", false));

    // OSC 1 PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC", "Osc 1 Waveform", juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOLUME", "Osc 1 Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE1", "Osc 1 Tune", juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DETUNE1", "Osc 1 Detune", juce::NormalisableRange<float> (-50.0f, 50.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterInt>   ("UNISON1", "Osc 1 Unison", 1, 7, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UDETUNE1", "Osc 1 Unison Detune", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UBLEND1", "Osc 1 Unison Blend", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("WTPOS1", "Osc 1 WT Position", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // OSC 2 PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC2", "Osc 2 Waveform", juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOL2", "Osc 2 Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE2", "Osc 2 Tune", juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DETUNE2", "Osc 2 Detune", juce::NormalisableRange<float> (-50.0f, 50.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterInt>   ("UNISON2", "Osc 2 Unison", 1, 7, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UDETUNE2", "Osc 2 Unison Detune", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("UBLEND2", "Osc 2 Unison Blend", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.75f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("WTPOS2", "Osc 2 WT Position", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // FILTER PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CUTOFF", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RESONANCE", "Resonance", juce::NormalisableRange<float> (0.707f, 10.0f, 0.01f), 0.707f));

    // LFO PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LFO_RATE", "LFO Rate", juce::NormalisableRange<float> (0.1f, 20.0f, 0.1f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LFO_DEPTH", "LFO Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("LFO_TARGET", "LFO Target", juce::StringArray { "None", "Cutoff", "Pitch 1" }, 1));

    // ENVELOPE PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float> (0.01f, 3.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float> (0.01f, 3.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.4f));

    const juce::StringArray srcNames { "None", "LFO 1", "Env 1", "Vel", "ModWheel", "Keytrack" };
    const juce::StringArray dstNames { "None", "Cutoff", "Osc1 Pitch", "Osc2 Pitch", "Osc1 Vol", "Osc2 Vol", "Osc1 WT", "Osc2 WT", "Master" };

    for (int i = 1; i <= ModSlotPack::kNumSlots; ++i)
    {
        const juce::String n = juce::String (i);
        const int defaultSrc = (i == 1) ? 1 : 0;
        const int defaultDst = (i == 1) ? 1 : 0;
        params.push_back (std::make_unique<juce::AudioParameterChoice> ("MTX" + n + "_SRC", "Matrix " + n + " Source", srcNames, defaultSrc));
        params.push_back (std::make_unique<juce::AudioParameterChoice> ("MTX" + n + "_DST", "Matrix " + n + " Dest", dstNames, defaultDst));
        params.push_back (std::make_unique<juce::AudioParameterFloat> ("MTX" + n + "_AMT", "Matrix " + n + " Amount",
                                                                      juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterBool> ("MTX" + n + "_BIP", "Matrix " + n + " Bipolar", false));
    }

    return { params.begin(), params.end() };
}

const juce::String CortexiaAudioProcessor::getName() const { return JucePlugin_Name; }
bool CortexiaAudioProcessor::acceptsMidi() const { return true; }
bool CortexiaAudioProcessor::producesMidi() const { return false; }
bool CortexiaAudioProcessor::isMidiEffect() const { return false; }
double CortexiaAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int CortexiaAudioProcessor::getNumPrograms() { return 1; }
int CortexiaAudioProcessor::getCurrentProgram() { return 0; }
void CortexiaAudioProcessor::setCurrentProgram (int index) {}
const juce::String CortexiaAudioProcessor::getProgramName (int index) { return {}; }
void CortexiaAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void CortexiaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
            voice->prepareToPlay (sampleRate, samplesPerBlock);
    }
}

void CortexiaAudioProcessor::releaseResources() {}

bool CortexiaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void CortexiaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isPitchWheel())
            lastPitchWheel = msg.getPitchWheelValue();
        else if (msg.isController())
        {
            const float n = juce::jlimit (0.0f, 1.0f, (float) msg.getControllerValue() / 127.0f);
            if (msg.getControllerNumber() == 1)
                lastModWheel.store (n);
            else if (msg.getControllerNumber() == 11)
                lastExpression.store (n);
        }
    }

    auto masterVol = apvts.getRawParameterValue ("MASTER_VOL")->load();
    auto bendRange = apvts.getRawParameterValue ("BEND_RANGE")->load();
    auto voiceMode = (VoiceMode) (int) apvts.getRawParameterValue ("VOICE_MODE")->load();
    auto portaSec = apvts.getRawParameterValue ("PORTA")->load();
    auto alwaysGlide = apvts.getRawParameterValue ("ALWAYS_GLIDE")->load() > 0.5f;
    synth.setVoicing (voiceMode, portaSec, alwaysGlide);

    auto waveType1 = (WaveType) apvts.getRawParameterValue ("OSC")->load();
    auto gain1 = apvts.getRawParameterValue ("VOLUME")->load();
    auto tune1 = apvts.getRawParameterValue ("TUNE1")->load();
    auto detune1 = apvts.getRawParameterValue ("DETUNE1")->load();
    auto uni1 = (int) apvts.getRawParameterValue ("UNISON1")->load();
    auto uDet1 = apvts.getRawParameterValue ("UDETUNE1")->load();
    auto uBlnd1 = apvts.getRawParameterValue ("UBLEND1")->load();
    auto wtPos1 = apvts.getRawParameterValue ("WTPOS1")->load();
    
    auto waveType2 = (WaveType) apvts.getRawParameterValue ("OSC2")->load();
    auto gain2 = apvts.getRawParameterValue ("VOL2")->load();
    auto tune2 = apvts.getRawParameterValue ("TUNE2")->load();
    auto detune2 = apvts.getRawParameterValue ("DETUNE2")->load();
    auto uni2 = (int) apvts.getRawParameterValue ("UNISON2")->load();
    auto uDet2 = apvts.getRawParameterValue ("UDETUNE2")->load();
    auto uBlnd2 = apvts.getRawParameterValue ("UBLEND2")->load();
    auto wtPos2 = apvts.getRawParameterValue ("WTPOS2")->load();

    auto cutoff = apvts.getRawParameterValue ("CUTOFF")->load();
    auto resonance = apvts.getRawParameterValue ("RESONANCE")->load();
    auto lfoRate = apvts.getRawParameterValue ("LFO_RATE")->load();
    auto lfoDepth = apvts.getRawParameterValue ("LFO_DEPTH")->load();

    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = apvts.getRawParameterValue ("ATTACK")->load();
    adsrParams.decay = apvts.getRawParameterValue ("DECAY")->load();
    adsrParams.sustain = apvts.getRawParameterValue ("SUSTAIN")->load();
    adsrParams.release = apvts.getRawParameterValue ("RELEASE")->load();

    ModSlotPack matrix;
    for (int i = 0; i < ModSlotPack::kNumSlots; ++i)
    {
        const juce::String n = juce::String (i + 1);
        matrix.slots[i].source = (ModSource) (int) apvts.getRawParameterValue ("MTX" + n + "_SRC")->load();
        matrix.slots[i].dest = (ModDest) (int) apvts.getRawParameterValue ("MTX" + n + "_DST")->load();
        matrix.slots[i].amount = apvts.getRawParameterValue ("MTX" + n + "_AMT")->load();
        matrix.slots[i].bipolar = apvts.getRawParameterValue ("MTX" + n + "_BIP")->load() > 0.5f;
    }

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->updateParameters (waveType1, gain1, tune1, detune1, uni1, uDet1, uBlnd1, wtPos1,
                                     waveType2, gain2, tune2, detune2, uni2, uDet2, uBlnd2, wtPos2,
                                     adsrParams, cutoff, resonance, lfoRate, lfoDepth, masterVol,
                                     bendRange, lastPitchWheel, lastModWheel.load(), matrix);
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Mix down for visualizer (mono representation)
    if (auto* vis = visualizer.load())
    {
        juce::AudioBuffer<float> monoBuffer (1, buffer.getNumSamples());
        monoBuffer.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
        if (buffer.getNumChannels() > 1)
            monoBuffer.addFrom (0, 0, buffer, 1, 0, buffer.getNumSamples(), 0.5f);
        vis->pushBuffer (monoBuffer);
    }
}

bool CortexiaAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* CortexiaAudioProcessor::createEditor() { return new CortexiaAudioProcessorEditor (*this); }

void CortexiaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void CortexiaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CortexiaAudioProcessor(); }