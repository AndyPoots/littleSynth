#pragma once

#include <cstdint>
#include <memory>

namespace daisysp
{
class Oscillator;
class VariableShapeOscillator;
}

class SynthOscillator
{
public:
    enum Waveform
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Square,
        Noise,
        NumWaveforms
    };

    SynthOscillator();
    ~SynthOscillator();

    void init(double sampleRate);
    void setWaveform(Waveform wf);
    void setFrequency(float freqHz);
    void setDetune(float cents);
    void setOctave(int octaves);
    void setLevel(float level);
    void setPulseWidth(float pw);
    void resetPhase();
    float process();

private:
    void updateFreqMultiplier();

    double sampleRate_ = 44100.0;
    Waveform waveform_ = Sine;
    float frequency_ = 440.0f;
    float detuneCents_ = 0.0f;
    int octaveShift_ = 0;
    float level_ = 1.0f;
    float pulseWidth_ = 0.5f;

    // DaisySP oscillators (heap-allocated to avoid exposing DaisySP headers)
    std::unique_ptr<daisysp::Oscillator> basicOsc_;
    std::unique_ptr<daisysp::VariableShapeOscillator> varShapeOsc_;

    // State for noise generation
    float noisePhase_ = 0.0f;
    float noiseFreq_ = 440.0f;

    // Cached last-set values to avoid redundant DaisySP calls
    float lastSetFreq_ = -1.0f;
    Waveform lastSetWaveform_ = static_cast<Waveform>(-1);
    float lastSetPW_ = -1.0f;

    // Pre-computed detune+octave multiplier
    float freqMultiplier_ = 1.0f;
    bool freqMultiplierDirty_ = true;
};
