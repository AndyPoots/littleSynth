// Source/ModMatrix.h
#pragma once

#include <array>
#include <cstdint>

class ModMatrix
{
public:
    enum class Source
    {
        AmpEnv = 0,
        FilterEnv,
        ModEnv,
        LFO1,
        LFO2,
        Velocity,
        ModWheel,
        Aftertouch,
        PitchBend,
        NumSources
    };

    enum class Destination
    {
        Osc1Pitch = 0,
        Osc2Pitch,
        Osc3Pitch,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        FilterCutoff,
        FilterResonance,
        AmpLevel,
        LFO1Rate,
        LFO1Depth,
        LFO2Rate,
        LFO2Depth,
        Pan,
        NumDestinations
    };

    struct ModSlot
    {
        Source source = Source::AmpEnv;
        Destination dest = Destination::FilterCutoff;
        float depth = 0.0f;
        bool bipolar = true;
        bool active = false;
    };

    static constexpr int kNumSlots = 16;

    ModMatrix();

    void setSource(int slot, Source source);
    void setDestination(int slot, Destination dest);
    void setDepth(int slot, float depth);
    void setBipolar(int slot, bool bipolar);
    void setActive(int slot, bool active);

    // Call this each sample with current source values
    void setSourceValue(Source source, float value);

    // Call this to get the modulated value for a destination.
    // Returns baseValue + accumulated modulation from all active slots targeting this destination.
    float getModulatedValue(Destination dest, float baseValue) const;

private:
    std::array<ModSlot, kNumSlots> slots_;
    std::array<float, static_cast<int>(Source::NumSources)> sourceValues_{};
};
