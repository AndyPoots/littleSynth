// Source/PluginProcessor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "VoiceManager.h"

class LittleSynthProcessor : public juce::AudioProcessor
{
public:
    LittleSynthProcessor();
    ~LittleSynthProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "littleSynth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // Public access for the editor to reach the APVTS (future use)
    juce::AudioProcessorValueTreeState& getAPVTS() { return *apvts_; }

private:
    VoiceManager synth_;
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthProcessor)
};
