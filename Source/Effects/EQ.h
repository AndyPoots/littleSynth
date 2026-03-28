#pragma once

#include "EffectBase.h"
#include <juce_dsp/juce_dsp.h>

class EQEffect : public EffectBase
{
public:
    void init(double sampleRate) override;
    void process(float* left, float* right, int numSamples) override;

    // Low shelf band
    void setLowFreq(float freq);
    void setLowGain(float gainDb);
    void setLowQ(float q);

    // Mid peak band
    void setMidFreq(float freq);
    void setMidGain(float gainDb);
    void setMidQ(float q);

    // High shelf band
    void setHighFreq(float freq);
    void setHighGain(float gainDb);
    void setHighQ(float q);

private:
    void updateCoefficients();

    // Per-channel, per-band filters: [channel][band]
    // band 0 = low shelf, band 1 = mid peak, band 2 = high shelf
    juce::dsp::IIR::Filter<float> filtersL_[3];
    juce::dsp::IIR::Filter<float> filtersR_[3];

    float lowFreq_ = 200.0f;
    float lowGainDb_ = 0.0f;
    float lowQ_ = 0.707f;

    float midFreq_ = 1000.0f;
    float midGainDb_ = 0.0f;
    float midQ_ = 1.0f;

    float highFreq_ = 5000.0f;
    float highGainDb_ = 0.0f;
    float highQ_ = 0.707f;
};
