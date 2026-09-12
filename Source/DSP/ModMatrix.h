#pragma once

enum class ModSource : int
{
    None = 0,
    Lfo1,
    Env1,
    Vel,
    ModWheel,
    Keytrack
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
