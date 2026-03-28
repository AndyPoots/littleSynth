#include "EQ.h"
#include <cmath>

void EQEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;

    for (int band = 0; band < 3; ++band)
    {
        filtersL_[band].reset();
        filtersR_[band].reset();
    }

    updateCoefficients();
}

void EQEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        float sampleL = left[i];
        float sampleR = right[i];

        // Process through all three bands
        for (int band = 0; band < 3; ++band)
        {
            sampleL = filtersL_[band].processSample(sampleL);
            sampleR = filtersR_[band].processSample(sampleR);
        }

        left[i] = sampleL;
        right[i] = sampleR;
    }
}

void EQEffect::setLowFreq(float freq)
{
    lowFreq_ = freq;
    updateCoefficients();
}

void EQEffect::setLowGain(float gainDb)
{
    lowGainDb_ = gainDb;
    updateCoefficients();
}

void EQEffect::setLowQ(float q)
{
    lowQ_ = q;
    updateCoefficients();
}

void EQEffect::setMidFreq(float freq)
{
    midFreq_ = freq;
    updateCoefficients();
}

void EQEffect::setMidGain(float gainDb)
{
    midGainDb_ = gainDb;
    updateCoefficients();
}

void EQEffect::setMidQ(float q)
{
    midQ_ = q;
    updateCoefficients();
}

void EQEffect::setHighFreq(float freq)
{
    highFreq_ = freq;
    updateCoefficients();
}

void EQEffect::setHighGain(float gainDb)
{
    highGainDb_ = gainDb;
    updateCoefficients();
}

void EQEffect::setHighQ(float q)
{
    highQ_ = q;
    updateCoefficients();
}

void EQEffect::updateCoefficients()
{
    float sr = static_cast<float>(sampleRate_);

    float lowGain = std::pow(10.0f, lowGainDb_ / 20.0f);
    float midGain = std::pow(10.0f, midGainDb_ / 20.0f);
    float highGain = std::pow(10.0f, highGainDb_ / 20.0f);

    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr, lowFreq_, lowQ_, lowGain);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr, midFreq_, midQ_, midGain);
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr, highFreq_, highQ_, highGain);

    *filtersL_[0].coefficients = *lowCoeffs;
    *filtersR_[0].coefficients = *lowCoeffs;
    *filtersL_[1].coefficients = *midCoeffs;
    *filtersR_[1].coefficients = *midCoeffs;
    *filtersL_[2].coefficients = *highCoeffs;
    *filtersR_[2].coefficients = *highCoeffs;
}
