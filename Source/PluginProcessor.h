// Source/PluginProcessor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "VoiceManager.h"
#include "EffectsChain.h"

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

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Public access for the editor to reach the APVTS
    juce::AudioProcessorValueTreeState& getAPVTS() { return *apvts_; }

    // Access for the GUI to push state
    VoiceManager& getSynth() { return synth_; }
    EffectsChain* getEffectsChain() { return &effectsChain_; }

    /// Called from the audio thread: pushes a mono-mixed sample to the output FIFO
    void pushOutputSample(float sample);

    /// Called from the GUI thread: copies up to maxSamples from the output FIFO into dest
    int readOutputSamples(float* dest, int maxSamples);

private:
    VoiceManager synth_;
    EffectsChain effectsChain_;
    std::unique_ptr<juce::AudioProcessorValueTreeState> apvts_;

    // Factory presets
    int currentProgram_ = 0;
    juce::StringArray presetNames_;
    juce::Array<juce::ValueTree> factoryPresets_;

    // Lock-free output FIFO for visualizer (power-of-2 size for cheap modulo)
    static constexpr int kOutputFifoSize = 4096;
    std::array<float, kOutputFifoSize> outputFifo_{};
    std::atomic<int> outputWritePos_{0};

    // Cached parameter values — only push to voices when changed
    struct ParamCache
    {
        int   oscWaveform[3]  = { 0, 0, 0 };
        float oscDetune[3]    = {};
        int   oscOctave[3]    = { 2, 2, 2 };
        float oscLevel[3]     = { 1.0f, 0.0f, 0.0f };
        float oscPulseWidth[3] = { 0.5f, 0.5f, 0.5f };
        bool  oscOn[3]        = { true, false, false };

        int   filterMode      = 0;
        int   filterSlope     = 1;
        float filterCutoff    = 5000.0f;
        float filterResonance = 0.0f;
        float filterDrive     = 1.0f;
        float filterEnvAmount = 0.0f;
        float filterKeyTracking = 0.0f;

        float envAttack[3]    = { 0.01f, 0.01f, 0.01f };
        float envDecay[3]     = { 0.1f, 0.1f, 0.1f };
        float envSustain[3]   = { 0.7f, 0.0f, 0.0f };
        float envRelease[3]   = { 0.3f, 0.3f, 0.3f };

        int   lfoShape[2]     = { 0, 0 };
        float lfoRate[2]      = { 1.0f, 1.0f };
        float lfoDepth[2]     = {};
    } paramCache_;

    void createFactoryPresets();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthProcessor)
};
