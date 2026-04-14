#include "Oscillator.h"
#include <daisysp.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

SynthOscillator::SynthOscillator()
    : basicOsc_(std::make_unique<daisysp::Oscillator>())
    , varShapeOsc_(std::make_unique<daisysp::VariableShapeOscillator>())
{
}

SynthOscillator::~SynthOscillator() = default;

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void SynthOscillator::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    auto sr = static_cast<float>(sampleRate);

    basicOsc_->Init(sr);
    varShapeOsc_->Init(sr);

    // Set sensible defaults
    basicOsc_->SetAmp(1.0f);
    basicOsc_->SetWaveform(daisysp::Oscillator::WAVE_SIN);
    varShapeOsc_->SetWaveshape(0.0f); // saw/ramp/tri mode
    varShapeOsc_->SetPW(0.5f);
}

// ---------------------------------------------------------------
// Parameter setters
// ---------------------------------------------------------------

void SynthOscillator::setWaveform(Waveform wf)
{
    waveform_ = wf;
}

void SynthOscillator::setFrequency(float freqHz)
{
    frequency_ = freqHz;
}

void SynthOscillator::setDetune(float cents)
{
    detuneCents_ = cents;
}

void SynthOscillator::setOctave(int octaves)
{
    octaveShift_ = octaves;
}

void SynthOscillator::setLevel(float level)
{
    level_ = level;
}

void SynthOscillator::setPulseWidth(float pw)
{
    pulseWidth_ = std::clamp(pw, 0.01f, 0.99f);
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

float SynthOscillator::process()
{
    const float tunedFreq = applyDetune(frequency_);

    float sample = 0.0f;

    switch (waveform_)
    {
    case Sine:
    {
        basicOsc_->SetFreq(tunedFreq);
        basicOsc_->SetWaveform(daisysp::Oscillator::WAVE_SIN);
        sample = basicOsc_->Process();
        break;
    }
    case Triangle:
    {
        basicOsc_->SetFreq(tunedFreq);
        basicOsc_->SetWaveform(daisysp::Oscillator::WAVE_TRI);
        sample = basicOsc_->Process();
        break;
    }
    case Sawtooth:
    {
        // VariableShapeOscillator: waveshape 0 = saw/ramp/tri family
        // pw controls the mix between saw and triangle.
        // pw=1.0 gives saw, pw=0.0 gives triangle.
        varShapeOsc_->SetFreq(tunedFreq);
        varShapeOsc_->SetWaveshape(0.0f);
        varShapeOsc_->SetPW(1.0f);
        sample = varShapeOsc_->Process();
        break;
    }
    case Square:
    {
        // VariableShapeOscillator: waveshape 1 = square mode
        varShapeOsc_->SetFreq(tunedFreq);
        varShapeOsc_->SetWaveshape(1.0f);
        varShapeOsc_->SetPW(pulseWidth_);
        sample = varShapeOsc_->Process();
        break;
    }
    case Noise:
    {
        // Simple white noise using rand()
        sample = 2.0f * (static_cast<float>(std::rand()) * daisysp::kRandFrac) - 1.0f;
        break;
    }
    default:
        break;
    }

    return sample * level_;
}

// ---------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------

float SynthOscillator::applyDetune(float freq) const
{
    // Apply octave shift: multiply by 2^octaveShift_
    if (octaveShift_ != 0)
    {
        freq *= std::pow(2.0f, static_cast<float>(octaveShift_));
    }

    // Apply detune in cents: multiply by 2^(cents/1200)
    if (detuneCents_ != 0.0f)
    {
        freq *= std::pow(2.0f, detuneCents_ / 1200.0f);
    }

    return freq;
}
