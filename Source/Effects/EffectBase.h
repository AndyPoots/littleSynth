#pragma once

class EffectBase
{
public:
    virtual ~EffectBase() = default;

    virtual void init(double sampleRate) = 0;
    virtual void process(float* left, float* right, int numSamples) = 0;

    virtual void setEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool isEnabled() const { return enabled_; }

protected:
    bool enabled_ = false;
    double sampleRate_ = 44100.0;
};
