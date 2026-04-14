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
    varShapeOsc_->SetSync(false);     // no sync — use slave_frequency_ directly
    varShapeOsc_->SetSyncFreq(440.0f);
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
    freqMultiplierDirty_ = true;
}

void SynthOscillator::setOctave(int octaves)
{
    octaveShift_ = octaves;
    freqMultiplierDirty_ = true;
}

void SynthOscillator::setLevel(float level)
{
    level_ = level;
}

void SynthOscillator::setPulseWidth(float pw)
{
    pulseWidth_ = std::clamp(pw, 0.01f, 0.99f);
}

void SynthOscillator::resetPhase()
{
    basicOsc_->Reset();
    // VariableShapeOscillator has no Reset() — re-init to clear phase
    varShapeOsc_->Init(static_cast<float>(sampleRate_));
    varShapeOsc_->SetSync(false);
    varShapeOsc_->SetSyncFreq(440.0f);
    varShapeOsc_->SetWaveshape(0.0f);
    varShapeOsc_->SetPW(0.5f);
    noisePhase_ = 0.0f;

    // Invalidate caches so next process() re-sends params to DaisySP
    lastSetFreq_ = -1.0f;
    lastSetWaveform_ = static_cast<Waveform>(-1);
    lastSetPW_ = -1.0f;
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

float SynthOscillator::process()
{
    // Recompute multiplier only when detune/octave changed
    if (freqMultiplierDirty_)
        updateFreqMultiplier();

    const float tunedFreq = frequency_ * freqMultiplier_;

    float sample = 0.0f;

    switch (waveform_)
    {
    case Sine:
    {
        if (lastSetFreq_ != tunedFreq)
        {
            basicOsc_->SetFreq(tunedFreq);
            lastSetFreq_ = tunedFreq;
        }
        if (lastSetWaveform_ != Sine)
        {
            basicOsc_->SetWaveform(daisysp::Oscillator::WAVE_SIN);
            lastSetWaveform_ = Sine;
        }
        sample = basicOsc_->Process();
        break;
    }
    case Triangle:
    {
        if (lastSetFreq_ != tunedFreq)
        {
            basicOsc_->SetFreq(tunedFreq);
            lastSetFreq_ = tunedFreq;
        }
        if (lastSetWaveform_ != Triangle)
        {
            basicOsc_->SetWaveform(daisysp::Oscillator::WAVE_TRI);
            lastSetWaveform_ = Triangle;
        }
        sample = basicOsc_->Process();
        break;
    }
    case Sawtooth:
    {
        if (lastSetFreq_ != tunedFreq)
        {
            varShapeOsc_->SetSyncFreq(tunedFreq);
            lastSetFreq_ = tunedFreq;
        }
        if (lastSetWaveform_ != Sawtooth)
        {
            varShapeOsc_->SetWaveshape(0.0f);
            varShapeOsc_->SetPW(1.0f);
            lastSetWaveform_ = Sawtooth;
            lastSetPW_ = 1.0f;
        }
        sample = varShapeOsc_->Process();
        break;
    }
    case Square:
    {
        if (lastSetFreq_ != tunedFreq)
        {
            varShapeOsc_->SetSyncFreq(tunedFreq);
            lastSetFreq_ = tunedFreq;
        }
        if (lastSetWaveform_ != Square)
        {
            varShapeOsc_->SetWaveshape(1.0f);
            lastSetWaveform_ = Square;
        }
        if (lastSetPW_ != pulseWidth_)
        {
            varShapeOsc_->SetPW(pulseWidth_);
            lastSetPW_ = pulseWidth_;
        }
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

void SynthOscillator::updateFreqMultiplier()
{
    float mult = 1.0f;
    if (octaveShift_ != 0)
        mult *= std::pow(2.0f, static_cast<float>(octaveShift_));
    if (detuneCents_ != 0.0f)
        mult *= std::pow(2.0f, detuneCents_ / 1200.0f);
    freqMultiplier_ = mult;
    freqMultiplierDirty_ = false;
}
