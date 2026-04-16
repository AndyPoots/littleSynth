// Source/GUI/PresetBar.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GUI/CustomLookAndFeel.h"
#include "PresetManager.h"

class PresetBar : public juce::Component,
                  public PresetManager::Listener,
                  public juce::Button::Listener
{
public:
    PresetBar(PresetManager& presetManager);
    ~PresetBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // PresetManager::Listener
    void presetChanged(const juce::String& name) override;
    void presetDirtyChanged(bool isDirty) override;
    void presetListChanged() override;

    // Button::Listener
    void buttonClicked(juce::Button* button) override;

    /// Callback fired when the preset name button is clicked (open browser).
    std::function<void()> onOpenBrowser;

private:
    void updatePresetNameDisplay();
    void showSaveDialog();

    PresetManager& presetManager_;

    juce::TextButton prevButton_;
    juce::TextButton nextButton_;
    juce::TextButton presetNameButton_;
    juce::TextButton saveButton_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBar)
};
