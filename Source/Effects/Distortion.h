#pragma once

#include "EffectBase.h"
#include "Effects/overdrive.h"

class DistortionEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    void setDrive(float drive);
    void setTone(float tone);
    void setMix(float mix);

private:
    daisysp::Overdrive overdriveL_;
    daisysp::Overdrive overdriveR_;
    float drive_ = 0.5f;
    float tone_ = 0.5f;
    float mix_ = 0.5f;

    // Simple one-pole lowpass for tone control
    float toneStateL_ = 0.0f;
    float toneStateR_ = 0.0f;
};
