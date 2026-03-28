// Source/PluginEditor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class LittleSynthProcessor;

class LittleSynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit LittleSynthEditor(LittleSynthProcessor&);
    ~LittleSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LittleSynthProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthEditor)
};
