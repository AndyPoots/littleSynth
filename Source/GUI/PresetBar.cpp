// Source/GUI/PresetBar.cpp
#include "PresetBar.h"

// ---------------------------------------------------------------
// Save dialog — self-contained modal component
// ---------------------------------------------------------------

class SavePresetDialog : public juce::Component
{
public:
    SavePresetDialog(PresetManager& pm, const juce::String& currentName)
        : presetManager_(pm)
    {
        setName("Save Preset");

        addAndMakeVisible(nameEditor_);
        nameEditor_.setText(currentName);
        nameEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        nameEditor_.setColour(juce::TextEditor::textColourId, juce::Colour(CustomLookAndFeel::kText));
        nameEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(CustomLookAndFeel::kAccent));
        nameEditor_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(CustomLookAndFeel::kAccent));
        nameEditor_.setJustification(juce::Justification::centred);
        nameEditor_.setFont(juce::FontOptions(14.0f));

        addAndMakeVisible(categoryCombo_);
        static const char* catNames[] = { "Pads", "Bass", "Lead", "Keys", "Organ", "FX", "Ambient" };
        for (int i = 0; i < 7; ++i)
            categoryCombo_.addItem(catNames[i], i + 1);
        categoryCombo_.setSelectedItemIndex(0);
        categoryCombo_.setColour(juce::ComboBox::backgroundColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        categoryCombo_.setColour(juce::ComboBox::textColourId, juce::Colour(CustomLookAndFeel::kText));
        categoryCombo_.setColour(juce::ComboBox::outlineColourId, juce::Colour(CustomLookAndFeel::kAccent));

        addAndMakeVisible(saveButton_);
        saveButton_.setButtonText("Save");
        saveButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kAccent));
        saveButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));
        saveButton_.onClick = [this]() { doSave(); };

        addAndMakeVisible(cancelButton_);
        cancelButton_.setButtonText("Cancel");
        cancelButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
        cancelButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kTextDim));
        cancelButton_.onClick = [this]() { exitModal(); };
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(12);

        nameEditor_.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(8);
        categoryCombo_.setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(8);

        auto btnRow = bounds.removeFromTop(28);
        cancelButton_.setBounds(btnRow.removeFromRight(70).reduced(2));
        saveButton_.setBounds(btnRow.removeFromRight(70).reduced(2));
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(CustomLookAndFeel::kPanelBg));
        g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 1.0f);
    }

private:
    void doSave()
    {
        auto name = nameEditor_.getText().trim();
        auto category = categoryCombo_.getText();

        if (name.isNotEmpty())
        {
            presetManager_.savePreset(name, category);
            exitModal();
        }
    }

    void exitModal()
    {
        if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
            dw->exitModalState(0);
    }

    PresetManager& presetManager_;
    juce::TextEditor nameEditor_;
    juce::ComboBox categoryCombo_;
    juce::TextButton saveButton_;
    juce::TextButton cancelButton_;
};

// ---------------------------------------------------------------
// PresetBar
// ---------------------------------------------------------------

