#include "Filter.h"
#include <daisysp.h>

#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

SynthFilter::SynthFilter()
    : ladder_(std::make_unique<daisysp::LadderFilter>())
    , svf_(std::make_unique<daisysp::Svf>())
{
}

SynthFilter::~SynthFilter() = default;

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void SynthFilter::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    auto sr = static_cast<float>(sampleRate);

    ladder_->Init(sr);
    svf_->Init(sr);

    ladder_->SetFreq(cutoff_);
    ladder_->SetRes(resonance_);
    ladder_->SetInputDrive(drive_);

    svf_->SetFreq(cutoff_);
    svf_->SetRes(resonance_);
    svf_->SetDrive(drive_);
}

// ---------------------------------------------------------------
// Parameter setters
// ---------------------------------------------------------------

void SynthFilter::setMode(Mode mode)
{
    mode_ = mode;
    updateLadderMode();
}

void SynthFilter::setSlope(Slope slope)
{
    slope_ = slope;
    updateLadderMode();
}

void SynthFilter::setCutoff(float freqHz)
{
    cutoff_ = std::clamp(freqHz, 10.0f, static_cast<float>(sampleRate_) * 0.49f);
}

void SynthFilter::setResonance(float res)
{
    resonance_ = std::clamp(res, 0.0f, 1.0f);
}

void SynthFilter::setDrive(float drive)
{
    drive_ = drive;
}

void SynthFilter::setEnvAmount(float amount)
{
    envAmount_ = amount;
}

void SynthFilter::setKeyTracking(float amount)
{
    keyTracking_ = std::clamp(amount, 0.0f, 1.0f);
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

float SynthFilter::process(float input, float envelopeValue, float noteFreq)
{
    // Compute effective cutoff with envelope modulation and key tracking
    float effectiveCutoff = cutoff_;

    // Envelope modulation: envAmount * envelopeValue (0..1) scaled to octaves
    if (envAmount_ != 0.0f)
    {
        float envMod = envelopeValue * envAmount_;
        // Scale env amount as a multiplier in octaves (positive = brighter)
        effectiveCutoff *= std::pow(2.0f, envMod * 4.0f);
    }

    // Key tracking: blend between fixed cutoff and note frequency
    if (keyTracking_ != 0.0f)
    {
        effectiveCutoff = effectiveCutoff * (1.0f - keyTracking_)
                        + noteFreq * keyTracking_;
    }

    // Clamp final cutoff to valid range
    effectiveCutoff = std::clamp(effectiveCutoff, 10.0f, static_cast<float>(sampleRate_) * 0.49f);

    // Use the Ladder filter for Lowpass / Highpass (it supports LP/HP/BP in 12/24 dB)
    if (mode_ == Lowpass || mode_ == Highpass || mode_ == Bandpass)
    {
        ladder_->SetFreq(effectiveCutoff);
        ladder_->SetRes(resonance_);
        ladder_->SetInputDrive(drive_);
        return ladder_->Process(input);
    }

    // Use SVF for Notch (ladder doesn't have a notch mode)
    svf_->SetFreq(effectiveCutoff);
    svf_->SetRes(resonance_);
    svf_->SetDrive(drive_);
    svf_->Process(input);

    switch (mode_)
    {
    case Notch:
        return svf_->Notch();
    default:
        return svf_->Low();
    }
}

// ---------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------

void SynthFilter::updateLadderMode()
{
    using FM = daisysp::LadderFilter::FilterMode;

    FM targetMode = FM::LP24; // default

    switch (mode_)
    {
    case Lowpass:
        targetMode = (slope_ == Slope12) ? FM::LP12 : FM::LP24;
        break;
    case Highpass:
        targetMode = (slope_ == Slope12) ? FM::HP12 : FM::HP24;
        break;
    case Bandpass:
        targetMode = (slope_ == Slope12) ? FM::BP12 : FM::BP24;
        break;
    default:
        // Notch uses SVF, not ladder; keep whatever is set
        return;
    }

    ladder_->SetFilterMode(targetMode);
}
