// Source/SynthVoice.cpp
#include "SynthVoice.h"

#include <cmath>

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

SynthVoice::SynthVoice()  = default;
SynthVoice::~SynthVoice() = default;

// ---------------------------------------------------------------
// SynthesiserVoice overrides
// ---------------------------------------------------------------

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    juce::ignoreUnused(sound);
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound* sound,
                           int currentPitchWheelPosition)
{
    juce::ignoreUnused(sound);

    currentMidiNote_ = midiNoteNumber;
    velocity_ = velocity;
    pitchWheelValue_ = currentPitchWheelPosition;

    // Convert MIDI note to frequency (with pitch bend)
    currentFrequency_ = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    currentFrequency_ *= std::pow(2.0f, (pitchWheelValue_ - 8192) / 8192.0f);

    // Set oscillator frequencies
    for (auto& osc : oscs_)
        osc.setFrequency(currentFrequency_);

    // Trigger envelopes
    ampEnv_.noteOn();
    filterEnv_.noteOn();
    modEnv_.noteOn();

    isActive_ = true;
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);

    // Release envelopes
    ampEnv_.noteOff();
    filterEnv_.noteOff();
    modEnv_.noteOff();

    if (!allowTailOff)
    {
        // Immediate silence
        ampEnv_.reset();
        filterEnv_.reset();
        modEnv_.reset();
        clearCurrentNote();
        isActive_ = false;
    }
}

void SynthVoice::pitchWheelMoved(int newValue)
{
    pitchWheelValue_ = newValue;

    // Recalculate frequency with pitch bend (range: +/- 2 semitones)
    currentFrequency_ = juce::MidiMessage::getMidiNoteInHertz(currentMidiNote_);
    currentFrequency_ *= std::pow(2.0f, (pitchWheelValue_ - 8192) / 8192.0f);

    for (auto& osc : oscs_)
        osc.setFrequency(currentFrequency_);
}

void SynthVoice::controllerMoved(int controllerNumber, int newValue)
{
    juce::ignoreUnused(controllerNumber, newValue);
    // Future: handle CC messages (mod wheel, etc.)
}

bool SynthVoice::isVoiceActive() const
{
    return isActive_;
}

// ---------------------------------------------------------------
// Audio rendering
// ---------------------------------------------------------------

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                 int startSample, int numSamples)
{
    if (!isActive_)
        return;

    const int numChannels = outputBuffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Process LFOs
        // Process LFOs to advance their phase; values will be used for
        // modulation routing in a later task (ModMatrix integration).
        (void)lfos_[0].process();
        (void)lfos_[1].process();

        // 2. Process envelopes
        float ampEnv    = ampEnv_.process();
        float filterEnv = filterEnv_.process();
        // Process mod envelope; value will be used in ModMatrix integration.
        (void)modEnv_.process();

        // If amp envelope is done, the voice is silent
        if (!ampEnv_.isActive() && ampEnv < 0.0001f)
        {
            isActive_ = false;
            clearCurrentNote();

            // Fill remaining buffer with silence
            for (int ch = 0; ch < numChannels; ++ch)
            {
                auto* channelData = outputBuffer.getWritePointer(ch);
                for (int j = i; j < numSamples; ++j)
                    channelData[startSample + j] = 0.0f;
            }
            return;
        }

        // 3. Mix 3 oscillators (equal weight: 1/3 each)
        float oscMix = 0.0f;
        for (auto& osc : oscs_)
            oscMix += osc.process();
        oscMix *= (1.0f / 3.0f);

        // 4. Apply filter with envelope modulation
        float filtered = filter_.process(oscMix, filterEnv, currentFrequency_);

        // 5. Apply amp envelope and velocity
        float sample = filtered * ampEnv * velocity_;

        // 6. Write stereo output
        for (int ch = 0; ch < numChannels; ++ch)
        {
            outputBuffer.addSample(ch, startSample + i, sample);
        }
    }
}

// ---------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    sampleRate_ = sampleRate;

    for (auto& osc : oscs_)
        osc.init(sampleRate);

    filter_.init(sampleRate);
    ampEnv_.init(sampleRate);
    filterEnv_.init(sampleRate);
    modEnv_.init(sampleRate);

    for (auto& lfo : lfos_)
        lfo.init(sampleRate);
}

