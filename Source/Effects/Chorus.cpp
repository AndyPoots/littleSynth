#include "Chorus.h"
#include <daisysp.h>

ChorusEffect::~ChorusEffect() = default;

void ChorusEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    chorus_ = std::make_unique<daisysp::Chorus>();
    chorus_->Init(static_cast<float>(sampleRate));
    chorus_->SetLfoDepth(0.5f);
    chorus_->SetLfoFreq(0.5f);
    chorus_->SetDelay(0.5f);
    chorus_->SetFeedback(0.5f);
    chorus_->SetPan(0.25f, 0.75f);
}

void ChorusEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float monoIn = (left[i] + right[i]) * 0.5f;
        chorus_->Process(monoIn);
        float chorusL = chorus_->GetLeft();
        float chorusR = chorus_->GetRight();

        left[i] = left[i] * (1.0f - mix_) + chorusL * mix_;
        right[i] = right[i] * (1.0f - mix_) + chorusR * mix_;
    }
}

void ChorusEffect::setRate(float rate)
{
    chorus_->SetLfoFreq(rate);
}

void ChorusEffect::setDepth(float depth)
{
    chorus_->SetLfoDepth(depth);
}

void ChorusEffect::setMix(float mix)
{
    mix_ = mix;
}
