#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay() {}
WaveformDisplay::~WaveformDisplay() {}

void WaveformDisplay::setWaveType (int type)
{
    if (currentWaveType != type)
    {
        currentWaveType = type;
        repaint();
    }
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);
    g.setColour (juce::Colour (0xff38bdf8));
    
    juce::Path p;
    if (currentWaveType == 0) // Sine
    {
        p.startNewSubPath (bounds.getX(), bounds.getCentreY());
        for (float x = 0; x < bounds.getWidth(); x += 1.0f)
            p.lineTo (bounds.getX() + x, bounds.getCentreY() - std::sin (x / bounds.getWidth() * juce::MathConstants<float>::twoPi) * bounds.getHeight() * 0.5f);
    }
    else if (currentWaveType == 1) // Saw
    {
        p.startNewSubPath (bounds.getX(), bounds.getBottom());
        p.lineTo (bounds.getRight(), bounds.getY());
        p.lineTo (bounds.getRight(), bounds.getBottom());
    }
    else if (currentWaveType == 2) // Square
    {
        p.startNewSubPath (bounds.getX(), bounds.getY());
        p.lineTo (bounds.getCentreX(), bounds.getY());
        p.lineTo (bounds.getCentreX(), bounds.getBottom());
        p.lineTo (bounds.getRight(), bounds.getBottom());
    }
    else if (currentWaveType == 3) // Triangle
    {
        p.startNewSubPath (bounds.getX(), bounds.getBottom());
        p.lineTo (bounds.getCentreX(), bounds.getY());
        p.lineTo (bounds.getRight(), bounds.getBottom());
    }
    
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}