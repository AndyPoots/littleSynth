// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

LittleSynthProcessor::LittleSynthProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

LittleSynthProcessor::~LittleSynthProcessor() {}

void LittleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void LittleSynthProcessor::releaseResources() {}

void LittleSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    buffer.clear();
}

juce::AudioProcessorEditor* LittleSynthProcessor::createEditor()
{
    return new LittleSynthEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LittleSynthProcessor();
}
