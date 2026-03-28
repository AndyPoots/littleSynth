#pragma once

#include <array>
#include <memory>
#include "Effects/EffectBase.h"
#include "Effects/EQ.h"
#include "Effects/Compressor.h"
#include "Effects/Chorus.h"
#include "Effects/Phaser.h"
#include "Effects/Flanger.h"
#include "Effects/Delay.h"
#include "Effects/Reverb.h"
#include "Effects/Distortion.h"

class EffectsChain
{
public:
    EffectsChain();

    void init(double sampleRate);
    void process(float* left, float* right, int numSamples);

    // Access individual effects for parameter control
    EffectBase* getEffect(int index);
    int getNumEffects() const;

    // Typed accessors for convenient parameter setting
    EQEffect* getEQ();
    CompressorEffect* getCompressor();
    ChorusEffect* getChorus();
    PhaserEffect* getPhaser();
    FlangerEffect* getFlanger();
    DelayEffect* getDelay();
    ReverbEffect* getReverb();
    DistortionEffect* getDistortion();

private:
    // Signal flow: input -> EQ -> Compressor -> Chorus -> Phaser -> Flanger -> Delay -> Reverb -> Distortion -> output
    std::array<std::unique_ptr<EffectBase>, 8> effects_;
};