// ---------------------------------------------------------------
// Oscillator parameter setters
// ---------------------------------------------------------------

void SynthVoice::setOscWaveform(int oscIndex, SynthOscillator::Waveform wf)
{
    if (oscIndex >= 0 && oscIndex < 3)
        oscs_[oscIndex].setWaveform(wf);
}

void SynthVoice::setOscDetune(int oscIndex, float cents)
{
    if (oscIndex >= 0 && oscIndex < 3)
        oscs_[oscIndex].setDetune(cents);
}

void SynthVoice::setOscOctave(int oscIndex, int octaves)
{
    if (oscIndex >= 0 && oscIndex < 3)
        oscs_[oscIndex].setOctave(octaves);
}

void SynthVoice::setOscLevel(int oscIndex, float level)
{
    if (oscIndex >= 0 && oscIndex < 3)
        oscs_[oscIndex].setLevel(level);
}

void SynthVoice::setOscPulseWidth(int oscIndex, float pw)
{
    if (oscIndex >= 0 && oscIndex < 3)
        oscs_[oscIndex].setPulseWidth(pw);
}

// ---------------------------------------------------------------
// Filter parameter setters
// ---------------------------------------------------------------

void SynthVoice::setFilterMode(SynthFilter::Mode mode)       { filter_.setMode(mode); }
void SynthVoice::setFilterSlope(SynthFilter::Slope slope)    { filter_.setSlope(slope); }
void SynthVoice::setFilterCutoff(float freqHz)               { filter_.setCutoff(freqHz); }
void SynthVoice::setFilterResonance(float res)               { filter_.setResonance(res); }
void SynthVoice::setFilterDrive(float drive)                 { filter_.setDrive(drive); }
void SynthVoice::setFilterEnvAmount(float amount)            { filter_.setEnvAmount(amount); }
void SynthVoice::setFilterKeyTracking(float amount)          { filter_.setKeyTracking(amount); }

// ---------------------------------------------------------------
// Amp envelope parameter setters
// ---------------------------------------------------------------

void SynthVoice::setAmpAttack(float seconds)    { ampEnv_.setAttack(seconds); }
void SynthVoice::setAmpDecay(float seconds)     { ampEnv_.setDecay(seconds); }
void SynthVoice::setAmpSustain(float level)     { ampEnv_.setSustain(level); }
void SynthVoice::setAmpRelease(float seconds)   { ampEnv_.setRelease(seconds); }

// ---------------------------------------------------------------
// Filter envelope parameter setters
// ---------------------------------------------------------------

void SynthVoice::setFilterAttack(float seconds)  { filterEnv_.setAttack(seconds); }
void SynthVoice::setFilterDecay(float seconds)   { filterEnv_.setDecay(seconds); }
void SynthVoice::setFilterSustain(float level)   { filterEnv_.setSustain(level); }
void SynthVoice::setFilterRelease(float seconds) { filterEnv_.setRelease(seconds); }

// ---------------------------------------------------------------
// Mod envelope parameter setters
// ---------------------------------------------------------------

void SynthVoice::setModAttack(float seconds)     { modEnv_.setAttack(seconds); }
void SynthVoice::setModDecay(float seconds)      { modEnv_.setDecay(seconds); }
void SynthVoice::setModSustain(float level)      { modEnv_.setSustain(level); }
void SynthVoice::setModRelease(float seconds)    { modEnv_.setRelease(seconds); }

// ---------------------------------------------------------------
// LFO parameter setters
// ---------------------------------------------------------------

void SynthVoice::setLFOShape(int lfoIndex, SynthLFO::Shape shape)
{
    if (lfoIndex >= 0 && lfoIndex < 2)
        lfos_[lfoIndex].setShape(shape);
}

void SynthVoice::setLFORate(int lfoIndex, float hz)
{
    if (lfoIndex >= 0 && lfoIndex < 2)
        lfos_[lfoIndex].setRate(hz);
}

void SynthVoice::setLFODepth(int lfoIndex, float depth)
{
    if (lfoIndex >= 0 && lfoIndex < 2)
        lfos_[lfoIndex].setDepth(depth);
}
