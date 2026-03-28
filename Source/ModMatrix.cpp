// Source/ModMatrix.cpp
#include "ModMatrix.h"

#include <algorithm>
#include <cstddef>

ModMatrix::ModMatrix()
{
    // Initialise all slots to default state
    for (auto& slot : slots_)
    {
        slot.source   = Source::AmpEnv;
        slot.dest     = Destination::FilterCutoff;
        slot.depth    = 0.0f;
        slot.bipolar  = true;
        slot.active   = false;
    }

    // Initialise source values to neutral
    sourceValues_.fill(0.0f);
}

void ModMatrix::setSource(int slot, Source source)
{
    if (slot >= 0 && slot < kNumSlots)
        slots_[static_cast<std::size_t>(slot)].source = source;
}

void ModMatrix::setDestination(int slot, Destination dest)
{
    if (slot >= 0 && slot < kNumSlots)
        slots_[static_cast<std::size_t>(slot)].dest = dest;
}

void ModMatrix::setDepth(int slot, float depth)
{
    if (slot >= 0 && slot < kNumSlots)
        slots_[static_cast<std::size_t>(slot)].depth = depth;
}

void ModMatrix::setBipolar(int slot, bool bipolar)
{
    if (slot >= 0 && slot < kNumSlots)
        slots_[static_cast<std::size_t>(slot)].bipolar = bipolar;
}

void ModMatrix::setActive(int slot, bool active)
{
    if (slot >= 0 && slot < kNumSlots)
        slots_[static_cast<std::size_t>(slot)].active = active;
}

void ModMatrix::setSourceValue(Source source, float value)
{
    const int idx = static_cast<int>(source);
    if (idx >= 0 && idx < static_cast<int>(Source::NumSources))
        sourceValues_[static_cast<std::size_t>(idx)] = value;
}

float ModMatrix::getModulatedValue(Destination dest, float baseValue) const
{
    float modulation = 0.0f;

    for (int i = 0; i < kNumSlots; ++i)
    {
        const auto& slot = slots_[static_cast<std::size_t>(i)];

        // Skip inactive slots or slots targeting a different destination
        if (!slot.active || slot.dest != dest)
            continue;

        const int srcIdx = static_cast<int>(slot.source);
        float srcValue = sourceValues_[static_cast<std::size_t>(srcIdx)];

        if (slot.bipolar)
        {
            // Bipolar: source in [-1, 1], scale by depth
            modulation += srcValue * slot.depth;
        }
        else
        {
            // Unipolar: source in [0, 1], scale by depth
            // Remap from [-1,1] to [0,1] if the source is naturally bipolar
            float uni = 0.5f * srcValue + 0.5f;
            modulation += uni * slot.depth;
        }
    }

    return baseValue + modulation;
}
