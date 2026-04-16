// Source/PresetManager.cpp
#include "PresetManager.h"

static const char* kDefaultCategoryNames[] = {
    "Pads", "Bass", "Lead", "Keys", "Organ", "FX", "Ambient"
};

const char* PresetManager::getDefaultCategories()
{
    // Returns first entry; caller uses getNumDefaultCategories() + indexing
    return kDefaultCategoryNames[0];
}

int PresetManager::getNumDefaultCategories()
{
    return sizeof(kDefaultCategoryNames) / sizeof(kDefaultCategoryNames[0]);
}

// ---------------------------------------------------------------
// Construction
// ---------------------------------------------------------------

PresetManager::PresetManager(juce::AudioProcessorValueTreeState& apvts,
                             const juce::Array<juce::ValueTree>& factoryPresets,
                             const juce::StringArray& factoryNames)
    : apvts_(apvts)
    , factoryPresets_(factoryPresets)
    , factoryNames_(factoryNames)
{
    refreshPresets();

    // Listen to all parameter changes for dirty tracking
    for (auto* param : apvts_.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            apvts_.addParameterListener(paramWithID->paramID, this);
    }
}

PresetManager::~PresetManager()
{
    for (auto* param : apvts_.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            apvts_.removeParameterListener(paramWithID->paramID, this);
    }
}

// ---------------------------------------------------------------
// Preset scanning
// ---------------------------------------------------------------

void PresetManager::refreshPresets()
{
    allPresets_.clear();

    // Add factory presets
    for (int i = 0; i < factoryPresets_.size(); ++i)
    {
        PresetInfo info;
        info.name = factoryNames_[i];
        info.category = factoryPresets_[i].getProperty("category", "Factory").toString();
        info.isFactory = true;
        info.factoryIndex = i;
        allPresets_.add(info);
    }

    // Scan user presets from disk
    ensureDefaultLibraryGenerated();
    scanUserPresets();

    // Default to first preset (Init)
    if (currentIndex_ >= allPresets_.size())
        currentIndex_ = 0;

    listeners_.call(&Listener::presetListChanged);
}

juce::StringArray PresetManager::getCategories() const
{
    juce::StringArray cats;
    cats.add("All");

    // Add all categories found in presets (factory and user)
    for (const auto& preset : allPresets_)
    {
        if (!cats.contains(preset.category))
            cats.add(preset.category);
    }

    return cats;
}

void PresetManager::scanUserPresets()
{
    auto presetDir = getUserPresetDirectory();
    if (!presetDir.isDirectory())
        return;

    for (auto& categoryDir : presetDir.findChildFiles(juce::File::findDirectories, false))
    {
        auto categoryName = categoryDir.getFileName();
        for (auto& presetFile : categoryDir.findChildFiles(juce::File::findFiles, false, "*.preset"))
        {
            PresetInfo info;
            info.name = presetFile.getFileNameWithoutExtension();
            info.category = categoryName;
            info.isFactory = false;
            info.factoryIndex = -1;
            info.file = presetFile;
            allPresets_.add(info);
        }
    }
}

void PresetManager::ensureDefaultLibraryGenerated()
{
    auto presetDir = getUserPresetDirectory();
    presetDir.createDirectory();

    // Create category subdirectories if they don't exist yet
    for (int i = 0; i < getNumDefaultCategories(); ++i)
        presetDir.getChildFile(kDefaultCategoryNames[i]).createDirectory();
}

// ---------------------------------------------------------------
// User preset directory
// ---------------------------------------------------------------

juce::File PresetManager::getUserPresetDirectory()
{
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("littleSynth")
                   .getChildFile("Presets");
    dir.createDirectory();
    return dir;
}

// ---------------------------------------------------------------
// Navigation
// ---------------------------------------------------------------

void PresetManager::selectPreset(int index)
{
    if (index < 0 || index >= allPresets_.size())
        return;

    currentIndex_ = index;
    dirty_ = false;
    loadPresetIntoAPVTS(allPresets_.getReference(index));

    listeners_.call(&Listener::presetChanged, allPresets_.getReference(index).name);
    listeners_.call(&Listener::presetDirtyChanged, false);
}

void PresetManager::selectPrev()
{
    if (allPresets_.isEmpty())
        return;

    int newIndex = (currentIndex_ - 1 + allPresets_.size()) % allPresets_.size();
    selectPreset(newIndex);
}

