// Source/GUI/Visualizer.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <array>

class Visualizer : public juce::Component, public juce::Timer
{
public:
    Visualizer();
    ~Visualizer() override;

    /// Call from audio thread to push a mono sample
    void pushSample(float sample);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    // Circular buffer for oscilloscope
    static constexpr int kOscBufferSize = 2048;
    std::array<float, kOscBufferSize> oscBuffer_{};
    std::atomic<int> writePos_{0};

    // FFT
    static constexpr int kFFTOrder = 10;
    static constexpr int kFFTSize = 1 << kFFTOrder; // 1024
    juce::dsp::FFT fft_{ kFFTOrder };
    std::array<float, kFFTSize * 2> fftData_{}; // complex interleaved
    std::array<float, kFFTSize> fftMagnitudes_{};
    std::atomic<int> fftWritePos_{0};
    std::atomic<bool> fftReady_{false};

    // Display buffers (written from timer, read from paint)
    std::array<float, kOscBufferSize> displayBuffer_{};
    std::array<float, kFFTSize / 2> displaySpectrum_{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Visualizer)
};
