#pragma once

#include "EffectBase.h"
#include "Effects/chorus.h"

class ChorusEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRate(float rate);
    void setDepth(float depth);
    void setMix(float mix);

private:
    daisysp::Chorus chorus_;
    float mix_ = 0.5f;
};
