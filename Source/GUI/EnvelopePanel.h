// Source/GUI/EnvelopePanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class EnvelopePanel : public juce::Component
{
public:
    EnvelopePanel(juce::AudioProcessorValueTreeState& apvts, const juce::String& prefix, const juce::String& title);
    ~EnvelopePanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::String prefix_;
    juce::String titleText_;

    juce::Label titleLabel_;

    // ADSR curve preview area
    juce::Component adsrPreview_;

    juce::Slider attackSlider_;
    juce::Label attackLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment_;

    juce::Slider decaySlider_;
    juce::Label decayLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment_;

    juce::Slider sustainSlider_;
    juce::Label sustainLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment_;

    juce::Slider releaseSlider_;
    juce::Label releaseLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment_;

    juce::AudioProcessorValueTreeState& apvts_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopePanel)
};
