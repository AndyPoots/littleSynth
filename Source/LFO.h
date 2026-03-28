#pragma once

#include <cstdint>
#include <memory>

namespace daisysp
{
class Oscillator;
}

class SynthLFO
{
public:
    enum Shape
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Square,
        SampleAndHold,
        Random,
        NumShapes
    };

    SynthLFO();
    ~SynthLFO();

    void init(double sampleRate);
    void setShape(Shape shape);
    void setRate(float hz);
    void setDepth(float depth);

    float process();

private:
    double sampleRate_ = 44100.0;
    Shape shape_ = Sine;
    float rate_ = 1.0f;
    float depth_ = 1.0f;

    // DaisySP oscillator for standard LFO shapes
    std::unique_ptr<daisysp::Oscillator> osc_;

    // State for sample-and-hold and random
    float holdValue_ = 0.0f;
    float phase_ = 0.0f;   // running phase for S&H / random trigger
};
