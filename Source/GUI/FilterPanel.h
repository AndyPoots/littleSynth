// Source/GUI/FilterPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class FilterPanel : public juce::Component
{
public:
    FilterPanel(juce::AudioProcessorValueTreeState& apvts);
    ~FilterPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel_;

    juce::ComboBox modeCombo_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment_;

    juce::ComboBox slopeCombo_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> slopeAttachment_;

    juce::Slider cutoffSlider_;
    juce::Label cutoffLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment_;

    juce::Slider resonanceSlider_;
    juce::Label resonanceLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment_;

    juce::Slider driveSlider_;
    juce::Label driveLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment_;

    juce::Slider envAmountSlider_;
    juce::Label envAmountLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envAmountAttachment_;

    juce::Slider keyTrackingSlider_;
    juce::Label keyTrackingLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> keyTrackingAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterPanel)
};
