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
                       ), 
       apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
    for (int i = 0; i < 16; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new SynthSound());
}

CortexiaAudioProcessor::~CortexiaAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CortexiaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC", "Oscillator", juce::StringArray { "Sine", "Sawtooth", "Square", "Triangle" }, 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.1f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float> (0.01f, 5.0f, 0.01f), 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("VOLUME", "Volume", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.2f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("CUTOFF", "Cutoff", juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f), 20000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("RESONANCE", "Resonance", juce::NormalisableRange<float> (0.707f, 10.0f, 0.01f), 0.707f));

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
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->prepareToPlay (sampleRate, samplesPerBlock);
    }
}

void CortexiaAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CortexiaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}
#endif

void CortexiaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // DEFINITIVELY clear the buffer to ensure absolute silence before we render
    buffer.clear();

    auto waveType = (WaveType) apvts.getRawParameterValue ("OSC")->load();
    auto gain = apvts.getRawParameterValue ("VOLUME")->load();
    auto cutoff = apvts.getRawParameterValue ("CUTOFF")->load();
    auto resonance = apvts.getRawParameterValue ("RESONANCE")->load();
    
    juce::ADSR::Parameters adsrParams;
    adsrParams.attack = apvts.getRawParameterValue ("ATTACK")->load();
    adsrParams.decay = apvts.getRawParameterValue ("DECAY")->load();
    adsrParams.sustain = apvts.getRawParameterValue ("SUSTAIN")->load();
    adsrParams.release = apvts.getRawParameterValue ("RELEASE")->load();

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->updateParameters (waveType, adsrParams, gain, cutoff, resonance);
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Push audio to oscilloscope
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        pushNextSampleIntoScope (buffer.getSample (0, sample));
    }
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
void CortexiaAudioProcessor::pushNextSampleIntoScope (float sample)
{
    if (scopePos < scopeData.size())
    {
        scopeData[(size_t) scopePos] = sample;
        scopePos++;
    }
    else
    {
        scopePos = 0;
        nextFrameReady = true;
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CortexiaAudioProcessor();
}