void PresetManager::selectNext()
{
    if (allPresets_.isEmpty())
        return;

    int newIndex = (currentIndex_ + 1) % allPresets_.size();
    selectPreset(newIndex);
}

// ---------------------------------------------------------------
// Loading presets into APVTS
// ---------------------------------------------------------------

void PresetManager::loadPresetIntoAPVTS(const PresetInfo& preset)
{
    if (preset.isFactory)
    {
        loadFactoryPreset(preset.factoryIndex);
    }
    else
    {
        loadUserPreset(preset.file);
    }
}

void PresetManager::loadFactoryPreset(int factoryIndex)
{
    if (factoryIndex < 0 || factoryIndex >= factoryPresets_.size())
        return;

    const auto& preset = factoryPresets_[factoryIndex];
    for (int i = 0; i < preset.getNumProperties(); ++i)
    {
        auto propID = preset.getPropertyName(i);
        if (propID.toString() == "name" || propID.toString() == "category")
            continue;

        auto paramID = propID.toString();
        if (auto* param = apvts_.getParameter(paramID))
        {
            auto value = static_cast<float>(preset.getProperty(propID));
            auto range = param->getNormalisableRange();
            float normalisedValue = range.convertTo0to1(value);
            param->setValueNotifyingHost(normalisedValue);
        }
    }
}

void PresetManager::loadUserPreset(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || xml->getTagName() != juce::String("Preset"))
        return;

    for (auto* child : xml->getChildIterator())
    {
        if (child->getTagName() == "PARAM")
        {
            auto paramID = child->getStringAttribute("id");
            auto value = child->getDoubleAttribute("value");

            if (auto* param = apvts_.getParameter(paramID))
            {
                auto range = param->getNormalisableRange();
                float normalisedValue = range.convertTo0to1(static_cast<float>(value));
                param->setValueNotifyingHost(normalisedValue);
            }
        }
    }
}

// ---------------------------------------------------------------
// Save preset
// ---------------------------------------------------------------

void PresetManager::savePreset(const juce::String& name, const juce::String& category)
{
    auto categoryDir = getUserPresetDirectory().getChildFile(category);
    categoryDir.createDirectory();

    auto presetFile = categoryDir.getChildFile(name + ".preset");

    // Build XML
    juce::XmlElement presetXml("Preset");
    presetXml.setAttribute("name", name);
    presetXml.setAttribute("category", category);

    for (auto* param : apvts_.processor.getParameters())
    {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
        {
            if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(param))
            {
                auto range = rangedParam->getNormalisableRange();
                float rawValue = range.convertFrom0to1(param->getValue());
                auto* paramEl = presetXml.createNewChildElement("PARAM");
                paramEl->setAttribute("id", paramWithID->paramID);
                paramEl->setAttribute("value", rawValue);
            }
        }
    }

    presetXml.writeTo(presetFile);

    // Refresh and select the newly saved preset
    refreshPresets();

    // Find and select the saved preset
    for (int i = 0; i < allPresets_.size(); ++i)
    {
        if (allPresets_.getReference(i).name == name &&
            allPresets_.getReference(i).category == category)
        {
            currentIndex_ = i;
            dirty_ = false;
            listeners_.call(&Listener::presetChanged, name);
            listeners_.call(&Listener::presetDirtyChanged, false);
            break;
        }
    }
}

// ---------------------------------------------------------------
// Delete preset
// ---------------------------------------------------------------

bool PresetManager::deletePreset(int index)
{
    if (index < 0 || index >= allPresets_.size())
        return false;

    auto& preset = allPresets_.getReference(index);
    if (preset.isFactory)
        return false;

    bool wasCurrent = (index == currentIndex_);
    preset.file.deleteFile();
    refreshPresets();

    if (wasCurrent)
    {
        currentIndex_ = 0;
        dirty_ = false;
        listeners_.call(&Listener::presetChanged, allPresets_.getReference(0).name);
        listeners_.call(&Listener::presetDirtyChanged, false);
    }

    return true;
}

// ---------------------------------------------------------------
// Dirty tracking via parameter listener
// ---------------------------------------------------------------

void PresetManager::parameterChanged(const juce::String& /*parameterID*/, float /*newValue*/)
{
    if (!dirty_)
    {
        dirty_ = true;
        listeners_.call(&Listener::presetDirtyChanged, true);
    }
}
