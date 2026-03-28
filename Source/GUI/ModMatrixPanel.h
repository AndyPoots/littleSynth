// Source/GUI/ModMatrixPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../ModMatrix.h"

class LittleSynthProcessor;

class ModMatrixPanel : public juce::Component
{
public:
    ModMatrixPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor);
    ~ModMatrixPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /// Called to push current UI state to the processor's voices
    void updateProcessorState();

private:
    static constexpr int kNumSlots = 16;

    struct SlotUI
    {
        std::unique_ptr<juce::ComboBox> sourceCombo;
        std::unique_ptr<juce::ComboBox> destCombo;
        std::unique_ptr<juce::Slider> depthSlider;
        std::unique_ptr<juce::ToggleButton> bipolarToggle;
        std::unique_ptr<juce::ToggleButton> activeToggle;
    };

    juce::Label titleLabel_;
    juce::Label sourceHeader_;
    juce::Label destHeader_;
    juce::Label depthHeader_;
    juce::Label bipolarHeader_;
    juce::Label activeHeader_;

    std::array<SlotUI, kNumSlots> slots_;
    juce::AudioProcessorValueTreeState& apvts_;
    LittleSynthProcessor& processor_;

    static juce::StringArray getSourceNames();
    static juce::StringArray getDestNames();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModMatrixPanel)
};
