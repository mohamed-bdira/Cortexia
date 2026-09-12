#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

enum class WaveType { Sine = 0, Saw, Square, Triangle };

struct Wavetable
{
    static constexpr int kTableSize = 2048;
    static constexpr int kNumMips = 8;
    static constexpr int kMipStride = kTableSize + 3; // wrap padding for cubic

    struct Frame
    {
        std::array<std::array<float, kMipStride>, kNumMips> mip {};
        std::array<int, kNumMips> maxHarmonic {};
    };

    std::vector<Frame> frames;
};

class AnalogWavetableBank
{
public:
    static AnalogWavetableBank& getInstance();
    const Wavetable& get (WaveType type) const;

private:
    AnalogWavetableBank();
    void fillAnalog (Wavetable& table, WaveType type);

    Wavetable tables[4];
};
