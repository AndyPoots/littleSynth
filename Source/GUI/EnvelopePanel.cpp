// Source/GUI/EnvelopePanel.cpp
#include "EnvelopePanel.h"
#include "CustomLookAndFeel.h"

EnvelopePanel::EnvelopePanel(juce::AudioProcessorValueTreeState& apvts,
                             const juce::String& prefix,
                             const juce::String& title)
    : prefix_(prefix), titleText_(title), apvts_(apvts)
{
    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText(titleText_, juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(11.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centred);

    // ADSR preview
    addAndMakeVisible(adsrPreview_);

    // Attack
    addAndMakeVisible(attackSlider_);
    attackSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    attackSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    attackAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "attack", attackSlider_);
    addAndMakeVisible(attackLabel_);
    attackLabel_.setText("A", juce::dontSendNotification);
    attackLabel_.setJustificationType(juce::Justification::centred);

    // Decay
    addAndMakeVisible(decaySlider_);
    decaySlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    decaySlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    decayAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "decay", decaySlider_);
    addAndMakeVisible(decayLabel_);
    decayLabel_.setText("D", juce::dontSendNotification);
    decayLabel_.setJustificationType(juce::Justification::centred);

    // Sustain
    addAndMakeVisible(sustainSlider_);
    sustainSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    sustainSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sustainAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "sustain", sustainSlider_);
    addAndMakeVisible(sustainLabel_);
    sustainLabel_.setText("S", juce::dontSendNotification);
    sustainLabel_.setJustificationType(juce::Justification::centred);

    // Release
    addAndMakeVisible(releaseSlider_);
    releaseSlider_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    releaseSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    releaseAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix_ + "release", releaseSlider_);
    addAndMakeVisible(releaseLabel_);
    releaseLabel_.setText("R", juce::dontSendNotification);
    releaseLabel_.setJustificationType(juce::Justification::centred);

    // Style labels
    auto styleLabel = [](juce::Label& l) {
        l.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        l.setFont(9.0f);
    };
    styleLabel(attackLabel_);
    styleLabel(decayLabel_);
    styleLabel(sustainLabel_);
    styleLabel(releaseLabel_);
}

void EnvelopePanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccentBlue));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    // Draw ADSR preview curve
    auto previewBounds = adsrPreview_.getBounds().toFloat().reduced(4.0f);
    if (previewBounds.getWidth() > 10.0f && previewBounds.getHeight() > 10.0f)
    {
        // Read current values
        float a = apvts_.getRawParameterValue(prefix_ + "attack")->load();
        float d = apvts_.getRawParameterValue(prefix_ + "decay")->load();
        float s = apvts_.getRawParameterValue(prefix_ + "sustain")->load();
        float r = apvts_.getRawParameterValue(prefix_ + "release")->load();

        // Normalize times (max ~10 seconds each)
        const float totalTime = a + d + 0.5f + r; // sustain held for 0.5s for display
        const float pw = previewBounds.getWidth();
        const float ph = previewBounds.getHeight();

        // Background
        g.setColour(juce::Colour(CustomLookAndFeel::kDarkerBg));
        g.fillRoundedRectangle(previewBounds, 2.0f);

        // Draw curve
        g.setColour(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.8f));
        juce::Path curve;

        float x0 = previewBounds.getX();
        float y0 = previewBounds.getBottom();

        // Start
        curve.startNewSubPath(x0, y0);

        // Attack phase
        float aX = x0 + (a / totalTime) * pw;
        curve.lineTo(aX, previewBounds.getY());

        // Decay phase
        float dX = aX + (d / totalTime) * pw;
        float sY = previewBounds.getBottom() - s * ph;
        curve.lineTo(dX, sY);

        // Sustain phase (short hold)
        float sX = dX + (0.5f / totalTime) * pw;
        curve.lineTo(sX, sY);

        // Release phase
        float rX = sX + (r / totalTime) * pw;
        curve.lineTo(rX, y0);

        g.strokePath(curve, juce::PathStrokeType(1.5f));

        // Fill under curve
        curve.lineTo(rX, y0);
        curve.closeSubPath();
        g.setColour(juce::Colour(CustomLookAndFeel::kAccent).withAlpha(0.15f));
        g.fillPath(curve);
    }
}

void EnvelopePanel::resized()
{
    auto area = getLocalBounds().reduced(6);

    // Title
    titleLabel_.setBounds(area.removeFromTop(14));

    // ADSR preview
    adsrPreview_.setBounds(area.removeFromTop(40));

    // Knobs
    const int knobSize = 40;
    const int labelH = 12;
    const int colW = area.getWidth() / 4;

    auto buildCol = [&](juce::Label& label, juce::Slider& slider, int x) {
        label.setBounds(x, area.getY(), colW, labelH);
        slider.setBounds(x + (colW - knobSize) / 2, area.getY() + labelH, knobSize, knobSize);
    };

    buildCol(attackLabel_, attackSlider_, area.getX());
    buildCol(decayLabel_, decaySlider_, area.getX() + colW);
    buildCol(sustainLabel_, sustainSlider_, area.getX() + colW * 2);
    buildCol(releaseLabel_, releaseSlider_, area.getX() + colW * 3);
}
