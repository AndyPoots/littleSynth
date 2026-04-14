#pragma once

#include "EffectBase.h"
#include <memory>

namespace daisysp { class Flanger; }

class FlangerEffect : public EffectBase
{
public:
    FlangerEffect() = default;
    ~FlangerEffect() override;
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setMix(float mix);

private:
    std::unique_ptr<daisysp::Flanger> flangerL_;
    std::unique_ptr<daisysp::Flanger> flangerR_;
    float mix_ = 0.5f;
};
