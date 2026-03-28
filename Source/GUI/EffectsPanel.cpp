// Source/GUI/EffectsPanel.cpp
#include "EffectsPanel.h"
#include "CustomLookAndFeel.h"
#include "../PluginProcessor.h"

EffectsPanel::EffectsPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor)
    : apvts_(apvts), processor_(processor)
{
    // Title
    addAndMakeVisible(titleLabel_);
    titleLabel_.setText("EFFECTS", juce::dontSendNotification);
    titleLabel_.setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kAccent));
    titleLabel_.setFont(juce::Font(12.0f, juce::Font::bold));
    titleLabel_.setJustificationType(juce::Justification::centred);

    createEQSection();           // 0
    createCompressorSection();   // 1
    createChorusSection();       // 2
    createPhaserSection();       // 3
    createFlangerSection();      // 4
    createDelaySection();        // 5
    createReverbSection();       // 6
    createDistortionSection();   // 7
}

void EffectsPanel::createEQSection()
{
    auto& s = sections_[0];
    s.name = "EQ";

    s.enableButton = std::make_unique<juce::ToggleButton>("EQ");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("LoF"); addKnob("LoG"); addKnob("MidF");
    addKnob("MidG"); addKnob("HiF"); addKnob("HiG");

    // Set ranges for EQ knobs
    s.sliders[0]->setRange(20.0, 500.0, 1.0);    // Lo Freq
    s.sliders[0]->setValue(200.0);
    s.sliders[1]->setRange(-12.0, 12.0, 0.1);    // Lo Gain
    s.sliders[1]->setValue(0.0);
    s.sliders[2]->setRange(200.0, 5000.0, 1.0);  // Mid Freq
    s.sliders[2]->setValue(1000.0);
    s.sliders[3]->setRange(-12.0, 12.0, 0.1);    // Mid Gain
    s.sliders[3]->setValue(0.0);
    s.sliders[4]->setRange(2000.0, 16000.0, 1.0); // Hi Freq
    s.sliders[4]->setValue(5000.0);
    s.sliders[5]->setRange(-12.0, 12.0, 0.1);    // Hi Gain
    s.sliders[5]->setValue(0.0);
}

void EffectsPanel::createCompressorSection()
{
    auto& s = sections_[1];
    s.name = "Comp";

    s.enableButton = std::make_unique<juce::ToggleButton>("Comp");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Thresh"); addKnob("Ratio"); addKnob("Atk");
    addKnob("Rel"); addKnob("Gain");

    s.sliders[0]->setRange(-60.0, 0.0, 0.1); s.sliders[0]->setValue(-20.0);
    s.sliders[1]->setRange(1.0, 20.0, 0.1);  s.sliders[1]->setValue(4.0);
    s.sliders[2]->setRange(0.1, 50.0, 0.1);  s.sliders[2]->setValue(10.0);
    s.sliders[3]->setRange(10.0, 500.0, 1.0); s.sliders[3]->setValue(100.0);
    s.sliders[4]->setRange(-12.0, 12.0, 0.1); s.sliders[4]->setValue(0.0);
}

void EffectsPanel::createChorusSection()
{
    auto& s = sections_[2];
    s.name = "Chorus";

    s.enableButton = std::make_unique<juce::ToggleButton>("Chorus");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Rate"); addKnob("Depth"); addKnob("Mix");

    s.sliders[0]->setRange(0.1, 5.0, 0.01);  s.sliders[0]->setValue(1.0);
    s.sliders[1]->setRange(0.0, 1.0, 0.01);  s.sliders[1]->setValue(0.5);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);  s.sliders[2]->setValue(0.3);
}

void EffectsPanel::createPhaserSection()
{
    auto& s = sections_[3];
    s.name = "Phaser";

    s.enableButton = std::make_unique<juce::ToggleButton>("Phaser");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Rate"); addKnob("Depth"); addKnob("FB"); addKnob("Mix");

    s.sliders[0]->setRange(0.1, 10.0, 0.01); s.sliders[0]->setValue(1.0);
    s.sliders[1]->setRange(0.0, 1.0, 0.01);  s.sliders[1]->setValue(0.5);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);  s.sliders[2]->setValue(0.3);
    s.sliders[3]->setRange(0.0, 1.0, 0.01);  s.sliders[3]->setValue(0.5);
}

