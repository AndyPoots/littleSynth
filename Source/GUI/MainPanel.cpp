// Source/GUI/MainPanel.cpp
#include "MainPanel.h"
#include "PluginProcessor.h"

MainPanel::MainPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor)
    : processor_(processor), apvts_(apvts)
{
    setLookAndFeel(&lookAndFeel_);

    // Visualizer
    addAndMakeVisible(visualizer_);

    // Synth name
    addAndMakeVisible(synthNameLabel_);
    synthNameLabel_.setText("littleSynth", juce::dontSendNotification);
    synthNameLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    synthNameLabel_.setFont(juce::Font(16.0f, juce::Font::bold));
    synthNameLabel_.setJustificationType(juce::Justification::centredLeft);

    // Master level
    addAndMakeVisible(masterLevelSlider_);
    masterLevelSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    masterLevelSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterLevelAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, "master_level", masterLevelSlider_);
    addAndMakeVisible(masterLevelLabel_);
    masterLevelLabel_.setText("Master", juce::dontSendNotification);
    masterLevelLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
    masterLevelLabel_.setFont(9.0f);
    masterLevelLabel_.setJustificationType(juce::Justification::centred);

    // Oscillator panels
    for (int i = 0; i < 3; ++i)
    {
        oscPanels_[static_cast<size_t>(i)] = std::make_unique<OscillatorPanel>(apvts, i);
        addAndMakeVisible(*oscPanels_[static_cast<size_t>(i)]);
    }

    // Filter panel
    filterPanel_ = std::make_unique<FilterPanel>(apvts);
    addAndMakeVisible(*filterPanel_);

    // Mod matrix panel
    modMatrixPanel_ = std::make_unique<ModMatrixPanel>(apvts, processor);
    addAndMakeVisible(*modMatrixPanel_);

    // Envelope panels
    const char* envPrefixes[] = { "amp_env_", "filter_env_", "mod_env_" };
    const char* envTitles[] = { "AMP ENV", "FILTER ENV", "MOD ENV" };
    for (int i = 0; i < 3; ++i)
    {
        envPanels_[static_cast<size_t>(i)] = std::make_unique<EnvelopePanel>(
            apvts, envPrefixes[i], envTitles[i]);
        addAndMakeVisible(*envPanels_[static_cast<size_t>(i)]);
    }

    // LFO panels
    for (int i = 0; i < 2; ++i)
    {
        lfoPanels_[static_cast<size_t>(i)] = std::make_unique<LFOPanel>(apvts, i);
        addAndMakeVisible(*lfoPanels_[static_cast<size_t>(i)]);
    }

    // Effects panel
    effectsPanel_ = std::make_unique<EffectsPanel>(apvts, processor);
    addAndMakeVisible(*effectsPanel_);
}

void MainPanel::pushStateToProcessor()
{
    modMatrixPanel_->updateProcessorState();
    effectsPanel_->updateProcessorState();
}

void MainPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(CustomLookAndFeel::kBackground));
}

void MainPanel::resized()
{
    auto bounds = getLocalBounds();

    // Outer padding — breathing room around everything
    bounds.reduce(10, 10);

    // Top bar: 36px
    auto topBar = bounds.removeFromTop(36);
    synthNameLabel_.setBounds(topBar.removeFromLeft(140).reduced(4));
    auto masterArea = topBar.removeFromRight(140);
    masterLevelLabel_.setBounds(masterArea.getX(), masterArea.getY(), 50, 36);
    masterLevelSlider_.setBounds(masterArea.getX() + 50, masterArea.getY() + 6, 80, 24);

    // Bottom strip: effects 140px + padding
    auto bottomStrip = bounds.removeFromBottom(140);
    effectsPanel_->setBounds(bottomStrip.reduced(4));

    // Visualizer fills remaining as background
    visualizer_.setBounds(bounds);

    // Now overlay panels on top — each column has generous inner padding

    const int panelPad = 8;  // padding between sibling panels
    const int outerPad = 6; // padding inside column edges

    // Left column: oscillators (~280px wide)
    auto leftCol = bounds.removeFromLeft(280);
    const int oscH = (leftCol.getHeight() - outerPad * 2 - panelPad * 2) / 3;
    for (int i = 0; i < 3; ++i)
    {
        oscPanels_[static_cast<size_t>(i)]->setBounds(
            leftCol.getX() + outerPad,
            leftCol.getY() + outerPad + i * (oscH + panelPad),
            leftCol.getWidth() - outerPad * 2,
            oscH);
    }

    // Right column: envelopes + LFOs (~220px wide)
    auto rightCol = bounds.removeFromRight(220);
    const int envH = (rightCol.getHeight() - outerPad * 2 - panelPad * 4) / 5; // 3 envs + 2 LFOs
    for (int i = 0; i < 3; ++i)
    {
        envPanels_[static_cast<size_t>(i)]->setBounds(
            rightCol.getX() + outerPad,
            rightCol.getY() + outerPad + i * (envH + panelPad),
            rightCol.getWidth() - outerPad * 2,
            envH);
    }
    for (int i = 0; i < 2; ++i)
    {
        lfoPanels_[static_cast<size_t>(i)]->setBounds(
            rightCol.getX() + outerPad,
            rightCol.getY() + outerPad + (3 + i) * (envH + panelPad),
            rightCol.getWidth() - outerPad * 2,
            envH);
    }

    // Center column: Filter (top) + ModMatrix (bottom)
    auto centerCol = bounds.reduced(outerPad);
    const int filterH = juce::jmin(190, centerCol.getHeight() / 3);
    filterPanel_->setBounds(centerCol.getX() + outerPad, centerCol.getY() + outerPad,
                            centerCol.getWidth() - outerPad * 2, filterH);
    modMatrixPanel_->setBounds(centerCol.getX() + outerPad,
                               centerCol.getY() + filterH + outerPad + panelPad,
                               centerCol.getWidth() - outerPad * 2,
                               centerCol.getHeight() - filterH - outerPad - panelPad - outerPad);
}
