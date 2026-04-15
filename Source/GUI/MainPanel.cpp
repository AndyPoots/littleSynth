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
    synthNameLabel_.setFont(juce::Font(28.0f, juce::Font::bold | juce::Font::italic));
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
    masterLevelLabel_.setFont(juce::Font(11.0f, juce::Font::bold));
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

    // Top bar: 48px
    auto topBar = bounds.removeFromTop(48);
    synthNameLabel_.setBounds(topBar.removeFromLeft(260).reduced(4));
    auto masterArea = topBar.removeFromRight(140);
    masterLevelLabel_.setBounds(masterArea.getX(), masterArea.getY(), 50, 36);
    masterLevelSlider_.setBounds(masterArea.getX() + 50, masterArea.getY() + 6, 80, 24);

    // Left column: oscillators (~280px wide)
    auto leftCol = bounds.removeFromLeft(280);

    // Far right column: effects (vertical stack)
    auto effectsCol = bounds.removeFromRight(280);
    effectsPanel_->setBounds(effectsCol.reduced(4));

    // Env/LFO column (beside center)
    auto envCol = bounds.removeFromRight(220);

    // Visualizer fills center as background
    visualizer_.setBounds(bounds);

    const int panelPad = 8;
    const int outerPad = 6;

    // Left column: oscillators
    const int oscH = (leftCol.getHeight() - outerPad * 2 - panelPad * 2) / 3;
    for (int i = 0; i < 3; ++i)
    {
        oscPanels_[static_cast<size_t>(i)]->setBounds(
            leftCol.getX() + outerPad,
            leftCol.getY() + outerPad + i * (oscH + panelPad),
            leftCol.getWidth() - outerPad * 2,
            oscH);
    }

    // Env/LFO column
    const int envH = (envCol.getHeight() - outerPad * 2 - panelPad * 4) / 5;
    for (int i = 0; i < 3; ++i)
    {
        envPanels_[static_cast<size_t>(i)]->setBounds(
            envCol.getX() + outerPad,
            envCol.getY() + outerPad + i * (envH + panelPad),
            envCol.getWidth() - outerPad * 2,
            envH);
    }
    for (int i = 0; i < 2; ++i)
    {
        lfoPanels_[static_cast<size_t>(i)]->setBounds(
            envCol.getX() + outerPad,
            envCol.getY() + outerPad + (3 + i) * (envH + panelPad),
            envCol.getWidth() - outerPad * 2,
            envH);
    }

    // Center column: Visualizer (top) + Filter (middle) + ModMatrix (bottom)
    auto centerCol = bounds;
    const int vizH = juce::jmin(120, centerCol.getHeight() / 4);
    visualizer_.setBounds(centerCol.getX() + outerPad, centerCol.getY() + outerPad,
                          centerCol.getWidth() - outerPad * 2, vizH);

    const int filterH = juce::jmin(190, (centerCol.getHeight() - vizH - panelPad) / 2);
    filterPanel_->setBounds(centerCol.getX() + outerPad,
                            centerCol.getY() + outerPad + vizH + panelPad,
                            centerCol.getWidth() - outerPad * 2, filterH);
    modMatrixPanel_->setBounds(centerCol.getX() + outerPad,
                               centerCol.getY() + outerPad + vizH + panelPad + filterH + panelPad,
                               centerCol.getWidth() - outerPad * 2,
                               centerCol.getHeight() - vizH - filterH - panelPad * 2 - outerPad * 2);
}
