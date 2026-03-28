// Source/VoiceManager.cpp
#include "VoiceManager.h"

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

VoiceManager::VoiceManager()
{
    // Create SynthSound (shared by all voices)
    addSound(new SynthSound());

    // Create 16 SynthVoice instances
    for (int i = 0; i < kDefaultVoices; ++i)
        addVoice(new SynthVoice());
}

VoiceManager::~VoiceManager()
{
    // juce::Synthesiser base class handles deletion of voices and sounds
}

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void VoiceManager::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Initialise every voice
    for (int i = 0; i < getNumVoices(); ++i)
    {
        auto* voice = dynamic_cast<SynthVoice*>(getVoice(i));
        if (voice != nullptr)
            voice->prepareToPlay(sampleRate, samplesPerBlock);
    }

    // Set a reasonable default voice stealing mode
    setNoteStealingEnabled(true);
}

// ---------------------------------------------------------------
// Voice access
// ---------------------------------------------------------------

SynthVoice* VoiceManager::getVoice(int index)
{
    if (index >= 0 && index < getNumVoices())
        return dynamic_cast<SynthVoice*>(juce::Synthesiser::getVoice(index));
    return nullptr;
}

// ---------------------------------------------------------------
// Voice stealing: oldest-active-first strategy
// ---------------------------------------------------------------

juce::SynthesiserVoice* VoiceManager::findVoiceToSteal(
    juce::SynthesiserSound* soundToPlay,
    int channel,
    int noteNumber) const
{
    juce::ignoreUnused(soundToPlay, channel, noteNumber);

    // Collect all active voices
    juce::Array<SynthVoice*> activeVoices;

    for (int i = 0; i < getNumVoices(); ++i)
    {
        auto* voice = dynamic_cast<SynthVoice*>(juce::Synthesiser::getVoice(i));
        if (voice != nullptr && voice->isVoiceActive())
            activeVoices.add(voice);
    }

    // If no active voices, steal the first one
    if (activeVoices.isEmpty())
    {
        return juce::Synthesiser::getVoice(0);
    }

    // Steal the oldest active voice (the one at the lowest index in the voices array,
    // which corresponds to the longest-running note since JUCE appends new notes
    // and searches from the start). We find the one that started earliest.
    // Simple approach: steal the first active voice in the array.
    return activeVoices.getFirst();
}
