#include "Envelope.h"

#include "Control/adsr.h"

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

SynthEnvelope::SynthEnvelope()
    : adsr_(std::make_unique<daisysp::Adsr>())
{
}

SynthEnvelope::~SynthEnvelope() = default;

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void SynthEnvelope::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    auto sr = static_cast<float>(sampleRate);

    adsr_->Init(sr);

    // Set reasonable defaults
    adsr_->SetAttackTime(0.01f);
    adsr_->SetDecayTime(0.1f);
    adsr_->SetSustainLevel(0.7f);
    adsr_->SetReleaseTime(0.3f);
}

// ---------------------------------------------------------------
// Parameter setters
// ---------------------------------------------------------------

void SynthEnvelope::setAttack(float seconds)
{
    adsr_->SetAttackTime(std::max(seconds, 0.001f));
}

void SynthEnvelope::setDecay(float seconds)
{
    adsr_->SetDecayTime(std::max(seconds, 0.001f));
}

void SynthEnvelope::setSustain(float level)
{
    adsr_->SetSustainLevel(level);
}

void SynthEnvelope::setRelease(float seconds)
{
    adsr_->SetReleaseTime(std::max(seconds, 0.001f));
}

// ---------------------------------------------------------------
// Gate control
// ---------------------------------------------------------------

void SynthEnvelope::noteOn()
{
    gate_ = true;
}

void SynthEnvelope::noteOff()
{
    gate_ = false;
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

float SynthEnvelope::process()
{
    return adsr_->Process(gate_);
}

// ---------------------------------------------------------------
// State queries
// ---------------------------------------------------------------

bool SynthEnvelope::isActive() const
{
    return adsr_->IsRunning();
}

bool SynthEnvelope::isReleasing() const
{
    return adsr_->GetCurrentSegment() == daisysp::ADSR_SEG_RELEASE;
}

// ---------------------------------------------------------------
// Reset
// ---------------------------------------------------------------

void SynthEnvelope::reset()
{
    gate_ = false;
    adsr_->Retrigger(true);
}
