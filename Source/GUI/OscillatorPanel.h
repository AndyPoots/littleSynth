// Source/GUI/OscillatorPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class OscillatorPanel : public juce::Component
{
public:
    OscillatorPanel(juce::AudioProcessorValueTreeState& apvts, int index);
    ~OscillatorPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    int index_;
    juce::String prefix_;

    juce::Label titleLabel_;

    juce::ComboBox waveformCombo_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment_;

    juce::Slider detuneSlider_;
    juce::Label detuneLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment_;

    juce::ComboBox octaveCombo_;
    juce::Label octaveLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> octaveAttachment_;

    juce::Slider levelSlider_;
    juce::Label levelLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelAttachment_;

    juce::Slider pulseWidthSlider_;
    juce::Label pulseWidthLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pulseWidthAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorPanel)
};
