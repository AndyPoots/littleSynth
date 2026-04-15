// Source/GUI/ModMatrixPanel.cpp
#include "ModMatrixPanel.h"
#include "CustomLookAndFeel.h"
#include "../PluginProcessor.h"
#include "../VoiceManager.h"
#include "../SynthVoice.h"

ModMatrixPanel::ModMatrixPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor)
    : apvts_(apvts), processor_(processor)
{
    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText("MOD MATRIX", juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centredLeft);

    // Column headers
    auto makeHeader = [this](juce::Label& label, const juce::String& text) {
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        label.setFont(juce::Font(14.0f, juce::Font::bold));
        label.setJustificationType(juce::Justification::centred);
    };
    makeHeader(sourceHeader_, "SRC");
    makeHeader(destHeader_, "DEST");
    makeHeader(depthHeader_, "DEPTH");
    makeHeader(bipolarHeader_, "BI");
    makeHeader(activeHeader_, "ON");

    auto sourceNames = getSourceNames();
    auto destNames = getDestNames();

    for (int i = 0; i < kNumSlots; ++i)
    {
        auto& slot = slots_[i];

        // Source combo
        slot.sourceCombo = std::make_unique<juce::ComboBox>();
        slot.sourceCombo->addItemList(sourceNames, 1);
        slot.sourceCombo->setSelectedItemIndex(0, juce::dontSendNotification);
        addAndMakeVisible(*slot.sourceCombo);

        // Dest combo
        slot.destCombo = std::make_unique<juce::ComboBox>();
        slot.destCombo->addItemList(destNames, 1);
        slot.destCombo->setSelectedItemIndex(6, juce::dontSendNotification); // FilterCutoff
        addAndMakeVisible(*slot.destCombo);

        // Depth slider
        slot.depthSlider = std::make_unique<juce::Slider>();
        slot.depthSlider->setSliderStyle(juce::Slider::LinearHorizontal);
        slot.depthSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slot.depthSlider->setRange(-1.0, 1.0, 0.01);
        slot.depthSlider->setValue(0.0);
        addAndMakeVisible(*slot.depthSlider);

        // Bipolar toggle
        slot.bipolarToggle = std::make_unique<juce::ToggleButton>();
        slot.bipolarToggle->setToggleState(true, juce::dontSendNotification);
        addAndMakeVisible(*slot.bipolarToggle);

        // Active toggle
        slot.activeToggle = std::make_unique<juce::ToggleButton>();
        slot.activeToggle->setToggleState(false, juce::dontSendNotification);
        addAndMakeVisible(*slot.activeToggle);
    }
}

juce::StringArray ModMatrixPanel::getSourceNames()
{
    return {
        "AmpEnv", "FilterEnv", "ModEnv", "LFO1", "LFO2",
        "Velocity", "ModWheel", "Aftertouch", "PitchBend"
    };
}

juce::StringArray ModMatrixPanel::getDestNames()
{
    return {
        "Osc1Pitch", "Osc2Pitch", "Osc3Pitch",
        "Osc1Level", "Osc2Level", "Osc3Level",
        "FilterCutoff", "FilterReso", "AmpLevel",
        "LFO1Rate", "LFO1Depth", "LFO2Rate", "LFO2Depth",
        "Pan"
    };
}

void ModMatrixPanel::updateProcessorState()
{
    auto& synth = processor_.getSynth();
    for (int v = 0; v < synth.getNumVoices(); ++v)
    {
        auto* voice = synth.getVoice(v);
        if (voice == nullptr) continue;

        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& slot = slots_[i];
            voice->setModMatrixSource(i, static_cast<ModMatrix::Source>(slot.sourceCombo->getSelectedItemIndex()));
            voice->setModMatrixDestination(i, static_cast<ModMatrix::Destination>(slot.destCombo->getSelectedItemIndex()));
            voice->setModMatrixDepth(i, (float)slot.depthSlider->getValue());
            voice->setModMatrixBipolar(i, slot.bipolarToggle->getToggleState());
            voice->setModMatrixActive(i, slot.activeToggle->getToggleState());
        }
    }
}

void ModMatrixPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
}

void ModMatrixPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Title
    titleLabel_.setBounds(area.removeFromTop(16));
    area.removeFromTop(3);

    // Column layout:
    // Source(100) | Dest(90) | Depth(flex) | Bipolar(30) | Active(30)
    const int srcW = 95;
    const int destW = 85;
    const int depthW = 60;
    const int biW = 28;
    const int actW = 28;

    // Headers
    auto headerRow = area.removeFromTop(14);
    int hx = headerRow.getX();
    sourceHeader_.setBounds(hx, headerRow.getY(), srcW, 14);
    hx += srcW;
    destHeader_.setBounds(hx, headerRow.getY(), destW, 14);
    hx += destW;
    depthHeader_.setBounds(hx, headerRow.getY(), depthW, 14);
    hx += depthW;
    bipolarHeader_.setBounds(hx, headerRow.getY(), biW, 14);
    hx += biW;
    activeHeader_.setBounds(hx, headerRow.getY(), actW, 14);

    // Rows
    const int rowH = 18;
    for (int i = 0; i < kNumSlots; ++i)
    {
        auto row = area.removeFromTop(rowH);
        int rx = row.getX();

        slots_[i].sourceCombo->setBounds(rx, row.getY(), srcW, rowH);
        rx += srcW;

        slots_[i].destCombo->setBounds(rx, row.getY(), destW, rowH);
        rx += destW;

        slots_[i].depthSlider->setBounds(rx, row.getY(), depthW, rowH);
        rx += depthW;

        slots_[i].bipolarToggle->setBounds(rx, row.getY(), biW, rowH);
        rx += biW;

        slots_[i].activeToggle->setBounds(rx, row.getY(), actW, rowH);
    }
}
