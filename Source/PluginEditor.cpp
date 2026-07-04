#include "PluginProcessor.h"
#include "PluginEditor.h"

AiSerumAudioProcessorEditor::AiSerumAudioProcessorEditor (AiSerumAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      webComponent(juce::WebBrowserComponent::Options()
                   .withBackend(juce::WebBrowserComponent::Options::Backend::webview2))
{
    // 1. Add the browser to the plugin window
    addAndMakeVisible(webComponent);

    // 2. Point it to your React dev server
    webComponent.goToURL("http://localhost:5173");

    // 3. Set the default size of your plugin (800x600 pixels)
    setSize (800, 600);
}

AiSerumAudioProcessorEditor::~AiSerumAudioProcessorEditor() {}

void AiSerumAudioProcessorEditor::paint (juce::Graphics& g)
{
    // We don't draw anything in C++ because React handles the UI!
    g.fillAll (juce::Colours::black);
}

void AiSerumAudioProcessorEditor::resized()
{
    // Make the web browser stretch to fill the entire VST window
    webComponent.setBounds(getLocalBounds());
}