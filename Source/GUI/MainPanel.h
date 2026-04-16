// Source/GUI/MainPanel.h
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "GUI/CustomLookAndFeel.h"
#include "GUI/Visualizer.h"
#include "GUI/OscillatorPanel.h"
#include "GUI/FilterPanel.h"
#include "GUI/EnvelopePanel.h"
#include "GUI/LFOPanel.h"
#include "GUI/ModMatrixPanel.h"
#include "GUI/EffectsPanel.h"
#include "GUI/PresetBar.h"
#include "GUI/PresetBrowser.h"

class LittleSynthProcessor;

class MainPanel : public juce::Component
{
public:
    MainPanel(juce::AudioProcessorValueTreeState& apvts, LittleSynthProcessor& processor);
    ~MainPanel() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    Visualizer* getVisualizer() { return &visualizer_; }

    /// Push current state to processor (call from a timer or periodically)
    void pushStateToProcessor();

private:
    /// Toggle the preset browser overlay on/off.
    void togglePresetBrowser();

    /// Close the preset browser if it's open.
    void closePresetBrowser();

    /// Global mouse listener to close browser on outside clicks.
    void handleGlobalMouseDown(const juce::MouseEvent& e);

    CustomLookAndFeel lookAndFeel_;

    // Mouse listener that forwards all child mouseDown events to MainPanel
    struct GlobalMouseListener : public juce::MouseListener
    {
        MainPanel& owner;
        explicit GlobalMouseListener(MainPanel& o) : owner(o) {}
        void mouseDown(const juce::MouseEvent& e) override { owner.handleGlobalMouseDown(e); }
    };
    GlobalMouseListener globalMouseListener_;

    // Visualizer (background layer)
    Visualizer visualizer_;

    // Top bar
    juce::Label synthNameLabel_;
    std::unique_ptr<PresetBar> presetBar_;
    std::unique_ptr<PresetBrowser> presetBrowser_;
    juce::Slider masterLevelSlider_;
    juce::Label masterLevelLabel_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterLevelAttachment_;

    // Oscillators (left column)
    std::array<std::unique_ptr<OscillatorPanel>, 3> oscPanels_;

    // Center column
    std::unique_ptr<FilterPanel> filterPanel_;
    std::unique_ptr<ModMatrixPanel> modMatrixPanel_;

    // Right column
    std::array<std::unique_ptr<EnvelopePanel>, 3> envPanels_;
    std::array<std::unique_ptr<LFOPanel>, 2> lfoPanels_;

    // Bottom strip
    std::unique_ptr<EffectsPanel> effectsPanel_;

    LittleSynthProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPanel)
};
