// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"
#include "Oscillator.h"
#include "Filter.h"
#include "LFO.h"

// ---------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------

LittleSynthProcessor::LittleSynthProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    apvts_ = std::make_unique<juce::AudioProcessorValueTreeState>(
        *this, nullptr, "Parameters", createParameterLayout());
}

LittleSynthProcessor::~LittleSynthProcessor() = default;

// ---------------------------------------------------------------
// AudioProcessor lifecycle
// ---------------------------------------------------------------

void LittleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth_.prepareToPlay(sampleRate, samplesPerBlock);
}

void LittleSynthProcessor::releaseResources()
{
    // Nothing extra to do; VoiceManager handles its own cleanup
}

// ---------------------------------------------------------------
// Parameter → voice update helpers
// ---------------------------------------------------------------

namespace
{

// Read a float parameter by ID
float readFloat(juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    return apvts.getRawParameterValue(id)->load();
}

// Read a choice parameter as an integer index
int readChoice(juce::AudioProcessorValueTreeState& apvts, const juce::String& id)
{
    return static_cast<int>(apvts.getRawParameterValue(id)->load());
}

// Map oscillator choice index to SynthOscillator::Waveform enum
SynthOscillator::Waveform oscWaveformFromChoice(int choice)
{
    switch (choice)
    {
        case 0:  return SynthOscillator::Sine;
        case 1:  return SynthOscillator::Triangle;
        case 2:  return SynthOscillator::Sawtooth;
        case 3:  return SynthOscillator::Square;
        case 4:  return SynthOscillator::Noise;
        default: return SynthOscillator::Sine;
    }
}

// Map filter mode choice index to SynthFilter::Mode enum
SynthFilter::Mode filterModeFromChoice(int choice)
{
    switch (choice)
    {
        case 0:  return SynthFilter::Lowpass;
        case 1:  return SynthFilter::Highpass;
        case 2:  return SynthFilter::Bandpass;
        case 3:  return SynthFilter::Notch;
        default: return SynthFilter::Lowpass;
    }
}

// Map filter slope choice index to SynthFilter::Slope enum
SynthFilter::Slope filterSlopeFromChoice(int choice)
{
    switch (choice)
    {
        case 0:  return SynthFilter::Slope12;
        case 1:  return SynthFilter::Slope24;
        default: return SynthFilter::Slope24;
    }
}

// Map LFO shape choice index to SynthLFO::Shape enum
SynthLFO::Shape lfoShapeFromChoice(int choice)
{
    switch (choice)
    {
        case 0:  return SynthLFO::Sine;
        case 1:  return SynthLFO::Triangle;
        case 2:  return SynthLFO::Sawtooth;
        case 3:  return SynthLFO::Square;
        case 4:  return SynthLFO::SampleAndHold;
        case 5:  return SynthLFO::Random;
        default: return SynthLFO::Sine;
    }
}

} // anonymous namespace

// ---------------------------------------------------------------
// Audio processing
// ---------------------------------------------------------------

void LittleSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // --- Push parameters to all voices ---
    const char* oscPrefixes[] = { "osc1_", "osc2_", "osc3_" };
    const char* lfoPrefixes[] = { "lfo1_", "lfo2_" };

    for (int v = 0; v < synth_.getNumVoices(); ++v)
    {
        auto* voice = synth_.getVoice(v);
        if (voice == nullptr) continue;

        // -- Oscillators --
        for (int osc = 0; osc < 3; ++osc)
        {
            auto p = juce::String(oscPrefixes[osc]);

            voice->setOscWaveform(osc, oscWaveformFromChoice(readChoice(*apvts_, p + "waveform")));
            voice->setOscDetune(osc, readFloat(*apvts_, p + "detune"));
            voice->setOscOctave(osc, readChoice(*apvts_, p + "octave") - 2);  // choice 0..4 maps to -2..+2
            voice->setOscLevel(osc, readFloat(*apvts_, p + "level"));
            voice->setOscPulseWidth(osc, readFloat(*apvts_, p + "pulse_width"));
        }

        // -- Filter --
        voice->setFilterMode(filterModeFromChoice(readChoice(*apvts_, "filter_mode")));
        voice->setFilterSlope(filterSlopeFromChoice(readChoice(*apvts_, "filter_slope")));
        voice->setFilterCutoff(readFloat(*apvts_, "filter_cutoff"));
        voice->setFilterResonance(readFloat(*apvts_, "filter_resonance"));
        voice->setFilterDrive(readFloat(*apvts_, "filter_drive"));
        voice->setFilterEnvAmount(readFloat(*apvts_, "filter_env_amount"));
        voice->setFilterKeyTracking(readFloat(*apvts_, "filter_key_tracking"));

        // -- Envelopes --
        // amp_env_
        voice->setAmpAttack(readFloat(*apvts_, "amp_env_attack"));
        voice->setAmpDecay(readFloat(*apvts_, "amp_env_decay"));
        voice->setAmpSustain(readFloat(*apvts_, "amp_env_sustain"));
        voice->setAmpRelease(readFloat(*apvts_, "amp_env_release"));

        // filter_env_
        voice->setFilterAttack(readFloat(*apvts_, "filter_env_attack"));
        voice->setFilterDecay(readFloat(*apvts_, "filter_env_decay"));
        voice->setFilterSustain(readFloat(*apvts_, "filter_env_sustain"));
        voice->setFilterRelease(readFloat(*apvts_, "filter_env_release"));

        // mod_env_
        voice->setModAttack(readFloat(*apvts_, "mod_env_attack"));
        voice->setModDecay(readFloat(*apvts_, "mod_env_decay"));
        voice->setModSustain(readFloat(*apvts_, "mod_env_sustain"));
        voice->setModRelease(readFloat(*apvts_, "mod_env_release"));

        // -- LFOs --
        for (int lfo = 0; lfo < 2; ++lfo)
        {
            auto p = juce::String(lfoPrefixes[lfo]);

            voice->setLFOShape(lfo, lfoShapeFromChoice(readChoice(*apvts_, p + "shape")));
            voice->setLFORate(lfo, readFloat(*apvts_, p + "rate"));
            voice->setLFODepth(lfo, readFloat(*apvts_, p + "depth"));
        }
    }

    // --- Render audio ---
    buffer.clear();
    synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // --- Apply master level ---
    const float masterLevel = readFloat(*apvts_, "master_level");
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.applyGain(ch, 0, buffer.getNumSamples(), masterLevel);
}

// ---------------------------------------------------------------
// Editor
// ---------------------------------------------------------------

juce::AudioProcessorEditor* LittleSynthProcessor::createEditor()
{
    return new LittleSynthEditor(*this);
}

// ---------------------------------------------------------------
// Plugin entry point
// ---------------------------------------------------------------

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LittleSynthProcessor();
}
