// Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

LittleSynthEditor::LittleSynthEditor(LittleSynthProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      mainPanel_(p.getAPVTS(), p)
{
    setSize(1200, 700);
    setResizable(true, true);
    setResizeLimits(1000, 600, 1600, 900);

    addAndMakeVisible(mainPanel_);

    // Timer to push GUI state (mod matrix, effects) to processor
    startTimerHz(30);
}

LittleSynthEditor::~LittleSynthEditor()
{
    stopTimer();
}

void LittleSynthEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(CustomLookAndFeel::kBackground));
}

void LittleSynthEditor::resized()
{
    mainPanel_.setBounds(getLocalBounds());
}

void LittleSynthEditor::timerCallback()
{
    // Push GUI state to processor
    mainPanel_.pushStateToProcessor();

    // Push audio data to visualizer from the processor's output FIFO
    auto* visualizer = mainPanel_.getVisualizer();
    if (visualizer != nullptr)
    {
        constexpr int kChunkSize = 256;
        float temp[kChunkSize];
        int numRead = processorRef.readOutputSamples(temp, kChunkSize);
        for (int i = 0; i < numRead; ++i)
            visualizer->pushSample(temp[i]);
    }
}
