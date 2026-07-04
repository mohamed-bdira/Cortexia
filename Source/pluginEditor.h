#pragma once 
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AiSerumAudioProcessorEditor   :public juce::AudioProcessorEditor{
    public:
        AiSerumAudioProcessorEditor(AiSerumAudioProcessor&);
        ~AiSerumAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        AiSerumAudioProcessor& audioProcessor;
        juce::WebBrowserComponent webComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiSerumAudioProcessorEditor)
};