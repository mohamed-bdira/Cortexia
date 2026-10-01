#pragma once
#include <JuceHeader.h>

// Choice indices are saved in host state: append new entries, never reorder.
enum class ModSource : int
{
    None = 0,
    Lfo1,
    Env1,
    Vel,
    ModWheel,
    Keytrack,
    Aftertouch,
    Expression,
    Env2,
    Lfo2,
    Lfo3,
    Macro1,
    Macro2,
    Macro3,
    Macro4
};

enum class ModDest : int
{
    None = 0,
    Cutoff,
    Osc1Pitch,
    Osc2Pitch,
    Osc1Vol,
    Osc2Vol,
    Osc1Wt,
    Osc2Wt,
    Master
};

inline juce::StringArray modSourceNames()
{
    return { "None", "LFO 1", "Env 1", "Vel", "ModWheel", "Keytrack", "Aftertouch", "Expression", "Env 2", "LFO 2", "LFO 3", "Macro 1", "Macro 2", "Macro 3", "Macro 4" };
}

inline juce::StringArray modDestNames()
{
    return { "None", "Cutoff", "Osc1 Pitch", "Osc2 Pitch", "Osc1 Vol", "Osc2 Vol", "Osc1 WT", "Osc2 WT", "Master" };
}

struct ModSlot
{
    ModSource source = ModSource::None;
    ModDest dest = ModDest::None;
    float amount = 0.0f;
    bool bipolar = false;
};

struct ModSlotPack
{
    static constexpr int kNumSlots = 8;
    ModSlot slots[kNumSlots] {};
};
