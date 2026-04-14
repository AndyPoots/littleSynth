#include "EffectsChain.h"
#include <daisysp.h>

EffectsChain::EffectsChain()
{
    // Signal flow order:
    // 0: EQ
    // 1: Compressor
    // 2: Chorus
    // 3: Phaser
    // 4: Flanger
    // 5: Delay
    // 6: Reverb
    // 7: Distortion
    effects_[0] = std::make_unique<EQEffect>();
    effects_[1] = std::make_unique<CompressorEffect>();
    effects_[2] = std::make_unique<ChorusEffect>();
    effects_[3] = std::make_unique<PhaserEffect>();
    effects_[4] = std::make_unique<FlangerEffect>();
    effects_[5] = std::make_unique<DelayEffect>();
    effects_[6] = std::make_unique<ReverbEffect>();
    effects_[7] = std::make_unique<DistortionEffect>();
}

void EffectsChain::init(double sampleRate)
{
    for (auto& effect : effects_)
    {
        effect->init(sampleRate);
    }
}

void EffectsChain::process(float* left, float* right, int numSamples)
{
    for (auto& effect : effects_)
    {
        if (effect->isEnabled())
        {
            effect->process(left, right, numSamples);
        }
    }
}

EffectBase* EffectsChain::getEffect(int index)
{
    if (index >= 0 && index < static_cast<int>(effects_.size()))
        return effects_[static_cast<size_t>(index)].get();
    return nullptr;
}

int EffectsChain::getNumEffects() const
{
    return static_cast<int>(effects_.size());
}

EQEffect* EffectsChain::getEQ()
{
    return static_cast<EQEffect*>(effects_[0].get());
}

CompressorEffect* EffectsChain::getCompressor()
{
    return static_cast<CompressorEffect*>(effects_[1].get());
}

ChorusEffect* EffectsChain::getChorus()
{
    return static_cast<ChorusEffect*>(effects_[2].get());
}

PhaserEffect* EffectsChain::getPhaser()
{
    return static_cast<PhaserEffect*>(effects_[3].get());
}

FlangerEffect* EffectsChain::getFlanger()
{
    return static_cast<FlangerEffect*>(effects_[4].get());
}

DelayEffect* EffectsChain::getDelay()
{
    return static_cast<DelayEffect*>(effects_[5].get());
}

ReverbEffect* EffectsChain::getReverb()
{
    return static_cast<ReverbEffect*>(effects_[6].get());
}

DistortionEffect* EffectsChain::getDistortion()
{
    return static_cast<DistortionEffect*>(effects_[7].get());
}
