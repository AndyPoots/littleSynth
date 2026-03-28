// Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

LittleSynthEditor::LittleSynthEditor(LittleSynthProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(800, 500);
}

LittleSynthEditor::~LittleSynthEditor() {}

void LittleSynthEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("littleSynth", getLocalBounds(),
        juce::Justification::centred, true);
}

void LittleSynthEditor::resized() {}