void EffectsPanel::createFlangerSection()
{
    auto& s = sections_[4];
    s.name = "Flanger";

    s.enableButton = std::make_unique<juce::ToggleButton>("Flanger");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Rate"); addKnob("Depth"); addKnob("FB"); addKnob("Mix");

    s.sliders[0]->setRange(0.05, 5.0, 0.01); s.sliders[0]->setValue(0.5);
    s.sliders[1]->setRange(0.0, 1.0, 0.01);  s.sliders[1]->setValue(0.5);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);  s.sliders[2]->setValue(0.3);
    s.sliders[3]->setRange(0.0, 1.0, 0.01);  s.sliders[3]->setValue(0.5);
}

void EffectsPanel::createDelaySection()
{
    auto& s = sections_[5];
    s.name = "Delay";

    s.enableButton = std::make_unique<juce::ToggleButton>("Delay");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Time"); addKnob("FB"); addKnob("Mix");

    s.sliders[0]->setRange(10.0, 1000.0, 1.0); s.sliders[0]->setValue(250.0);
    s.sliders[1]->setRange(0.0, 0.95, 0.01);   s.sliders[1]->setValue(0.4);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);    s.sliders[2]->setValue(0.3);
}

void EffectsPanel::createReverbSection()
{
    auto& s = sections_[6];
    s.name = "Reverb";

    s.enableButton = std::make_unique<juce::ToggleButton>("Reverb");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Room"); addKnob("Damp"); addKnob("Wet"); addKnob("PreDly");

    s.sliders[0]->setRange(0.0, 1.0, 0.01);   s.sliders[0]->setValue(0.5);
    s.sliders[1]->setRange(0.0, 1.0, 0.01);   s.sliders[1]->setValue(0.5);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);   s.sliders[2]->setValue(0.3);
    s.sliders[3]->setRange(0.0, 100.0, 1.0);  s.sliders[3]->setValue(0.0);
}

void EffectsPanel::createDistortionSection()
{
    auto& s = sections_[7];
    s.name = "Dist";

    s.enableButton = std::make_unique<juce::ToggleButton>("Dist");
    addAndMakeVisible(*s.enableButton);

    auto addKnob = [&](const juce::String& label) {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        addAndMakeVisible(*slider);
        auto lbl = std::make_unique<juce::Label>();
        lbl->setText(label, juce::dontSendNotification);
        lbl->setColour(juce::Label::textColourId, juce::Colour(CustomLookAndFeel::kTextDim));
        lbl->setFont(8.0f);
        lbl->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*lbl);
        s.sliders.push_back(std::move(slider));
        s.labels.push_back(std::move(lbl));
    };

    addKnob("Drive"); addKnob("Tone"); addKnob("Mix");

    s.sliders[0]->setRange(0.0, 1.0, 0.01);  s.sliders[0]->setValue(0.5);
    s.sliders[1]->setRange(0.0, 1.0, 0.01);  s.sliders[1]->setValue(0.5);
    s.sliders[2]->setRange(0.0, 1.0, 0.01);  s.sliders[2]->setValue(0.5);
}

