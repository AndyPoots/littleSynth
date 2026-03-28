#include "Phaser.h"

void PhaserEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    float sr = static_cast<float>(sampleRate);

    phaserL_.Init(sr);
    phaserR_.Init(sr);

    phaserL_.SetLfoFreq(0.5f);
    phaserR_.SetLfoFreq(0.5f);
    phaserL_.SetLfoDepth(0.5f);
    phaserR_.SetLfoDepth(0.5f);
    phaserL_.SetFeedback(0.5f);
    phaserR_.SetFeedback(0.5f);
    phaserL_.SetFreq(1000.0f);
    phaserR_.SetFreq(1000.0f);
}

void PhaserEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        float wetL = phaserL_.Process(dryL);
        float wetR = phaserR_.Process(dryR);

        left[i] = dryL * (1.0f - mix_) + wetL * mix_;
        right[i] = dryR * (1.0f - mix_) + wetR * mix_;
    }
}

void PhaserEffect::setRate(float rate)
{
    phaserL_.SetLfoFreq(rate);
    phaserR_.SetLfoFreq(rate);
}

void PhaserEffect::setDepth(float depth)
{
    phaserL_.SetLfoDepth(depth);
    phaserR_.SetLfoDepth(depth);
}

void PhaserEffect::setFeedback(float feedback)
{
    phaserL_.SetFeedback(feedback);
    phaserR_.SetFeedback(feedback);
}

void PhaserEffect::setMix(float mix)
{
    mix_ = mix;
}
