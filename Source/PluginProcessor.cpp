// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

LittleSynthProcessor::LittleSynthProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Create APVTS with an empty parameter layout for now.
    // Parameters will be added in a later task.
    apvts_ = std::make_unique<juce::AudioProcessorValueTreeState>(
        *this, nullptr, "Parameters", juce::AudioProcessorValueTreeState::ParameterLayout{});
}

LittleSynthProcessor::~LittleSynthProcessor() = default;

// ---------------------------------------------------------------
// AudioProcessor lifecycle
// ---------------------------------------------------------------

void LittleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth_.prepareToPlay(sampleRate, samplesPerBlock);
}

void LittleSynthProcessor::releaseResources()
{
    // Nothing extra to do; VoiceManager handles its own cleanup
}

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

void LittleSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear the buffer first (synth produces sound additively via renderNextBlock)
    buffer.clear();

    // Let the VoiceManager render all active voices into the buffer
    synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

// ---------------------------------------------------------------
// Editor
// ---------------------------------------------------------------

juce::AudioProcessorEditor* LittleSynthProcessor::createEditor()
{
    return new LittleSynthEditor(*this);
}

// ---------------------------------------------------------------
// Plugin entry point
// ---------------------------------------------------------------

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LittleSynthProcessor();
}
