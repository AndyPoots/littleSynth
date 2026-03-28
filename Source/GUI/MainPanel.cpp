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

    // Top bar: 30px
    auto topBar = bounds.removeFromTop(30);
    synthNameLabel_.setBounds(topBar.removeFromLeft(120).reduced(4));
    auto masterArea = topBar.removeFromRight(120);
    masterLevelLabel_.setBounds(masterArea.getX(), masterArea.getY(), 50, 30);
    masterLevelSlider_.setBounds(masterArea.getX() + 50, masterArea.getY() + 4, 70, 22);

    // Bottom strip: effects 130px
    auto bottomStrip = bounds.removeFromBottom(130);
    effectsPanel_->setBounds(bottomStrip);

    // Visualizer fills remaining as background
    visualizer_.setBounds(bounds);

    // Now overlay panels on top

    // Left column: oscillators (~260px wide)
    auto leftCol = bounds.removeFromLeft(260);
    const int oscH = leftCol.getHeight() / 3;
    for (int i = 0; i < 3; ++i)
    {
        oscPanels_[static_cast<size_t>(i)]->setBounds(
            leftCol.getX() + 4, leftCol.getY() + i * oscH + 2, leftCol.getWidth() - 8, oscH - 4);
    }

    // Right column: envelopes + LFOs (~200px wide)
    auto rightCol = bounds.removeFromRight(200);
    const int envH = (rightCol.getHeight() - 20) / 5; // 3 envs + 2 LFOs
    for (int i = 0; i < 3; ++i)
    {
        envPanels_[static_cast<size_t>(i)]->setBounds(
            rightCol.getX() + 4, rightCol.getY() + i * envH + 2, rightCol.getWidth() - 8, envH - 4);
    }
    for (int i = 0; i < 2; ++i)
    {
        lfoPanels_[static_cast<size_t>(i)]->setBounds(
            rightCol.getX() + 4, rightCol.getY() + (3 + i) * envH + 2, rightCol.getWidth() - 8, envH - 4);
    }

    // Center column: Filter (top) + ModMatrix (bottom)
    auto centerCol = bounds.reduced(2);
    const int filterH = juce::jmin(170, centerCol.getHeight() / 3);
    filterPanel_->setBounds(centerCol.getX() + 4, centerCol.getY(), centerCol.getWidth() - 8, filterH);
    modMatrixPanel_->setBounds(centerCol.getX() + 4, centerCol.getY() + filterH + 4,
                                centerCol.getWidth() - 8, centerCol.getHeight() - filterH - 4);
}
