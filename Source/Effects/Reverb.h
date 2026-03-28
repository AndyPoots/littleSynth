#pragma once

#include "EffectBase.h"
#include <juce_audio_basics/juce_audio_basics.h>

class ReverbEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setRoomSize(float roomSize);
    void setDamping(float damping);
    void setWetDry(float wetDry);
    void setPreDelay(float preDelayMs);

private:
    juce::Reverb reverb_;
    float preDelayMs_ = 0.0f;

    // Simple pre-delay using a circular buffer
    std::vector<float> preDelayBufferL_;
    std::vector<float> preDelayBufferR_;
    size_t preDelayWritePos_ = 0;
    size_t preDelayReadPos_ = 0;
    size_t preDelaySize_ = 0;
};
