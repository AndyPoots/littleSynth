#pragma once

#include "EffectBase.h"
#include <juce_dsp/juce_dsp.h>

class CompressorEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setThreshold(float thresholdDb);
    void setRatio(float ratio);
    void setAttack(float attackMs);
    void setRelease(float releaseMs);
    void setMakeupGain(float gainDb);

private:
    juce::dsp::Compressor<float> compressor_;
    float makeupGain_ = 0.0f;
};
