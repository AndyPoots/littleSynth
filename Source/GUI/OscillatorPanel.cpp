// Source/GUI/OscillatorPanel.cpp
#include "OscillatorPanel.h"
#include "CustomLookAndFeel.h"

OscillatorPanel::OscillatorPanel(juce::AudioProcessorValueTreeState& apvts, int index)
    : index_(index)
{
    const char* prefixes[] = { "osc1_", "osc2_", "osc3_" };
    prefix_ = prefixes[index];

    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText("OSC " + juce::String(index + 1), juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centredLeft);

    // On/Off toggle
    addAndMakeVisible(onToggle_);
    onAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, prefix_ + "on", onToggle_);

    // Waveform
    addAndMakeVisible(waveformCombo_);
    waveformCombo_.addItemList({ "Sine", "Triangle", "Sawtooth", "Square", "Noise" }, 1);
    waveformAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, prefix_ + "waveform", waveformCombo_);

    // Detune
    addAndMakeVisible(detuneSlider_);
    detuneSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    detuneSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    detuneAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "detune", detuneSlider_);
    addAndMakeVisible(detuneLabel_);
    detuneLabel_.setText("Detune", juce::dontSendNotification);
    detuneLabel_.setJustificationType(juce::Justification::centred);

    // Octave
    addAndMakeVisible(octaveCombo_);
    octaveCombo_.addItemList({ "-2", "-1", "0", "+1", "+2" }, 1);
    octaveAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, prefix_ + "octave", octaveCombo_);
    addAndMakeVisible(octaveLabel_);
    octaveLabel_.setText("Oct", juce::dontSendNotification);
    octaveLabel_.setJustificationType(juce::Justification::centred);

    // Level
    addAndMakeVisible(levelSlider_);
    levelSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    levelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    levelAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "level", levelSlider_);
    addAndMakeVisible(levelLabel_);
    levelLabel_.setText("Level", juce::dontSendNotification);
    levelLabel_.setJustificationType(juce::Justification::centred);

    // Pulse Width
    addAndMakeVisible(pulseWidthSlider_);
    pulseWidthSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pulseWidthSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pulseWidthAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "pulse_width", pulseWidthSlider_);
    addAndMakeVisible(pulseWidthLabel_);
    pulseWidthLabel_.setText("PW", juce::dontSendNotification);
    pulseWidthLabel_.setJustificationType(juce::Justification::centred);

    // Style labels
    auto styleLabel = [](juce::Label& l) {
        l.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        l.setFont(juce::Font(14.0f, juce::Font::bold));
    };
    styleLabel(detuneLabel_);
    styleLabel(levelLabel_);
    styleLabel(pulseWidthLabel_);
    styleLabel(octaveLabel_);
}

void OscillatorPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border (dimmed when oscillator is off)
    if (onToggle_.getToggleState())
        g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    else
        g.setColour(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.6f));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
}

void OscillatorPanel::resized()
{
    auto area = getLocalBounds().reduced(6);

    // Title row: toggle on the left, title text fills the rest
    auto titleArea = area.removeFromTop(16);
    onToggle_.setBounds(titleArea.removeFromLeft(20).reduced(2));
    titleLabel_.setBounds(titleArea);
    area.removeFromTop(3);

    // Waveform combo
    waveformCombo_.setBounds(area.removeFromTop(20));

    // Knobs row: detune, octave combo, level, pulse width
    auto knobArea = area;
    const int knobSize = 50;
    const int labelH = 12;
    const int colW = knobArea.getWidth() / 4;

    // Detune
    {
        auto col = knobArea.removeFromLeft(colW);
        detuneLabel_.setBounds(col.getX(), col.getY(), colW, labelH);
        detuneSlider_.setBounds(col.getX() + (colW - knobSize) / 2, col.getY() + labelH, knobSize, knobSize);
    }

    // Octave
    {
        auto col = knobArea.removeFromLeft(colW);
        octaveLabel_.setBounds(col.getX(), col.getY(), colW, labelH);
        octaveCombo_.setBounds(col.getX() + 4, col.getY() + labelH, colW - 8, 20);
    }

    // Level
    {
        auto col = knobArea.removeFromLeft(colW);
        levelLabel_.setBounds(col.getX(), col.getY(), colW, labelH);
        levelSlider_.setBounds(col.getX() + (colW - knobSize) / 2, col.getY() + labelH, knobSize, knobSize);
    }

    // Pulse Width
    {
        auto col = knobArea.removeFromLeft(colW);
        pulseWidthLabel_.setBounds(col.getX(), col.getY(), colW, labelH);
        pulseWidthSlider_.setBounds(col.getX() + (colW - knobSize) / 2, col.getY() + labelH, knobSize, knobSize);
    }
}
