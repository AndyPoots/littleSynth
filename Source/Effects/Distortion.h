#pragma once

#include "EffectBase.h"
#include <memory>

namespace daisysp { class Overdrive; }

class DistortionEffect : public EffectBase
{
public:
    DistortionEffect() = default;
    ~DistortionEffect() override;
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setDrive(float drive);
    void setTone(float tone);
    void setMix(float mix);

private:
    std::unique_ptr<daisysp::Overdrive> overdriveL_;
    std::unique_ptr<daisysp::Overdrive> overdriveR_;
    float drive_ = 0.5f;
    float tone_ = 0.5f;
    float mix_ = 0.5f;

    // Simple one-pole lowpass for tone control
    float toneStateL_ = 0.0f;
    float toneStateR_ = 0.0f;
};
