// Source/GUI/LFOPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class LFOPanel : public juce::Component
{
public:
    LFOPanel(juce::AudioProcessorValueTreeState& apvts, int index);
    ~LFOPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    int index_;
    juce::String prefix_;

    juce::Label titleLabel_;

    juce::ComboBox shapeCombo_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> shapeAttachment_;

    juce::Slider rateSlider_;
    juce::Label rateLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rateAttachment_;

    juce::Slider depthSlider_;
    juce::Label depthLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment_;

    juce::AudioProcessorValueTreeState& apvts_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOPanel)
};
