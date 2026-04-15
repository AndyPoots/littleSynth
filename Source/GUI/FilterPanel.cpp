// Source/GUI/FilterPanel.cpp
#include "FilterPanel.h"
#include "CustomLookAndFeel.h"

FilterPanel::FilterPanel(juce::AudioProcessorValueTreeState& apvts)
{
    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText("FILTER", juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centredLeft);

    // Mode
    addAndMakeVisible(modeCombo_);
    modeCombo_.addItemList({ "Lowpass", "Highpass", "Bandpass", "Notch" }, 1);
    modeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "filter_mode", modeCombo_);

    // Slope
    addAndMakeVisible(slopeCombo_);
    slopeCombo_.addItemList({ "12 dB/oct", "24 dB/oct" }, 1);
    slopeAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, "filter_slope", slopeCombo_);

    // Cutoff - large prominent knob
    addAndMakeVisible(cutoffSlider_);
    cutoffSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "filter_cutoff", cutoffSlider_);
    addAndMakeVisible(cutoffLabel_);
    cutoffLabel_.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel_.setJustificationType(juce::Justification::centred);

    // Resonance
    addAndMakeVisible(resonanceSlider_);
    resonanceSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    resonanceAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "filter_resonance", resonanceSlider_);
    addAndMakeVisible(resonanceLabel_);
    resonanceLabel_.setText("Reso", juce::dontSendNotification);
    resonanceLabel_.setJustificationType(juce::Justification::centred);

    // Drive
    addAndMakeVisible(driveSlider_);
    driveSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "filter_drive", driveSlider_);
    addAndMakeVisible(driveLabel_);
    driveLabel_.setText("Drive", juce::dontSendNotification);
    driveLabel_.setJustificationType(juce::Justification::centred);

    // Env Amount
    addAndMakeVisible(envAmountSlider_);
    envAmountSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    envAmountSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    envAmountAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "filter_env_amount", envAmountSlider_);
    addAndMakeVisible(envAmountLabel_);
    envAmountLabel_.setText("Env", juce::dontSendNotification);
    envAmountLabel_.setJustificationType(juce::Justification::centred);

    // Key Tracking
    addAndMakeVisible(keyTrackingSlider_);
    keyTrackingSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    keyTrackingSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    keyTrackingAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "filter_key_tracking", keyTrackingSlider_);
    addAndMakeVisible(keyTrackingLabel_);
    keyTrackingLabel_.setText("Key", juce::dontSendNotification);
    keyTrackingLabel_.setJustificationType(juce::Justification::centred);

    // Style labels
    auto styleLabel = [](juce::Label& l) {
        l.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        l.setFont(juce::Font(14.0f, juce::Font::bold));
    };
    styleLabel(cutoffLabel_);
    styleLabel(resonanceLabel_);
    styleLabel(driveLabel_);
    styleLabel(envAmountLabel_);
    styleLabel(keyTrackingLabel_);
}

void FilterPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
}

void FilterPanel::resized()
{
    auto area = getLocalBounds().reduced(6);

    // Title
    titleLabel_.setBounds(area.removeFromTop(16));
    area.removeFromTop(3);

    // Dropdowns row
    auto dropdownRow = area.removeFromTop(22);
    modeCombo_.setBounds(dropdownRow.removeFromLeft(dropdownRow.getWidth() / 2).reduced(2));
    slopeCombo_.setBounds(dropdownRow.reduced(2));

    // Large cutoff knob in center
    const int bigKnobSize = 70;
    const int smallKnobSize = 44;
    const int labelH = 12;

    auto cutoffArea = area.removeFromTop(bigKnobSize + labelH);
    cutoffLabel_.setBounds(cutoffArea.getX(), cutoffArea.getY(), cutoffArea.getWidth(), labelH);
    cutoffSlider_.setBounds(cutoffArea.getX() + (cutoffArea.getWidth() - bigKnobSize) / 2,
                            cutoffArea.getY() + labelH, bigKnobSize, bigKnobSize);

    // Bottom row: Reso, Drive, Env, Key
    auto knobRow = area;
    const int colW = knobRow.getWidth() / 4;

    auto buildCol = [&](juce::Label& label, juce::Slider& slider, int x, int w) {
        label.setBounds(x, knobRow.getY(), w, labelH);
        slider.setBounds(x + (w - smallKnobSize) / 2, knobRow.getY() + labelH, smallKnobSize, smallKnobSize);
    };

    buildCol(resonanceLabel_, resonanceSlider_, knobRow.getX(), colW);
    buildCol(driveLabel_, driveSlider_, knobRow.getX() + colW, colW);
    buildCol(envAmountLabel_, envAmountSlider_, knobRow.getX() + colW * 2, colW);
    buildCol(keyTrackingLabel_, keyTrackingSlider_, knobRow.getX() + colW * 3, colW);
}
