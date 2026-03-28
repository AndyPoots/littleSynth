#include "Distortion.h"
#include <cmath>

void DistortionEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;

    overdriveL_.Init();
    overdriveR_.Init();

    overdriveL_.SetDrive(0.5f);
    overdriveR_.SetDrive(0.5f);

    toneStateL_ = 0.0f;
    toneStateR_ = 0.0f;
}

void DistortionEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    // Tone control: simple one-pole lowpass coefficient
    // tone_ = 0 -> cutoff at low freq (dark), tone_ = 1 -> cutoff at Nyquist (bright)
    float rc = 1.0f - std::exp(-1.0f / (tone_ * 0.01f + 0.001f));
    // Clamp to valid range
    float alpha = std::min(rc, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = left[i];
        float dryR = right[i];

        float wetL = overdriveL_.Process(dryL);
        float wetR = overdriveR_.Process(dryR);

        // Apply tone filter (lowpass)
        toneStateL_ += alpha * (wetL - toneStateL_);
        toneStateR_ += alpha * (wetR - toneStateR_);
        wetL = toneStateL_;
        wetR = toneStateR_;

        left[i] = dryL * (1.0f - mix_) + wetL * mix_;
        right[i] = dryR * (1.0f - mix_) + wetR * mix_;
    }
}

void DistortionEffect::setDrive(float drive)
{
    drive_ = drive;
    overdriveL_.SetDrive(drive);
    overdriveR_.SetDrive(drive);
}

void DistortionEffect::setTone(float tone)
{
    tone_ = tone;
}

void DistortionEffect::setMix(float mix)
{
    mix_ = mix;
}
