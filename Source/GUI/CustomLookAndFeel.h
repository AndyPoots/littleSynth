// Source/GUI/CustomLookAndFeel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Colors — black background theme
    static constexpr uint32_t kBackground    = 0xFF000000;
    static constexpr uint32_t kDarkerBg      = 0xFF0A0A0A;
    static constexpr uint32_t kPanelBg       = 0xFF141414;
    static constexpr uint32_t kAccent        = 0xFFE94560;
    static constexpr uint32_t kAccentBlue    = 0xFF0F3460;
    static constexpr uint32_t kDarkerBlue    = 0xFF0A1520;
    static constexpr uint32_t kText          = 0xFFE0E0E0;
    static constexpr uint32_t kTextDim       = 0xFF909090;
    static constexpr uint32_t kTrackBg       = 0xFF1E1E1E;
    static constexpr uint32_t kKnobBg        = 0xFF1E1E1E;

    // Rotary slider
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    // Linear slider
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle, juce::Slider& slider) override;

    // ComboBox
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    // Popup menu
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon,
                           const juce::Colour* textColour) override;

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
                                   int standardMenuItemHeight, int& idealWidth,
                                   int& idealHeight) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    // Toggle button
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // Label
    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    // Group component (panel outline)
    void drawGroupComponentOutline(juce::Graphics& g, int width, int height,
                                   const juce::String& text,
                                   const juce::Justification& position,
                                   juce::GroupComponent& group) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
