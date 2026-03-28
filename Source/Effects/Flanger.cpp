#include "Flanger.h"

void FlangerEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    float sr = static_cast<float>(sampleRate);

    flangerL_.Init(sr);
    flangerR_.Init(sr);

    flangerL_.SetLfoFreq(0.5f);
    flangerR_.SetLfoFreq(0.5f);
    flangerL_.SetLfoDepth(0.5f);
    flangerR_.SetLfoDepth(0.5f);
    flangerL_.SetFeedback(0.5f);
    flangerR_.SetFeedback(0.5f);
    flangerL_.SetDelay(0.5f);
    flangerR_.SetDelay(0.5f);
}

void FlangerEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        float wetL = flangerL_.Process(dryL);
        float wetR = flangerR_.Process(dryR);

        left[i] = dryL * (1.0f - mix_) + wetL * mix_;
        right[i] = dryR * (1.0f - mix_) + wetR * mix_;
    }
}

void FlangerEffect::setRate(float rate)
{
    flangerL_.SetLfoFreq(rate);
    flangerR_.SetLfoFreq(rate);
}

void FlangerEffect::setDepth(float depth)
{
    flangerL_.SetLfoDepth(depth);
    flangerR_.SetLfoDepth(depth);
}

void FlangerEffect::setFeedback(float feedback)
{
    flangerL_.SetFeedback(feedback);
    flangerR_.SetFeedback(feedback);
}

void FlangerEffect::setMix(float mix)
{
    mix_ = mix;
}
