#pragma once

#include "EffectBase.h"
#include "Effects/phaser.h"

class PhaserEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setMix(float mix);

private:
    daisysp::Phaser phaserL_;
    daisysp::Phaser phaserR_;
    float mix_ = 0.5f;
};
