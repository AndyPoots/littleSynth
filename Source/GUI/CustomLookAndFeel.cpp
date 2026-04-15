// Source/GUI/CustomLookAndFeel.cpp
#include "CustomLookAndFeel.h"
#include <cmath>

CustomLookAndFeel::CustomLookAndFeel()
{
    setColour(juce::Slider::backgroundColourId,    juce::Colour(kTrackBg));
    setColour(juce::Slider::thumbColourId,          juce::Colour(kAccent));
    setColour(juce::Slider::trackColourId,          juce::Colour(kAccent));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(kAccent));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(kTrackBg));
    setColour(juce::Slider::textBoxTextColourId,    juce::Colour(kText));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(kPanelBg));
    setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(kAccent));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(kPanelBg));

    setColour(juce::ComboBox::backgroundColourId,   juce::Colour(kPanelBg));
    setColour(juce::ComboBox::textColourId,         juce::Colour(kText));
    setColour(juce::ComboBox::outlineColourId,      juce::Colour(kAccentBlue));
    setColour(juce::ComboBox::buttonColourId,       juce::Colour(kAccentBlue));
    setColour(juce::ComboBox::arrowColourId,        juce::Colour(kText));

    setColour(juce::PopupMenu::backgroundColourId,  juce::Colour(kPanelBg));
    setColour(juce::PopupMenu::textColourId,        juce::Colour(kText));
    setColour(juce::PopupMenu::headerTextColourId,  juce::Colour(kAccent));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(kAccentBlue));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(juce::Colours::white));

    setColour(juce::ToggleButton::textColourId,     juce::Colour(kText));
    setColour(juce::TextButton::buttonColourId,     juce::Colour(kAccentBlue));
    setColour(juce::TextButton::textColourOnId,     juce::Colour(kText));
    setColour(juce::TextButton::textColourOffId,    juce::Colour(kText));

    setColour(juce::Label::textColourId,            juce::Colour(kText));
    setColour(juce::Label::backgroundColourId,      juce::Colour(kPanelBg));
    setColour(juce::Label::outlineColourId,         juce::Colour(kPanelBg));

    setColour(juce::GroupComponent::textColourId,   juce::Colour(kAccent));
    setColour(juce::GroupComponent::outlineColourId, juce::Colour(kAccentBlue));
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    const float radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    if (radius <= 0.0f) return;

    const float centreX = (float)x + (float)width * 0.5f;
    const float centreY = (float)y + (float)height * 0.5f;
    const float lineWidth = juce::jmax(2.0f, radius * 0.12f);

    // Background track arc
    const float trackRadius = radius - lineWidth * 0.5f;
    juce::Path bgArc;
    bgArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(kTrackBg));
    g.strokePath(bgArc, juce::PathStrokeType(lineWidth));

    // Value arc
    const float valueAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    if (valueAngle > rotaryStartAngle)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f,
                               rotaryStartAngle, valueAngle, true);
        g.setColour(juce::Colour(kAccent));
        g.strokePath(valueArc, juce::PathStrokeType(lineWidth));
    }

    // Indicator dot
    const float dotRadius = lineWidth * 0.8f;
    const float dotX = centreX + trackRadius * std::cos(valueAngle - juce::MathConstants<float>::halfPi);
    const float dotY = centreY + trackRadius * std::sin(valueAngle - juce::MathConstants<float>::halfPi);
    g.setColour(juce::Colour(kAccent));
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);

    // Center knob circle
    const float innerRadius = radius * 0.4f;
    g.setColour(juce::Colour(kKnobBg));
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

    // Value text below knob (only if slider has a text box that's not visible)
    if (!slider.isTextBoxEditable())
    {
        const float val = (float)slider.getValue();
        juce::String text;
        if (slider.getInterval() >= 1.0f && slider.getRange().getLength() < 20.0f)
            text = juce::String((int)val);
        else if (slider.getRange().getLength() >= 100.0f)
            text = juce::String(val, 0);
        else
            text = juce::String(val, 2);

        g.setColour(juce::Colour(kTextDim));
        g.setFont(11.0f);
        g.drawText(text, (int)(centreX - innerRadius), (int)(centreY - 5.0f),
                   (int)(innerRadius * 2.0f), 10, juce::Justification::centred);
    }
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar)
    {
        const float trackY = (float)y + (float)height * 0.5f - 2.0f;
        const float trackH = 4.0f;
        const float trackW = (float)width;

        // Background track
        g.setColour(juce::Colour(kTrackBg));
        g.fillRoundedRectangle((float)x, trackY, trackW, trackH, 2.0f);

        // Fill
        const float fillW = sliderPos - (float)x;
        if (fillW > 0.0f)
        {
            g.setColour(juce::Colour(kAccent));
            g.fillRoundedRectangle((float)x, trackY, fillW, trackH, 2.0f);
        }

        // Thumb
        const float thumbSize = 10.0f;
        g.setColour(juce::Colour(kAccent));
        g.fillEllipse(sliderPos - thumbSize * 0.5f, trackY - (thumbSize - trackH) * 0.5f,
                       thumbSize, thumbSize);
    }
    else
    {
        // Vertical
        const float trackX = (float)x + (float)width * 0.5f - 2.0f;
        const float trackH = (float)height;
        const float trackW = 4.0f;

        g.setColour(juce::Colour(kTrackBg));
        g.fillRoundedRectangle(trackX, (float)y, trackW, trackH, 2.0f);

        const float fillH = (float)y + (float)height - sliderPos;
        if (fillH > 0.0f)
        {
            g.setColour(juce::Colour(kAccent));
            g.fillRoundedRectangle(trackX, sliderPos, trackW, fillH, 2.0f);
        }

        const float thumbSize = 10.0f;
        g.setColour(juce::Colour(kAccent));
        g.fillEllipse(trackX - (thumbSize - trackW) * 0.5f, sliderPos - thumbSize * 0.5f,
                       thumbSize, thumbSize);
    }
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH,
                                     juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height);

    // Background
    g.setColour(juce::Colour(kPanelBg));
    g.fillRoundedRectangle(bounds.toFloat(), 3.0f);

    // Border
    g.setColour(juce::Colour(kAccent));
    g.drawRoundedRectangle(bounds.toFloat(), 3.0f, 1.5f);

    // Arrow
    const float arrowX = (float)buttonX + (float)buttonW * 0.5f;
    const float arrowY = (float)buttonY + (float)buttonH * 0.5f;
    const float arrowSize = 5.0f;

    juce::Path arrow;
    arrow.addTriangle(arrowX - arrowSize, arrowY - arrowSize * 0.5f,
                      arrowX + arrowSize, arrowY - arrowSize * 0.5f,
                      arrowX, arrowY + arrowSize * 0.5f);
    g.setColour(juce::Colour(kText));
    g.fillPath(arrow);
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                           bool isSeparator, bool isActive, bool isHighlighted,
                                           bool isTicked, bool hasSubMenu, const juce::String& text,
                                           const juce::String& shortcutKeyText,
                                           const juce::Drawable* icon,
                                           const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour(juce::Colour(kTrackBg));
        g.fillRect(area.reduced(4, 0).withHeight(1).withCentre(area.getCentre()));
        return;
    }

    auto bg = area.toFloat();
    if (isHighlighted && isActive)
    {
        g.setColour(juce::Colour(kAccentBlue));
        g.fillRect(bg);
    }

    auto textArea = area.reduced(6, 0);
    g.setColour(isActive ? (isHighlighted ? juce::Colours::white : juce::Colour(kText))
                         : juce::Colour(kTextDim));
    g.setFont(getPopupMenuFont());
    g.drawText(text, textArea, juce::Justification::centredLeft);

    if (isTicked)
    {
        g.setColour(juce::Colour(kAccent));
        g.fillRect(area.getX() + 3, area.getCentreY() - 1, 3, 3);
    }
}

