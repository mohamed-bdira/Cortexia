#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffc5cdd8));
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0e141c));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff1c2430));
    setColour (juce::ComboBox::textColourId, juce::Colour (0xffd7dee8));
    setColour (juce::ComboBox::arrowColourId, juce::Colour (0xff7a8599));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff121820));
    setColour (juce::PopupMenu::textColourId, juce::Colour (0xffd7dee8));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff2ee6c8).withAlpha (0.18f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xffffffff));
}

void ModernLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, const float rotaryStartAngle,
                                          const float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (3.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineW = 2.5f;
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(), bounds.getCentreY(),
                                 arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff1c2430));
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

    const float cap = juce::jmax (6.0f, arcRadius - 5.0f);
    g.setColour (juce::Colour (0xff0e141c));
    g.fillEllipse (bounds.getCentreX() - cap, bounds.getCentreY() - cap, cap * 2.0f, cap * 2.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawEllipse (bounds.getCentreX() - cap, bounds.getCentreY() - cap, cap * 2.0f, cap * 2.0f, 1.0f);

    juce::Path pointer;
    pointer.addRoundedRectangle (-1.0f, -cap + 2.0f, 2.0f, cap * 0.48f, 1.0f);
    pointer.applyTransform (juce::AffineTransform::rotation (toAngle).translated (bounds.getCentreX(), bounds.getCentreY()));
    g.setColour (juce::Colour (0xffe8edf4));
    g.fillPath (pointer);
}

void ModernLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                      int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    const float arrowX = (float) width - 14.0f;
    const float arrowY = (float) height * 0.5f;
    juce::Path arrow;
    arrow.addTriangle (arrowX - 4.0f, arrowY - 2.0f, arrowX + 4.0f, arrowY - 2.0f, arrowX, arrowY + 3.5f);
    g.setColour (box.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

void ModernLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (8, 1, box.getWidth() - 24, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

juce::Font ModernLookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font { juce::FontOptions (11.0f) };
}

juce::Font ModernLookAndFeel::getLabelFont (juce::Label&)
{
    return juce::Font { juce::FontOptions (10.0f) };
}
