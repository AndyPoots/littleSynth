#pragma once

#include "EffectBase.h"
#include <memory>

namespace daisysp { class Phaser; }

class PhaserEffect : public EffectBase
{
public:
    PhaserEffect() = default;
    ~PhaserEffect() override;
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setMix(float mix);

private:
    std::unique_ptr<daisysp::Phaser> phaserL_;
    std::unique_ptr<daisysp::Phaser> phaserR_;
    float mix_ = 0.5f;
};
