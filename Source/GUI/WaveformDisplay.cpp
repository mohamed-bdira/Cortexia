#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
{
    setOpaque (false);
}

void WaveformDisplay::setWaveType (int type)
{
    type = juce::jlimit (0, 3, type);
    if (currentWaveType != type)
    {
        currentWaveType = type;
        repaint();
    }
}

void WaveformDisplay::setAccentColour (juce::Colour colour)
{
    accent = colour;
    repaint();
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0e14));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    const auto& table = AnalogWavetableBank::getInstance().get (static_cast<WaveType> (currentWaveType));
    if (table.frames.empty())
        return;

    const auto& mip = table.frames[0].mip[0];
    auto plot = bounds.reduced (10.0f, 12.0f);

    auto makePath = [&] (float yOffset, float scale)
    {
        juce::Path p;
        const float mid = plot.getCentreY() + yOffset;
        const float amp = plot.getHeight() * 0.38f * scale;
        p.startNewSubPath (plot.getX(), mid - mip[0] * amp);
        const int steps = juce::jmax (2, (int) plot.getWidth());
        for (int i = 1; i <= steps; ++i)
        {
            const float t = (float) i / (float) steps;
            const int idx = juce::jlimit (0, Wavetable::kTableSize - 1,
                                          (int) (t * (float) (Wavetable::kTableSize - 1)));
            p.lineTo (plot.getX() + t * plot.getWidth(), mid - mip[(size_t) idx] * amp);
        }
        return p;
    };

    g.setColour (accent.withAlpha (0.18f));
    g.strokePath (makePath (-10.0f, 0.72f), juce::PathStrokeType (1.4f));
    g.setColour (accent.withAlpha (0.32f));
    g.strokePath (makePath (-5.0f, 0.86f), juce::PathStrokeType (1.6f));

    auto main = makePath (0.0f, 1.0f);
    juce::Path fill = main;
    fill.lineTo (plot.getRight(), plot.getBottom());
    fill.lineTo (plot.getX(), plot.getBottom());
    fill.closeSubPath();
    g.setColour (accent.withAlpha (0.10f));
    g.fillPath (fill);
    g.setColour (accent);
    g.strokePath (main, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void FilterResponseDisplay::setParams (float cutoffHz, float q)
{
    cutoff = cutoffHz;
    resonance = q;
    repaint();
}

void FilterResponseDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0e14));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    auto plot = bounds.reduced (12.0f, 14.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawHorizontalLine ((int) plot.getCentreY(), plot.getX(), plot.getRight());

    const float minF = 20.0f, maxF = 20000.0f;
    const float wc = juce::jmax (20.0f, cutoff);
    const float q = juce::jmax (0.5f, resonance);

    juce::Path curve;
    const int steps = juce::jmax (2, (int) plot.getWidth());
    float peakDb = -80.0f;

    auto magDb = [&] (float freq)
    {
        const float r = freq / wc;
        const float denom = std::sqrt ((1.0f - r * r) * (1.0f - r * r) + (r / q) * (r / q));
        const float mag = 1.0f / juce::jmax (denom, 1.0e-5f);
        return 20.0f * std::log10 (juce::jmax (mag, 1.0e-6f));
    };

    for (int i = 0; i <= steps; ++i)
    {
        const float t = (float) i / (float) steps;
        const float freq = minF * std::pow (maxF / minF, t);
        peakDb = juce::jmax (peakDb, magDb (freq));
    }

    const float dbMin = -24.0f;
    const float dbMax = juce::jmax (6.0f, peakDb + 2.0f);

    auto yForDb = [&] (float db)
    {
        const float n = juce::jlimit (0.0f, 1.0f, (db - dbMin) / (dbMax - dbMin));
        return plot.getBottom() - n * plot.getHeight();
    };

    for (int i = 0; i <= steps; ++i)
    {
        const float t = (float) i / (float) steps;
        const float freq = minF * std::pow (maxF / minF, t);
        const float y = yForDb (magDb (freq));
        const float x = plot.getX() + t * plot.getWidth();
        if (i == 0)
            curve.startNewSubPath (x, y);
        else
            curve.lineTo (x, y);
    }

    juce::Path fill = curve;
    fill.lineTo (plot.getRight(), plot.getBottom());
    fill.lineTo (plot.getX(), plot.getBottom());
    fill.closeSubPath();
    g.setColour (juce::Colour (0xff2ee6c8).withAlpha (0.14f));
    g.fillPath (fill);
    g.setColour (juce::Colour (0xff2ee6c8));
    g.strokePath (curve, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void EnvelopeGraph::setAdsr (float a, float d, float s, float r)
{
    attack = a;
    decay = d;
    sustain = s;
    release = r;
    repaint();
}

void EnvelopeGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0e14));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    auto plot = bounds.reduced (12.0f, 14.0f);
    const float a = juce::jmax (0.02f, attack);
    const float d = juce::jmax (0.02f, decay);
    const float r = juce::jmax (0.02f, release);
    const float hold = 0.35f;
    const float total = a + d + hold + r;
    auto xAt = [&] (float t) { return plot.getX() + (t / total) * plot.getWidth(); };
    auto yAt = [&] (float lvl) { return plot.getBottom() - juce::jlimit (0.0f, 1.0f, lvl) * plot.getHeight(); };

    juce::Path p;
    p.startNewSubPath (xAt (0.0f), yAt (0.0f));
    p.lineTo (xAt (a), yAt (1.0f));
    p.lineTo (xAt (a + d), yAt (sustain));
    p.lineTo (xAt (a + d + hold), yAt (sustain));
    p.lineTo (xAt (total), yAt (0.0f));

    juce::Path fill = p;
    fill.lineTo (plot.getRight(), plot.getBottom());
    fill.lineTo (plot.getX(), plot.getBottom());
    fill.closeSubPath();
    g.setColour (juce::Colour (0xff4ade80).withAlpha (0.14f));
    g.fillPath (fill);
    g.setColour (juce::Colour (0xff4ade80));
    g.strokePath (p, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void LfoGraph::setDepth (float d)
{
    depth = juce::jlimit (0.0f, 1.0f, d);
    repaint();
}

void LfoGraph::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0e14));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colour (0xff1c2430));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    auto plot = bounds.reduced (12.0f, 14.0f);
    const float amp = 0.15f + depth * 0.75f;
    juce::Path p;
    const int steps = juce::jmax (2, (int) plot.getWidth());
    for (int i = 0; i <= steps; ++i)
    {
        const float t = (float) i / (float) steps;
        const float y = plot.getCentreY() - std::sin (t * juce::MathConstants<float>::twoPi) * plot.getHeight() * 0.42f * amp;
        const float x = plot.getX() + t * plot.getWidth();
        if (i == 0)
            p.startNewSubPath (x, y);
        else
            p.lineTo (x, y);
    }

    g.setColour (juce::Colour (0xffe879f9).withAlpha (0.16f));
    juce::Path fill = p;
    fill.lineTo (plot.getRight(), plot.getCentreY());
    fill.lineTo (plot.getX(), plot.getCentreY());
    fill.closeSubPath();
    g.fillPath (fill);
    g.setColour (juce::Colour (0xffe879f9));
    g.strokePath (p, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
