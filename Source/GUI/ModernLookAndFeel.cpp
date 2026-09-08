#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe2e8f0));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffffffff));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (0xffffffff));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff171a1f));
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe2e8f0));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff38bdf8).withAlpha(0.2f));
}

void ModernLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, const float rotaryStartAngle,
                                          const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (4.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = 4.0f;
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                 arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff2d333b));
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    auto fillColour = slider.findColour (juce::Slider::rotarySliderFillColourId);
    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour (fillColour);
        g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour (juce::Colour (0xff171a1f));
    g.fillEllipse (bounds.getCentreX() - arcRadius + lineW, bounds.getCentreY() - arcRadius + lineW,
                   (arcRadius - lineW) * 2.0f, (arcRadius - lineW) * 2.0f);

    juce::Path pointer;
    pointer.addRectangle (-1.25f, -radius + lineW + 3.0f, 2.5f, radius * 0.6f);
    pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
    g.setColour (juce::Colour (0xffe2e8f0));
    g.fillPath (pointer);
}