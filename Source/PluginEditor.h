// Source/PluginEditor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GUI/MainPanel.h"

class LittleSynthProcessor;

class LittleSynthEditor : public juce::AudioProcessorEditor,
                          public juce::Timer
{
public:
    explicit LittleSynthEditor(LittleSynthProcessor&);
    ~LittleSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    LittleSynthProcessor& processorRef;

    MainPanel mainPanel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthEditor)
};
