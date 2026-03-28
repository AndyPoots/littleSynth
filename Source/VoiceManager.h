// Source/VoiceManager.h
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthVoice.h"

class VoiceManager : public juce::Synthesiser
{
public:
    VoiceManager();
    ~VoiceManager() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    // Returns a typed pointer to a specific voice for parameter updates
    SynthVoice* getVoice(int index);

    static constexpr int kDefaultVoices = 16;

protected:
    // Oldest-active-first stealing strategy
    juce::SynthesiserVoice* findVoiceToSteal(juce::SynthesiserSound* soundToPlay,
                                             int channel,
                                             int noteNumber) const override;
};
