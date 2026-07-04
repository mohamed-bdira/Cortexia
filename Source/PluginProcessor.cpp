#include "PluginProcessor.h"
#include "PluginEditor.h"

AiSerumAudioProcessor::AiSerumAudioProcessor()
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

AiSerumAudioProcessor::~AiSerumAudioProcessor() {}

const juce::String AiSerumAudioProcessor::getName() const { return JucePlugin_Name; }

void AiSerumAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {}

void AiSerumAudioProcessor::releaseResources() {}

void AiSerumAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear(); // Outputs silence for now
}

bool AiSerumAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* AiSerumAudioProcessor::createEditor()
{
    return new AiSerumAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AiSerumAudioProcessor();
}