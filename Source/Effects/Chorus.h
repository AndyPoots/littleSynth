#pragma once

#include "EffectBase.h"
#include <memory>

namespace daisysp { class Chorus; }

class ChorusEffect : public EffectBase
{
public:
    ChorusEffect() = default;
    ~ChorusEffect() override;
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setMix(float mix);

private:
    std::unique_ptr<daisysp::Chorus> chorus_;
    float mix_ = 0.5f;
};
