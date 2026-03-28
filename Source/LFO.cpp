#include "LFO.h"

#include "Synthesis/oscillator.h"
#include "Utility/dsp.h"

#include <cmath>
#include <cstdlib>

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

SynthLFO::SynthLFO()
    : osc_(std::make_unique<daisysp::Oscillator>())
{
}

SynthLFO::~SynthLFO() = default;

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void SynthLFO::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    auto sr = static_cast<float>(sampleRate);

    osc_->Init(sr);
    osc_->SetAmp(1.0f);
    osc_->SetWaveform(daisysp::Oscillator::WAVE_SIN);
    osc_->SetFreq(rate_);

    phase_ = 0.0f;
    holdValue_ = 0.0f;
}

// ---------------------------------------------------------------
// Parameter setters
// ---------------------------------------------------------------

void SynthLFO::setShape(Shape shape)
{
    shape_ = shape;

    // Update the DaisySP oscillator waveform for standard shapes
    switch (shape_)
    {
    case Sine:
        osc_->SetWaveform(daisysp::Oscillator::WAVE_SIN);
        break;
    case Triangle:
        osc_->SetWaveform(daisysp::Oscillator::WAVE_TRI);
        break;
    case Sawtooth:
        osc_->SetWaveform(daisysp::Oscillator::WAVE_SAW);
        break;
    case Square:
        osc_->SetWaveform(daisysp::Oscillator::WAVE_SQUARE);
        break;
    default:
        // SampleAndHold and Random are handled manually in process()
        break;
    }
}

void SynthLFO::setRate(float hz)
{
    rate_ = std::max(hz, 0.01f);
    osc_->SetFreq(rate_);
}

void SynthLFO::setDepth(float depth)
{
    depth_ = depth;
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

float SynthLFO::process()
{
    float value = 0.0f;

    switch (shape_)
    {
    case Sine:
    case Triangle:
    case Sawtooth:
    case Square:
    {
        value = osc_->Process();
        break;
    }
    case SampleAndHold:
    {
        // Advance a manual phase counter. When phase wraps, grab a new random value.
        float phaseInc = rate_ / static_cast<float>(sampleRate_);
        phase_ += phaseInc;

        if (phase_ >= 1.0f)
        {
            phase_ -= 1.0f;
            // Generate new random value in [-1, 1]
            holdValue_ = 2.0f * (static_cast<float>(std::rand()) * daisysp::kRandFrac) - 1.0f;
        }

        value = holdValue_;
        break;
    }
    case Random:
    {
        // Smooth random: similar to S&H but generates a new value every process call
        // with some smoothing. For simplicity, just generate white noise [-1, 1].
        value = 2.0f * (static_cast<float>(std::rand()) * daisysp::kRandFrac) - 1.0f;
        break;
    }
    default:
        break;
    }

    // Scale to [-depth, +depth] range
    return value * depth_;
}
