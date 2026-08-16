#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CortexiaAudioProcessor::CortexiaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    synth.addSound (new SynthSound());
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new SynthVoice());
}

CortexiaAudioProcessor::~CortexiaAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CortexiaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // OSC 1 PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC", "Osc 1 Waveform", juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOLUME", "Osc 1 Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE1", "Osc 1 Tune", juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DETUNE1", "Osc 1 Detune", juce::NormalisableRange<float> (-50.0f, 50.0f, 1.0f), 0.0f));

    // OSC 2 PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC2", "Osc 2 Waveform", juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOL2", "Osc 2 Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("TUNE2", "Osc 2 Tune", juce::NormalisableRange<float> (-24.0f, 24.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DETUNE2", "Osc 2 Detune", juce::NormalisableRange<float> (-50.0f, 50.0f, 1.0f), 0.0f));

    // FILTER PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CUTOFF", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RESONANCE", "Resonance", juce::NormalisableRange<float> (0.707f, 10.0f, 0.01f), 0.707f));

    // LFO PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LFO_RATE", "LFO Rate", juce::NormalisableRange<float> (0.1f, 20.0f, 0.1f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("LFO_DEPTH", "LFO Depth", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

    // ENVELOPE PARAMETERS
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float> (0.01f, 3.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float> (0.01f, 3.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.4f));

    return { params.begin(), params.end() };
}

//==============================================================================
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

//==============================================================================
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

#ifndef JucePlugin_PreferredChannelConfigurations
bool CortexiaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif
    return true;
  #endif
}
#endif

void CortexiaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // GET ALL APVTS VALUES
    auto waveType1 = (WaveType) apvts.getRawParameterValue ("OSC")->load();
    auto gain1 = apvts.getRawParameterValue ("VOLUME")->load();
    auto tune1 = apvts.getRawParameterValue ("TUNE1")->load();
    auto detune1 = apvts.getRawParameterValue ("DETUNE1")->load();
    
    auto waveType2 = (WaveType) apvts.getRawParameterValue ("OSC2")->load();
    auto gain2 = apvts.getRawParameterValue ("VOL2")->load();
    auto tune2 = apvts.getRawParameterValue ("TUNE2")->load();
    auto detune2 = apvts.getRawParameterValue ("DETUNE2")->load();

    auto cutoff = apvts.getRawParameterValue ("CUTOFF")->load();
    auto resonance = apvts.getRawParameterValue ("RESONANCE")->load();
    auto lfoRate = apvts.getRawParameterValue ("LFO_RATE")->load();
    auto lfoDepth = apvts.getRawParameterValue ("LFO_DEPTH")->load();
    
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = apvts.getRawParameterValue ("ATTACK")->load();
    adsrParams.decay = apvts.getRawParameterValue ("DECAY")->load();
    adsrParams.sustain = apvts.getRawParameterValue ("SUSTAIN")->load();
    adsrParams.release = apvts.getRawParameterValue ("RELEASE")->load();

    // UPDATE VOICES
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->updateParameters (waveType1, gain1, tune1, detune1, 
                                     waveType2, gain2, tune2, detune2, 
                                     adsrParams, cutoff, resonance, lfoRate, lfoDepth);
    }

    // RENDER AUDIO
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // FEED AUDIO TO VISUALIZER
    if (auto* vis = visualizer.load())
        vis->pushBuffer (buffer);
}

//==============================================================================
bool CortexiaAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* CortexiaAudioProcessor::createEditor() { return new CortexiaAudioProcessorEditor (*this); }

//==============================================================================
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

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CortexiaAudioProcessor();
}