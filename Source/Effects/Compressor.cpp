#include "Compressor.h"
#include <cmath>

void CompressorEffect::init(double sampleRate)
{
    sampleRate_ = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 2;

    compressor_.prepare(spec);
    compressor_.setThreshold(0.0f);
    compressor_.setRatio(1.0f);
    compressor_.setAttack(10.0f);
    compressor_.setRelease(100.0f);

    makeupGain_ = 0.0f;
}

void CompressorEffect::process(float* left, float* right, int numSamples)
{
    if (!enabled_)
        return;

    float gainLin = std::pow(10.0f, makeupGain_ / 20.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i];
        float inR = right[i];

        // Use the compressor's sample-by-sample processing
        float outL = compressor_.processSample(0, inL);
        float outR = compressor_.processSample(1, inR);

        left[i] = outL * gainLin;
        right[i] = outR * gainLin;
    }
}

void CompressorEffect::setThreshold(float thresholdDb)
{
    compressor_.setThreshold(thresholdDb);
}

void CompressorEffect::setRatio(float ratio)
{
    compressor_.setRatio(ratio);
}

void CompressorEffect::setAttack(float attackMs)
{
    compressor_.setAttack(attackMs);
}

void CompressorEffect::setRelease(float releaseMs)
{
    compressor_.setRelease(releaseMs);
}

void CompressorEffect::setMakeupGain(float gainDb)
{
    makeupGain_ = gainDb;
}