void EffectsPanel::updateProcessorState()
{
    auto* chain = processor_.getEffectsChain();
    if (chain == nullptr) return;

    // 0: EQ
    {
        auto& s = sections_[0];
        auto* eq = chain->getEQ();
        eq->setEnabled(s.enableButton->getToggleState());
        eq->setLowFreq((float)s.sliders[0]->getValue());
        eq->setLowGain((float)s.sliders[1]->getValue());
        eq->setMidFreq((float)s.sliders[2]->getValue());
        eq->setMidGain((float)s.sliders[3]->getValue());
        eq->setHighFreq((float)s.sliders[4]->getValue());
        eq->setHighGain((float)s.sliders[5]->getValue());
    }

    // 1: Compressor
    {
        auto& s = sections_[1];
        auto* comp = chain->getCompressor();
        comp->setEnabled(s.enableButton->getToggleState());
        comp->setThreshold((float)s.sliders[0]->getValue());
        comp->setRatio((float)s.sliders[1]->getValue());
        comp->setAttack((float)s.sliders[2]->getValue());
        comp->setRelease((float)s.sliders[3]->getValue());
        comp->setMakeupGain((float)s.sliders[4]->getValue());
    }

    // 2: Chorus
    {
        auto& s = sections_[2];
        auto* fx = chain->getChorus();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setRate((float)s.sliders[0]->getValue());
        fx->setDepth((float)s.sliders[1]->getValue());
        fx->setMix((float)s.sliders[2]->getValue());
    }

    // 3: Phaser
    {
        auto& s = sections_[3];
        auto* fx = chain->getPhaser();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setRate((float)s.sliders[0]->getValue());
        fx->setDepth((float)s.sliders[1]->getValue());
        fx->setFeedback((float)s.sliders[2]->getValue());
        fx->setMix((float)s.sliders[3]->getValue());
    }

    // 4: Flanger
    {
        auto& s = sections_[4];
        auto* fx = chain->getFlanger();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setRate((float)s.sliders[0]->getValue());
        fx->setDepth((float)s.sliders[1]->getValue());
        fx->setFeedback((float)s.sliders[2]->getValue());
        fx->setMix((float)s.sliders[3]->getValue());
    }

    // 5: Delay
    {
        auto& s = sections_[5];
        auto* fx = chain->getDelay();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setTimeMs((float)s.sliders[0]->getValue());
        fx->setFeedback((float)s.sliders[1]->getValue());
        fx->setMix((float)s.sliders[2]->getValue());
    }

    // 6: Reverb
    {
        auto& s = sections_[6];
        auto* fx = chain->getReverb();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setRoomSize((float)s.sliders[0]->getValue());
        fx->setDamping((float)s.sliders[1]->getValue());
        fx->setWetDry((float)s.sliders[2]->getValue());
        fx->setPreDelay((float)s.sliders[3]->getValue());
    }

    // 7: Distortion
    {
        auto& s = sections_[7];
        auto* fx = chain->getDistortion();
        fx->setEnabled(s.enableButton->getToggleState());
        fx->setDrive((float)s.sliders[0]->getValue());
        fx->setTone((float)s.sliders[1]->getValue());
        fx->setMix((float)s.sliders[2]->getValue());
    }
}

void EffectsPanel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(CustomLookAndFeel::kPanelBg).withAlpha(0.85f));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccentBlue));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}

void EffectsPanel::resized()
{
    auto area = getLocalBounds().reduced(4);

    // Title
    titleLabel_.setBounds(area.removeFromTop(16));

    // Layout: 8 effect sections side by side
    const int sectionW = area.getWidth() / 8;
    const int knobSize = 32;
    const int labelH = 10;

    for (int i = 0; i < 8; ++i)
    {
        auto col = area.removeFromLeft(sectionW).reduced(2);
        auto& s = sections_[i];

        // Enable button at top
        s.enableButton->setBounds(col.getX(), col.getY(), col.getWidth(), 18);

        // Knobs
        int y = col.getY() + 22;
        int knobsPerRow = juce::jmax(1, col.getWidth() / (knobSize + 4));
        int knobIdx = 0;

        for (size_t k = 0; k < s.sliders.size(); ++k)
        {
            int colInRow = knobIdx % knobsPerRow;
            if (colInRow == 0 && knobIdx > 0)
                y += knobSize + labelH + 2;

            int kx = col.getX() + colInRow * ((col.getWidth()) / knobsPerRow);
            int kw = col.getWidth() / knobsPerRow;

            s.labels[k]->setBounds(kx, y, kw, labelH);
            s.sliders[k]->setBounds(kx + (kw - knobSize) / 2, y + labelH, knobSize, knobSize);
            knobIdx++;
        }
    }
}
