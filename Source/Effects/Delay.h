#pragma once

#include "EffectBase.h"
#include <juce_dsp/juce_dsp.h>

class DelayEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setTimeMs(float timeMs);
    void setFeedback(float feedback);
    void setMix(float mix);
    void setPingPong(bool pingPong);

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL_;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayR_;

    float timeMs_ = 250.0f;
    float feedback_ = 0.4f;
    float mix_ = 0.3f;
    bool pingPong_ = false;

    // Feedback state
    float feedbackL_ = 0.0f;
    float feedbackR_ = 0.0f;
};
