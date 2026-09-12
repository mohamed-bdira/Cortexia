#include "Oscillator.h"

namespace
{
    constexpr float kDownsampleKernel[5] = { 0.125f, 0.5f, 0.75f, 0.5f, 0.125f };
    constexpr float kDownsampleNorm = 2.0f; // kernel sums to 2
}

Oscillator::Oscillator()
{
    wavetable = &AnalogWavetableBank::getInstance().get (currentWave);
}

void Oscillator::prepareToPlay (double sampleRate)
{
    currentSampleRate = sampleRate;
    AnalogWavetableBank::getInstance();
}

void Oscillator::resetPhases()
{
    for (int i = 0; i < 7; ++i)
    {
        phases[i] = random.nextDouble() * (double) Wavetable::kTableSize;
        for (int t = 0; t < 5; ++t)
            osHist[i][t] = 0.0f;
    }
}

void Oscillator::setParameters (WaveType type, float gain, float tune, float detune,
                                int uCount, float uDetune, float uBlend, float wtPos)
{
    if (type != currentWave)
    {
        currentWave = type;
        wavetable = &AnalogWavetableBank::getInstance().get (currentWave);
    }

    masterGain = gain;
    oscTune = tune;
    oscDetune = detune;
    unisonCount = juce::jlimit (1, 7, uCount);
    unisonDetune = uDetune;
    unisonBlend = uBlend;
    wtPosition = juce::jlimit (0.0f, 1.0f, wtPos);
}

int Oscillator::pickMip (double freqHz, const Wavetable::Frame& frame) const
{
    const double osRate = currentSampleRate * 2.0;
    const double maxH = 0.45 * osRate / juce::jmax (freqHz, 1.0);

    for (int m = 0; m < Wavetable::kNumMips; ++m)
        if ((double) frame.maxHarmonic[(size_t) m] <= maxH)
            return m;

    return Wavetable::kNumMips - 1;
}

float Oscillator::cubicAt (const std::array<float, Wavetable::kMipStride>& mip, double phase) const
{
    const double wrapped = phase - std::floor (phase / (double) Wavetable::kTableSize) * (double) Wavetable::kTableSize;
    const int i = juce::jlimit (0, Wavetable::kTableSize - 1, (int) wrapped);
    const float frac = (float) (wrapped - (double) i);

    const float y0 = (i == 0) ? mip[Wavetable::kTableSize - 1] : mip[(size_t) (i - 1)];
    const float y1 = mip[(size_t) i];
    const float y2 = mip[(size_t) (i + 1)];
    const float y3 = mip[(size_t) (i + 2)];

    const float c0 = y1;
    const float c1 = 0.5f * (y2 - y0);
    const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

float Oscillator::lookup (double phase, double freqHz) const
{
    if (wavetable == nullptr || wavetable->frames.empty())
        return 0.0f;

    const int n = (int) wavetable->frames.size();
    const float pos = wtPosition * (float) juce::jmax (0, n - 1);
    const int i0 = juce::jlimit (0, n - 1, (int) pos);
    const int i1 = juce::jmin (i0 + 1, n - 1);
    const float frac = pos - (float) i0;

    const auto& f0 = wavetable->frames[(size_t) i0];
    const float a = cubicAt (f0.mip[(size_t) pickMip (freqHz, f0)], phase);

    if (i0 == i1 || frac <= 1.0e-6f)
        return a;

    const auto& f1 = wavetable->frames[(size_t) i1];
    const float b = cubicAt (f1.mip[(size_t) pickMip (freqHz, f1)], phase);
    return a + frac * (b - a);
}

float Oscillator::downsample2x (int voice, float s0, float s1)
{
    auto& h = osHist[voice];
    h[0] = h[1];
    h[1] = h[2];
    h[2] = h[3];
    h[3] = h[4];
    h[4] = s0;

    h[0] = h[1];
    h[1] = h[2];
    h[2] = h[3];
    h[3] = h[4];
    h[4] = s1;

    float acc = 0.0f;
    for (int t = 0; t < 5; ++t)
        acc += h[t] * kDownsampleKernel[t];

    return acc / kDownsampleNorm;
}

void Oscillator::processNextSample (double basePitch, float extraSemitones, float& sampleL, float& sampleR)
{
    sampleL = 0.0f;
    sampleR = 0.0f;

    if (currentSampleRate <= 0.0)
        return;

    const double osRate = currentSampleRate * 2.0;
    auto wrapPhase = [] (double p)
    {
        const double size = (double) Wavetable::kTableSize;
        p = std::fmod (p, size);
        if (p < 0.0) p += size;
        return p;
    };

    for (int v = 0; v < unisonCount; ++v)
    {
        float vDetune = 0.0f;
        float vPan = 0.5f;
        float vGain = (v == 0) ? 1.0f : unisonBlend * (1.0f / std::sqrt ((float) unisonCount - 1.0f));

        if (unisonCount > 1 && v > 0)
        {
            float spread = 1.0f / (unisonCount / 2);
            int pair = (v + 1) / 2;
            float side = (v % 2 == 1) ? -1.0f : 1.0f;
            vPan = 0.5f + (side * 0.5f * (pair * spread));
            vDetune = side * pair * spread * unisonDetune * 50.0f;
        }

        const double pitch = basePitch * std::pow (2.0, (oscTune + extraSemitones + (oscDetune / 100.0) + (vDetune / 100.0)) / 12.0);
        const double phaseInc = (pitch / osRate) * (double) Wavetable::kTableSize;

        const float s0 = lookup (phases[v], pitch);
        phases[v] = wrapPhase (phases[v] + phaseInc);

        const float s1 = lookup (phases[v], pitch);
        phases[v] = wrapPhase (phases[v] + phaseInc);

        const float waveSample = downsample2x (v, s0, s1);
        sampleL += waveSample * masterGain * vGain * (1.0f - vPan);
        sampleR += waveSample * masterGain * vGain * vPan;
    }
}
