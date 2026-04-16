// Source/GUI/PresetBrowser.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GUI/CustomLookAndFeel.h"
#include "PresetManager.h"

class PresetBrowser : public juce::Component
{
public:
    PresetBrowser(PresetManager& presetManager);
    ~PresetBrowser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /// Dismiss the browser (hides it).
    void dismiss();

private:
    class ContentComponent;

    ContentComponent* content_ = nullptr;
    PresetManager& presetManager_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetBrowser)
};
