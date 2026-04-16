#pragma once

#include <memory>

namespace daisysp
{
class LadderFilter;
class Svf;
}

class SynthFilter
{
public:
    enum Mode
    {
        Lowpass = 0,
        Highpass,
        Bandpass,
        Notch,
        NumModes
    };

    enum Slope
    {
        Slope12 = 0,
        Slope24
    };

    SynthFilter();
    ~SynthFilter();

    void init(double sampleRate);
    void setMode(Mode mode);
    void setSlope(Slope slope);
    void setCutoff(float freqHz);
    void setResonance(float res);
    float getCutoff() const { return cutoff_; }
    float getResonance() const { return resonance_; }
    void setDrive(float drive);
    void setEnvAmount(float amount);
    void setKeyTracking(float amount);

    float process(float input, float envelopeValue = 0.0f, float noteFreq = 440.0f);

    /** Process with externally modulated cutoff/resonance.
        This avoids the caller having to set/restore base filter values each sample.
    */
    float processWithMod(float input, float envelopeValue, float noteFreq,
                         float modCutoff, float modRes);

private:
    // Map our Mode+Slope to the appropriate LadderFilter mode
    void updateLadderMode();

    double sampleRate_ = 44100.0;
    Mode mode_ = Lowpass;
    Slope slope_ = Slope24;

    float cutoff_ = 1000.0f;
    float resonance_ = 0.0f;
    float drive_ = 1.0f;
    float envAmount_ = 0.0f;
    float keyTracking_ = 0.0f;

    // DaisySP filters (heap-allocated to avoid exposing headers)
    std::unique_ptr<daisysp::LadderFilter> ladder_;
    std::unique_ptr<daisysp::Svf> svf_;
};
