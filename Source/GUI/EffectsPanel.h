// Source/GUI/EffectsPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../EffectsChain.h"

class LittleSynthProcessor;

class EffectsPanel : public juce::Component
{
public:
    EffectsPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor);
    ~EffectsPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /// Push UI state to effects
    void updateProcessorState();

private:
    struct EffectSection
    {
        juce::String name;
        std::unique_ptr<juce::ToggleButton> enableButton;
        std::vector<std::unique_ptr<juce::Slider>> sliders;
        std::vector<std::unique_ptr<juce::Label>> labels;
    };

    void createEQSection();
    void createCompressorSection();
    void createChorusSection();
    void createPhaserSection();
    void createFlangerSection();
    void createDelaySection();
    void createReverbSection();
    void createDistortionSection();

    juce::Label titleLabel_;
    std::array<EffectSection, 8> sections_;
    std::array<juce::Rectangle<int>, 8> sectionBounds_;
    juce::AudioProcessorValueTreeState& apvts_;
    LittleSynthProcessor& processor_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsPanel)
};
