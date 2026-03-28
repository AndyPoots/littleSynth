#pragma once

#include "EffectBase.h"
#include "Effects/flanger.h"

class FlangerEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setFeedback(float feedback);
    void setMix(float mix);

private:
    daisysp::Flanger flangerL_;
    daisysp::Flanger flangerR_;
    float mix_ = 0.5f;
};
