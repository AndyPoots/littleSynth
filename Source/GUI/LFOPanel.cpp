// Source/GUI/LFOPanel.cpp
#include "LFOPanel.h"
#include "CustomLookAndFeel.h"
#include <cmath>

LFOPanel::LFOPanel(juce::AudioProcessorValueTreeState& apvts, int index)
    : index_(index), apvts_(apvts)
{
    const char* prefixes[] = { "lfo1_", "lfo2_" };
    prefix_ = prefixes[index];

    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText("LFO " + juce::String(index + 1), juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(11.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centred);

    // Shape
    addAndMakeVisible(shapeCombo_);
    shapeCombo_.addItemList({ "Sine", "Triangle", "Sawtooth", "Square", "S&H", "Random" }, 1);
    shapeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, prefix_ + "shape", shapeCombo_);

    // Rate
    addAndMakeVisible(rateSlider_);
    rateSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rateSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rateAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "rate", rateSlider_);
    addAndMakeVisible(rateLabel_);
    rateLabel_.setText("Rate", juce::dontSendNotification);
    rateLabel_.setJustificationType(juce::Justification::centred);

    // Depth
    addAndMakeVisible(depthSlider_);
    depthSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    depthSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "depth", depthSlider_);
    addAndMakeVisible(depthLabel_);
    depthLabel_.setText("Depth", juce::dontSendNotification);
    depthLabel_.setJustificationType(juce::Justification::centred);

    // Style labels
    auto styleLabel = [](juce::Label& l) {
        l.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        l.setFont(9.0f);
    };
    styleLabel(rateLabel_);
    styleLabel(depthLabel_);
}

void LFOPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccentBlue));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    // Draw waveform preview
    // Find a small region below the shape combo and above the knobs
    auto area = getLocalBounds().reduced(6);
    area.removeFromTop(14); // title
    area.removeFromTop(22); // combo

    // Preview region (about 30px tall)
    auto previewArea = area.removeFromTop(30).toFloat().reduced(2.0f);

    if (previewArea.getWidth() > 10.0f && previewArea.getHeight() > 5.0f)
    {
        // Background
        g.setColour(juce::Colour(CustomLookAndFeel::kDarkerBg));
        g.fillRoundedRectangle(previewArea, 2.0f);

        // Get current shape
        int shape = static_cast<int>(apvts_.getRawParameterValue(prefix_ + "shape")->load());

        const float pw = previewArea.getWidth();
        const float ph = previewArea.getHeight();
        const float midY = previewArea.getCentreY();
        const float amp = ph * 0.4f;
        const int numPoints = 100;

        g.setColour(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.7f));
        juce::Path wavePath;
        bool started = false;

        for (int i = 0; i < numPoints; ++i)
        {
            float t = (float)i / (float)(numPoints - 1);
            float x = previewArea.getX() + t * pw;
            float y = midY;

            switch (shape)
            {
                case 0: // Sine
                    y = midY - amp * std::sin(t * 2.0f * 3.14159265f);
                    break;
                case 1: // Triangle
                    y = midY - amp * (2.0f * std::abs(2.0f * (t - std::floor(t + 0.5f))) - 1.0f);
                    break;
                case 2: // Sawtooth
                    y = midY - amp * (2.0f * (t - std::floor(t + 0.5f)));
                    break;
                case 3: // Square
                    y = midY - amp * (std::sin(t * 2.0f * 3.14159265f) >= 0.0f ? 1.0f : -1.0f);
                    break;
                case 4: // S&H - stepped
                {
                    int step = (int)(t * 8.0f);
                    float val = std::sin((float)step * 2.7f) * 0.8f;
                    y = midY - amp * val;
                    break;
                }
                case 5: // Random - jagged
                    y = midY - amp * std::sin(t * 17.3f) * std::cos(t * 7.1f);
                    break;
                default:
                    break;
            }

            if (!started)
            {
                wavePath.startNewSubPath(x, y);
                started = true;
            }
            else
            {
                wavePath.lineTo(x, y);
            }
        }
        g.strokePath(wavePath, juce::PathStrokeType(1.0f));
    }
}

void LFOPanel::resized()
{
    auto area = getLocalBounds().reduced(6);

    // Title
    titleLabel_.setBounds(area.removeFromTop(14));

    // Shape combo
    shapeCombo_.setBounds(area.removeFromTop(22));

    // Space for waveform preview (painted in paint())
    area.removeFromTop(30);

    // Knobs: Rate, Depth
    const int knobSize = 40;
    const int labelH = 12;
    const int colW = area.getWidth() / 2;

    rateLabel_.setBounds(area.getX(), area.getY(), colW, labelH);
    rateSlider_.setBounds(area.getX() + (colW - knobSize) / 2, area.getY() + labelH, knobSize, knobSize);

    depthLabel_.setBounds(area.getX() + colW, area.getY(), colW, labelH);
    depthSlider_.setBounds(area.getX() + colW + (colW - knobSize) / 2, area.getY() + labelH, knobSize, knobSize);
}
