#include "Delay.h"

void DelayEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    // Max delay of 2 seconds
    int maxDelaySamples = static_cast<int>(sampleRate * 2.0);

    delayL_.setMaximumDelayInSamples(maxDelaySamples);
    delayR_.setMaximumDelayInSamples(maxDelaySamples);
    delayL_.prepare(spec);
    delayR_.prepare(spec);

    int delaySamples = static_cast<int>(timeMs_ * static_cast<float>(sampleRate) / 1000.0f);
    delayL_.setDelay(static_cast<float>(delaySamples));
    delayR_.setDelay(static_cast<float>(delaySamples));

    feedbackL_ = 0.0f;
    feedbackR_ = 0.0f;
}

void DelayEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    int delaySamples = static_cast<int>(timeMs_ * static_cast<float>(sampleRate_) / 1000.0f);
    delayL_.setDelay(static_cast<float>(delaySamples));
    delayR_.setDelay(static_cast<float>(delaySamples));

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        delayL_.pushSample(0, dryL + feedbackL_);
        delayR_.pushSample(0, dryR + feedbackR_);

        float wetL = delayL_.popSample(0);
        float wetR = delayR_.popSample(0);

        if (pingPong_)
        {
            // Swap feedback channels for ping-pong
            feedbackL_ = wetR * feedback_;
            feedbackR_ = wetL * feedback_;
        }
        else
        {
            feedbackL_ = wetL * feedback_;
            feedbackR_ = wetR * feedback_;
        }

        left[i] = dryL + wetL * mix_;
        right[i] = dryR + wetR * mix_;
    }
}

void DelayEffect::setTimeMs(float timeMs)
{
    timeMs_ = timeMs;
}

void DelayEffect::setFeedback(float feedback)
{
    feedback_ = feedback;
}

void DelayEffect::setMix(float mix)
{
    mix_ = mix;
}

void DelayEffect::setPingPong(bool pingPong)
{
    pingPong_ = pingPong;
}