void CustomLookAndFeel::getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                                   int standardMenuItemHeight, int& idealWidth,
                                                   int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = 4;
    }
    else
    {
        juce::Font f(getPopupMenuFont());
        idealWidth = (int)f.getStringWidth(text) + 30;
        idealHeight = juce::jmax(18, standardMenuItemHeight);
    }
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::Font(12.0f);
}

juce::Font CustomLookAndFeel::getPopupMenuFont()
{
    return juce::Font(12.0f);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted,
                                          bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    const float size = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.7f;
    const float pad = 4.0f;

    // Toggle square aligned to the left
    auto toggleBounds = juce::Rectangle<float>(bounds.getX() + pad, bounds.getCentreY() - size * 0.5f, size, size);

    const bool isOn = button.getToggleState();

    // Background
    g.setColour(isOn ? juce::Colour(kAccent) : juce::Colour(kTrackBg));
    g.fillRoundedRectangle(toggleBounds, 3.0f);

    // Border
    g.setColour(juce::Colour(kAccent));
    g.drawRoundedRectangle(toggleBounds, 3.0f, 1.5f);

    // Text after toggle, left-aligned
    auto buttonText = button.getButtonText();
    if (buttonText.isNotEmpty())
    {
        g.setColour(juce::Colour(kText));
        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.drawText(buttonText,
                   (int)(toggleBounds.getRight() + 4.0f), (int)bounds.getY(),
                   (int)(bounds.getRight() - toggleBounds.getRight() - 4.0f), (int)bounds.getHeight(),
                   juce::Justification::centredLeft);
    }
}

void CustomLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited())
    {
        const float alpha = label.isEnabled() ? 1.0f : 0.5f;
        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(label.getFont());
        g.drawText(label.getText(), label.getLocalBounds(),
                   label.getJustificationType(), true);
    }
}

void CustomLookAndFeel::drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                                   const juce::String& text,
                                                   const juce::Justification& position,
                                                   juce::GroupComponent& group)
{
    // Fill background
    g.setColour(juce::Colour(kPanelBg).withAlpha(0.8f));
    g.fillRoundedRectangle(0.0f, 0.0f, (float)width, (float)height, 8.0f);

    // Border
    g.setColour(juce::Colour(kAccent));
    g.drawRoundedRectangle(1.0f, 1.0f, (float)(width - 2), (float)(height - 2), 8.0f, 2.0f);

    // Title
    if (text.isNotEmpty())
    {
        g.setColour(juce::Colour(kAccent));
        g.setFont(juce::Font(juce::FontOptions().withHeight(13.0f).withStyle("Bold")));
        g.drawText(text, 8, 3, width - 16, 16, juce::Justification::centredLeft);
    }
}
