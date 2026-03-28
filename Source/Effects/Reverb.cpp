#include "Reverb.h"

void ReverbEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    reverb_.setSampleRate(sampleRate);

    juce::Reverb::Parameters params;
    params.roomSize = 0.5f;
    params.damping = 0.5f;
    params.wetLevel = 0.33f;
    params.dryLevel = 0.4f;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    reverb_.setParameters(params);

    // Pre-delay buffer: max 200ms
    preDelaySize_ = static_cast<size_t>(sampleRate * 0.2) + 1;
    preDelayBufferL_.resize(preDelaySize_, 0.0f);
    preDelayBufferR_.resize(preDelaySize_, 0.0f);
    preDelayWritePos_ = 0;
    preDelayReadPos_ = 0;
    setPreDelay(0.0f);
}

void ReverbEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    // Apply pre-delay via circular buffer
    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right[i];

        preDelayBufferL_[preDelayWritePos_] = inL;
        preDelayBufferR_[preDelayWritePos_] = inR;

        left[i] = preDelayBufferL_[preDelayReadPos_];
        right[i] = preDelayBufferR_[preDelayReadPos_];

        preDelayWritePos_ = (preDelayWritePos_ + 1) % preDelaySize_;
        preDelayReadPos_ = (preDelayReadPos_ + 1) % preDelaySize_;
    }

    reverb_.processStereo(left, right, numSamples);
}

void ReverbEffect::setRoomSize(float roomSize)
{
    auto params = reverb_.getParameters();
    params.roomSize = roomSize;
    reverb_.setParameters(params);
}

void ReverbEffect::setDamping(float damping)
{
    auto params = reverb_.getParameters();
    params.damping = damping;
    reverb_.setParameters(params);
}

void ReverbEffect::setWetDry(float wetDry)
{
    auto params = reverb_.getParameters();
    params.wetLevel = wetDry;
    params.dryLevel = 1.0f - wetDry;
    reverb_.setParameters(params);
}

void ReverbEffect::setPreDelay(float preDelayMs)
{
    preDelayMs_ = preDelayMs;
    size_t delaySamples = static_cast<size_t>(preDelayMs_ * static_cast<float>(sampleRate_) / 1000.0f);
    delaySamples = std::min(delaySamples, preDelaySize_ > 0 ? preDelaySize_ - 1 : size_t(0));

    // Read position trails write position by delaySamples
    preDelayReadPos_ = (preDelayWritePos_ + preDelaySize_ - delaySamples) % preDelaySize_;
}
