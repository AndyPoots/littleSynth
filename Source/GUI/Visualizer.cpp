// Source/GUI/Visualizer.cpp
#include "Visualizer.h"
#include "CustomLookAndFeel.h"
#include <cmath>
#include <algorithm>

Visualizer::Visualizer()
{
    oscBuffer_.fill(0.0f);
    fftData_.fill(0.0f);
    fftMagnitudes_.fill(0.0f);
    displayBuffer_.fill(0.0f);
    displaySpectrum_.fill(0.0f);
    startTimerHz(30);
}

Visualizer::~Visualizer()
{
    stopTimer();
}

void Visualizer::pushSample(float sample)
{
    const int pos = writePos_.load(std::memory_order_relaxed);
    oscBuffer_[static_cast<size_t>(pos) % kOscBufferSize] = sample;

    // Also push to FFT buffer
    const int fftPos = fftWritePos_.load(std::memory_order_relaxed);
    fftData_[static_cast<size_t>(fftPos)] = sample;
    int nextPos = (fftPos + 1) % kFFTSize;
    fftWritePos_.store(nextPos, std::memory_order_relaxed);

    if (nextPos == 0)
    {
        fftReady_.store(true, std::memory_order_relaxed);
    }

    int nextOsc = (pos + 1) % kOscBufferSize;
    writePos_.store(nextOsc, std::memory_order_relaxed);
}

void Visualizer::timerCallback()
{
    // Copy oscilloscope data for display
    const int wp = writePos_.load(std::memory_order_acquire);
    for (int i = 0; i < kOscBufferSize; ++i)
    {
        int idx = (wp - kOscBufferSize + i + kOscBufferSize) % kOscBufferSize;
        displayBuffer_[static_cast<size_t>(i)] = oscBuffer_[static_cast<size_t>(idx)];
    }

    // Perform FFT if enough data
    if (fftReady_.load(std::memory_order_acquire))
    {
        fftReady_.store(false, std::memory_order_release);

        // Copy to temporary and perform FFT
        std::array<float, kFFTSize * 2> fftInput{};
        const int fftWp = fftWritePos_.load(std::memory_order_acquire);
        for (int i = 0; i < kFFTSize; ++i)
        {
            int idx = (fftWp + i) % kFFTSize;
            fftInput[static_cast<size_t>(i)] = fftData_[static_cast<size_t>(idx)];
        }

        // Apply Hann window
        for (int i = 0; i < kFFTSize; ++i)
        {
            const float window = 0.5f * (1.0f - std::cos(2.0f * 3.14159265f * (float)i / (float)(kFFTSize - 1)));
            fftInput[static_cast<size_t>(i)] *= window;
        }

        fft_.performFrequencyOnlyForwardTransform(fftInput.data());

        // Convert to magnitudes (dB-ish)
        for (int i = 0; i < kFFTSize / 2; ++i)
        {
            const float mag = fftInput[static_cast<size_t>(i)];
            // Simplified dB conversion, clamped
            const float db = 20.0f * std::log10(mag + 1e-6f);
            displaySpectrum_[static_cast<size_t>(i)] = juce::jlimit(0.0f, 1.0f,
                (db + 60.0f) / 60.0f); // 60dB range, normalized 0..1
        }
    }

    repaint();
}

void Visualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);

    const float halfW = bounds.getWidth() * 0.5f;
    const float padding = 4.0f;
    const float graphH = bounds.getHeight() - padding * 2.0f;

    // --- Oscilloscope (left half) ---
    {
        auto oscBounds = juce::Rectangle<float>(padding, padding, halfW - padding * 2.0f, graphH);

        // Grid lines
        g.setColour(juce::Colour(CustomLookAndFeel::kTrackBg));
        g.drawHorizontalLine((int)(oscBounds.getCentreY()), oscBounds.getX(), oscBounds.getRight());
        for (int i = 1; i < 4; ++i)
        {
            float x = oscBounds.getX() + oscBounds.getWidth() * (float)i / 4.0f;
            g.drawVerticalLine((int)x, oscBounds.getY(), oscBounds.getBottom());
        }

        // Waveform
        const int numSamples = juce::jmin(512, kOscBufferSize);
        const float dx = oscBounds.getWidth() / (float)numSamples;
        bool started = false;

        // Glow layer (thicker, translucent)
        juce::Path glowPath;
        for (int i = 0; i < numSamples; ++i)
        {
            float val = displayBuffer_[static_cast<size_t>(i + (kOscBufferSize - numSamples))];
            float x = oscBounds.getX() + (float)i * dx;
            float y = oscBounds.getCentreY() - val * oscBounds.getHeight() * 0.45f;

            if (!started)
            {
                glowPath.startNewSubPath(x, y);
                started = true;
            }
            else
            {
                glowPath.lineTo(x, y);
            }
        }
        g.setColour(juce::Colour(0xFF00FF88).withAlpha(0.15f));
        g.strokePath(glowPath, juce::PathStrokeType(4.0f));

        // Main waveform line
        g.setColour(juce::Colour(0xFF00FF88));
        g.strokePath(glowPath, juce::PathStrokeType(1.4f));

        // Label
        g.setColour(juce::Colour(CustomLookAndFeel::kTextDim));
        g.setFont(11.0f);
        g.drawText("OSC", oscBounds.getX() + 4, oscBounds.getY() + 2, 30, 12, juce::Justification::left);
    }

    // --- Spectrum (right half) ---
    {
        auto specBounds = juce::Rectangle<float>(halfW + padding, padding, halfW - padding * 2.0f, graphH);
        const int numBars = kFFTSize / 2;
        const float barW = specBounds.getWidth() / (float)numBars;

        for (int i = 0; i < numBars; ++i)
        {
            float mag = displaySpectrum_[static_cast<size_t>(i)];
            if (mag < 0.01f) continue;

            float barH = mag * specBounds.getHeight();
            float x = specBounds.getX() + (float)i * barW;
            float y = specBounds.getBottom() - barH;

            // Gradient from blue to red based on frequency
            float t = (float)i / (float)numBars;
            juce::Colour barColour = juce::Colour(0xFF0F3460).interpolatedWith(
                juce::Colour(0xFFE94560), t);
            g.setColour(barColour);
            g.fillRect(x, y, juce::jmax(1.0f, barW), barH);
        }

        // Label
        g.setColour(juce::Colour(CustomLookAndFeel::kTextDim));
        g.setFont(11.0f);
        g.drawText("FFT", specBounds.getX() + 4, specBounds.getY() + 2, 30, 12, juce::Justification::left);
    }
}

void Visualizer::resized() {}
