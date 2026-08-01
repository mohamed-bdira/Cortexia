#include "PluginProcessor.h"
#include "PluginEditor.h"

CortexiaAudioProcessor::CortexiaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                        #if ! JucePlugin_IsMidiEffect
                         #if ! JucePlugin_IsSynth
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                         #endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                        #endif
                       )
#endif
{
    // Add 16 polyphonic voices
    for (int i = 0; i < 16; ++i)
        synth.addVoice (new SynthVoice());

    synth.addSound (new SynthSound());
}

CortexiaAudioProcessor::~CortexiaAudioProcessor() {}

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
    buffer.clear();

    // Push latest UI parameters to active voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            voice->updateParameters (currentWaveType, adsrParams, synthGain);
    }

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* CortexiaAudioProcessor::createEditor()
{
    return new CortexiaAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CortexiaAudioProcessor();
}