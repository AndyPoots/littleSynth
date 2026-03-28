// Source/SynthVoice.h
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"
#include "ModMatrix.h"

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice();
    ~SynthVoice() override;

    // --- SynthesiserVoice overrides ---
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newValue) override;
    void controllerMoved(int controllerNumber, int newValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    bool isVoiceActive() const override;

    // --- Initialisation ---
    void prepareToPlay(double sampleRate, int samplesPerBlock);

    // --- Oscillator parameter setters ---
    void setOscWaveform(int oscIndex, SynthOscillator::Waveform wf);
    void setOscDetune(int oscIndex, float cents);
    void setOscOctave(int oscIndex, int octaves);
    void setOscLevel(int oscIndex, float level);
    void setOscPulseWidth(int oscIndex, float pw);

    // --- Filter parameter setters ---
    void setFilterMode(SynthFilter::Mode mode);
    void setFilterSlope(SynthFilter::Slope slope);
    void setFilterCutoff(float freqHz);
    void setFilterResonance(float res);
    void setFilterDrive(float drive);
    void setFilterEnvAmount(float amount);
    void setFilterKeyTracking(float amount);

    // --- Envelope parameter setters ---
    void setAmpAttack(float seconds);
    void setAmpDecay(float seconds);
    void setAmpSustain(float level);
    void setAmpRelease(float seconds);

    void setFilterAttack(float seconds);
    void setFilterDecay(float seconds);
    void setFilterSustain(float level);
    void setFilterRelease(float seconds);

    void setModAttack(float seconds);
    void setModDecay(float seconds);
    void setModSustain(float level);
    void setModRelease(float seconds);

    // --- LFO parameter setters ---
    void setLFOShape(int lfoIndex, SynthLFO::Shape shape);
    void setLFORate(int lfoIndex, float hz);
    void setLFODepth(int lfoIndex, float depth);

    // --- Mod Matrix setters ---
    void setModMatrixSource(int slot, ModMatrix::Source source);
    void setModMatrixDestination(int slot, ModMatrix::Destination dest);
    void setModMatrixDepth(int slot, float depth);
    void setModMatrixBipolar(int slot, bool bipolar);
    void setModMatrixActive(int slot, bool active);

private:
    // DSP components
    SynthOscillator oscs_[3];
    SynthFilter filter_;
    SynthEnvelope ampEnv_;
    SynthEnvelope filterEnv_;
    SynthEnvelope modEnv_;
    SynthLFO lfos_[2];

    // Voice state
    double sampleRate_ = 44100.0;
    float currentFrequency_ = 440.0f;
    float velocity_ = 0.0f;
    int currentMidiNote_ = 0;
    int pitchWheelValue_ = 8192; // centre = no bend
    float modWheelValue_ = 0.0f; // [0, 1]
    bool isActive_ = false;

    // Modulation matrix
    ModMatrix modMatrix_;
};