PresetBar::PresetBar(PresetManager& presetManager)
    : presetManager_(presetManager)
{
    // Prev button — pink accent background
    addAndMakeVisible(prevButton_);
    prevButton_.setButtonText("\xe2\x97\x80");  // UTF-8 for ◀
    prevButton_.addListener(this);
    prevButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kAccent));
    prevButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));
    prevButton_.setColour(juce::TextButton::textColourOnId, juce::Colour(CustomLookAndFeel::kText));

    // Next button — pink accent background
    addAndMakeVisible(nextButton_);
    nextButton_.setButtonText("\xe2\x96\xb6");  // UTF-8 for ▶
    nextButton_.addListener(this);
    nextButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kAccent));
    nextButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));
    nextButton_.setColour(juce::TextButton::textColourOnId, juce::Colour(CustomLookAndFeel::kText));

    // Preset name button (opens browser) — dark panel with pink border
    addAndMakeVisible(presetNameButton_);
    presetNameButton_.addListener(this);
    presetNameButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kPanelBg));
    presetNameButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));
    presetNameButton_.setColour(juce::TextButton::textColourOnId, juce::Colour(CustomLookAndFeel::kAccent));

    // Save button — pink accent background
    addAndMakeVisible(saveButton_);
    saveButton_.setButtonText("Save");
    saveButton_.addListener(this);
    saveButton_.setColour(juce::TextButton::buttonColourId, juce::Colour(CustomLookAndFeel::kAccent));
    saveButton_.setColour(juce::TextButton::textColourOffId, juce::Colour(CustomLookAndFeel::kText));
    saveButton_.setColour(juce::TextButton::textColourOnId, juce::Colour(CustomLookAndFeel::kText));

    presetManager_.addListener(this);
    updatePresetNameDisplay();
}

PresetBar::~PresetBar()
{
    presetManager_.removeListener(this);
}

void PresetBar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(CustomLookAndFeel::kBackground));

    // Draw a rounded border around the preset name button area in accent pink
    auto nameBounds = presetNameButton_.getBounds().expanded(1);
    g.setColour(juce::Colour(CustomLookAndFeel::kAccent));
    g.drawRoundedRectangle(nameBounds.toFloat(), 4.0f, 1.0f);
}

void PresetBar::resized()
{
    auto bounds = getLocalBounds();

    // Layout: [◀ 28px] [▶ 28px] [preset name - fills] [Save 54px]
    prevButton_.setBounds(bounds.removeFromLeft(28));
    nextButton_.setBounds(bounds.removeFromLeft(28));
    saveButton_.setBounds(bounds.removeFromRight(54));
    presetNameButton_.setBounds(bounds.reduced(4, 2));
}

void PresetBar::presetChanged(const juce::String& /*name*/)
{
    updatePresetNameDisplay();
}

void PresetBar::presetDirtyChanged(bool /*isDirty*/)
{
    updatePresetNameDisplay();
}

void PresetBar::presetListChanged()
{
    updatePresetNameDisplay();
}

void PresetBar::buttonClicked(juce::Button* button)
{
    if (button == &prevButton_)
    {
        presetManager_.selectPrev();
    }
    else if (button == &nextButton_)
    {
        presetManager_.selectNext();
    }
    else if (button == &presetNameButton_)
    {
        // Notify parent to toggle the preset browser overlay
        if (onOpenBrowser)
            onOpenBrowser();
    }
    else if (button == &saveButton_)
    {
        showSaveDialog();
    }
}

void PresetBar::updatePresetNameDisplay()
{
    auto& presets = presetManager_.getAllPresets();
    int idx = presetManager_.getCurrentPresetIndex();

    juce::String name;
    if (idx >= 0 && idx < presets.size())
        name = presets.getReference(idx).name;
    else
        name = "No Preset";

    if (presetManager_.isDirty())
        name += " *";

    presetNameButton_.setButtonText(name);
}

void PresetBar::showSaveDialog()
{
    auto* topLevel = getTopLevelComponent();
    if (topLevel == nullptr)
        return;

    // Get current preset name for pre-fill
    auto& presets = presetManager_.getAllPresets();
    int idx = presetManager_.getCurrentPresetIndex();
    juce::String currentName;
    if (idx >= 0 && idx < presets.size())
        currentName = presets.getReference(idx).name;

    auto* content = new SavePresetDialog(presetManager_, currentName);

    auto* dw = new juce::DialogWindow("Save Preset",
                                       juce::Colour(CustomLookAndFeel::kBackground),
                                       true, true);
    dw->setContentOwned(content, true);
    dw->setResizable(false, false);
    dw->setSize(300, 140);
    dw->centreAroundComponent(topLevel, dw->getWidth(), dw->getHeight());
    dw->setVisible(true);
    dw->enterModalState(true, nullptr, true);
}
