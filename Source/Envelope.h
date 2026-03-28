#pragma once

#include <memory>

namespace daisysp
{
class Adsr;
}

class SynthEnvelope
{
public:
    SynthEnvelope();
    ~SynthEnvelope();

    void init(double sampleRate);

    void setAttack(float seconds);
    void setDecay(float seconds);
    void setSustain(float level);
    void setRelease(float seconds);

    void noteOn();
    void noteOff();

    float process();

    bool isActive() const;
    bool isReleasing() const;

    void reset();

private:
    double sampleRate_ = 44100.0;
    bool gate_ = false;

    // DaisySP ADSR (heap-allocated to avoid exposing headers)
    std::unique_ptr<daisysp::Adsr> adsr_;
};
