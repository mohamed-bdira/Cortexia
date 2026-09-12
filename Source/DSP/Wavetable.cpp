#include "Wavetable.h"

AnalogWavetableBank& AnalogWavetableBank::getInstance()
{
    static AnalogWavetableBank instance;
    return instance;
}

AnalogWavetableBank::AnalogWavetableBank()
{
    fillAnalog (tables[0], WaveType::Sine);
    fillAnalog (tables[1], WaveType::Saw);
    fillAnalog (tables[2], WaveType::Square);
    fillAnalog (tables[3], WaveType::Triangle);
}

const Wavetable& AnalogWavetableBank::get (WaveType type) const
{
    const int index = juce::jlimit (0, 3, static_cast<int> (type));
    return tables[index];
}

void AnalogWavetableBank::fillAnalog (Wavetable& table, WaveType type)
{
    table.frames.resize (1);
    auto& frame = table.frames[0];
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float pi = juce::MathConstants<float>::pi;

    for (int m = 0; m < Wavetable::kNumMips; ++m)
    {
        const int nH = (type == WaveType::Sine) ? 1 : juce::jmax (1, 1024 >> m);
        frame.maxHarmonic[m] = nH;
        auto& mip = frame.mip[(size_t) m];
        mip.fill (0.0f);

        for (int n = 0; n < Wavetable::kTableSize; ++n)
        {
            const float theta = twoPi * (float) n / (float) Wavetable::kTableSize;
            float sample = 0.0f;

            if (type == WaveType::Sine)
            {
                sample = std::sin (theta);
            }
            else
            {
                for (int k = 1; k <= nH; ++k)
                {
                    if ((type == WaveType::Square || type == WaveType::Triangle) && (k % 2) == 0)
                        continue;

                    const float x = pi * (float) k / (float) (nH + 1);
                    const float sigma = (x > 1.0e-8f) ? std::sin (x) / x : 1.0f;
                    const float harm = std::sin ((float) k * theta) * sigma;

                    if (type == WaveType::Triangle)
                        sample += harm / (float) (k * k);
                    else
                        sample += harm / (float) k;
                }
            }

            mip[(size_t) n] = sample;
        }

        float peak = 0.0f;
        for (int n = 0; n < Wavetable::kTableSize; ++n)
            peak = juce::jmax (peak, std::abs (mip[(size_t) n]));

        if (peak > 1.0e-6f)
        {
            const float gain = 0.9f / peak;
            for (int n = 0; n < Wavetable::kTableSize; ++n)
                mip[(size_t) n] *= gain;
        }

        mip[Wavetable::kTableSize]     = mip[0];
        mip[Wavetable::kTableSize + 1] = mip[1];
        mip[Wavetable::kTableSize + 2] = mip[2];
    }
}
