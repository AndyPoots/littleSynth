// Source/PresetManager.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct PresetInfo
{
    juce::String name;
    juce::String category;    // "Factory", "Pads", "Bass", etc.
    bool isFactory = false;
    int factoryIndex = -1;    // -1 for user presets
    juce::File file;          // empty for factory presets
};

class PresetManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    PresetManager(juce::AudioProcessorValueTreeState& apvts,
                  const juce::Array<juce::ValueTree>& factoryPresets,
                  const juce::StringArray& factoryNames);

    ~PresetManager() override;

    // Rescan factory + user presets from disk
    void refreshPresets();

    // Access all presets
    const juce::Array<PresetInfo>& getAllPresets() const { return allPresets_; }
    juce::StringArray getCategories() const;

    // Current state
    int getCurrentPresetIndex() const { return currentIndex_; }
    bool isDirty() const { return dirty_; }
    void markDirty() { dirty_ = true; }

    // Navigation
    void selectPreset(int index);
    void selectPrev();
    void selectNext();

    // Save current state to disk
    void savePreset(const juce::String& name, const juce::String& category);

    // Delete a user preset
    bool deletePreset(int index);

    // Get the user preset root (creates if needed)
    static juce::File getUserPresetDirectory();

    // Default category names
    static const char* getDefaultCategories();
    static int getNumDefaultCategories();

    // Listener interface for UI updates
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void presetChanged(const juce::String& name) = 0;
        virtual void presetDirtyChanged(bool isDirty) = 0;
        virtual void presetListChanged() = 0;
    };

    void addListener(Listener* l) { listeners_.add(l); }
    void removeListener(Listener* l) { listeners_.remove(l); }

private:
    // AudioProcessorValueTreeState::Listener
    void parameterChanged(const juce::String& parameterID, float newValue) override;

    void loadPresetIntoAPVTS(const PresetInfo& preset);
    void loadFactoryPreset(int factoryIndex);
    void loadUserPreset(const juce::File& file);
    void scanUserPresets();
    void ensureDefaultLibraryGenerated();
    juce::String getCurrentPresetName() const;

    juce::AudioProcessorValueTreeState& apvts_;
    const juce::Array<juce::ValueTree>& factoryPresets_;
    const juce::StringArray& factoryNames_;

    juce::Array<PresetInfo> allPresets_;
    int currentIndex_ = 0;
    bool dirty_ = false;

    juce::ListenerList<Listener> listeners_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};
