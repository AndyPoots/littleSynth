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
    createFactoryPresets();
}

LittleSynthProcessor::~LittleSynthProcessor() = default;

// ---------------------------------------------------------------
// AudioProcessor lifecycle
// ---------------------------------------------------------------

void LittleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth_.prepareToPlay(sampleRate, samplesPerBlock);
    effectsChain_.init(sampleRate);

    // Kill any stuck notes from DAW plugin scanning or previous session
    synth_.allNotesOff(0, true);
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

    // --- Apply effects chain ---
    if (buffer.getNumChannels() >= 2)
    {
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getWritePointer(1);
        effectsChain_.process(left, right, buffer.getNumSamples());
    }
}

// ---------------------------------------------------------------
// Editor
// ---------------------------------------------------------------

juce::AudioProcessorEditor* LittleSynthProcessor::createEditor()
{
    return new LittleSynthEditor(*this);
}

// ---------------------------------------------------------------
// State save / load
// ---------------------------------------------------------------

void LittleSynthProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts_->copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LittleSynthProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts_->state.getType()))
        {
            apvts_->replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

// ---------------------------------------------------------------
// Factory presets
// ---------------------------------------------------------------

namespace
{

// Helper to build a ValueTree preset from a list of (paramID, value) pairs.
// Choice parameters are stored as their integer index (which is what APVTS expects).
juce::ValueTree makePreset(const char* name, std::initializer_list<std::pair<const char*, float>> params)
{
    juce::ValueTree preset("Preset");
    preset.setProperty("name", juce::String(name), nullptr);
    for (const auto& [id, val] : params)
        preset.setProperty(juce::Identifier(id), val, nullptr);
    return preset;
}

} // anonymous namespace

void LittleSynthProcessor::createFactoryPresets()
{
    // Waveform indices: 0=Sine, 1=Triangle, 2=Sawtooth, 3=Square, 4=Noise
    // Octave indices:   0="-2", 1="-1", 2="0", 3="+1", 4="+2"
    // Filter mode:      0=Lowpass, 1=Highpass, 2=Bandpass, 3=Notch

    // --- 1. Init: basic sine wave, filter open ---
    factoryPresets_.add(makePreset("Init", {
        // Osc 1: Sine, center, level 1
        {"osc1_waveform",  0.0f}, {"osc1_detune", 0.0f}, {"osc1_octave", 2.0f},
        {"osc1_level",     1.0f}, {"osc1_pulse_width", 0.5f},
        // Osc 2: off
        {"osc2_waveform",  0.0f}, {"osc2_detune", 0.0f}, {"osc2_octave", 2.0f},
        {"osc2_level",     0.0f}, {"osc2_pulse_width", 0.5f},
        // Osc 3: off
        {"osc3_waveform",  0.0f}, {"osc3_detune", 0.0f}, {"osc3_octave", 2.0f},
        {"osc3_level",     0.0f}, {"osc3_pulse_width", 0.5f},
        // Filter: open lowpass
        {"filter_mode",    0.0f}, {"filter_slope", 1.0f}, {"filter_cutoff", 20000.0f},
        {"filter_resonance", 0.0f}, {"filter_drive", 1.0f}, {"filter_env_amount", 0.0f},
        {"filter_key_tracking", 0.0f},
        // Amp envelope: short
        {"amp_env_attack", 0.01f}, {"amp_env_decay", 0.1f}, {"amp_env_sustain", 0.7f},
        {"amp_env_release", 0.3f},
        // Filter envelope
        {"filter_env_attack", 0.01f}, {"filter_env_decay", 0.1f}, {"filter_env_sustain", 0.0f},
        {"filter_env_release", 0.3f},
        // Mod envelope
        {"mod_env_attack", 0.01f}, {"mod_env_decay", 0.1f}, {"mod_env_sustain", 0.0f},
        {"mod_env_release", 0.3f},
        // LFOs
        {"lfo1_shape", 0.0f}, {"lfo1_rate", 1.0f}, {"lfo1_depth", 0.0f},
        {"lfo2_shape", 0.0f}, {"lfo2_rate", 1.0f}, {"lfo2_depth", 0.0f},
        // Master
        {"master_level", 0.7f}
    }));

    // --- 2. Warm Pad: saw + detuned saw, lowpass filter, slow attack ---
    factoryPresets_.add(makePreset("Warm Pad", {
        // Osc 1: Sawtooth
        {"osc1_waveform",  2.0f}, {"osc1_detune", 0.0f}, {"osc1_octave", 2.0f},
        {"osc1_level",     0.7f}, {"osc1_pulse_width", 0.5f},
        // Osc 2: Sawtooth, detuned +7 cents
        {"osc2_waveform",  2.0f}, {"osc2_detune", 7.0f}, {"osc2_octave", 2.0f},
        {"osc2_level",     0.6f}, {"osc2_pulse_width", 0.5f},
        // Osc 3: Triangle, one octave down for body
        {"osc3_waveform",  1.0f}, {"osc3_detune", 0.0f}, {"osc3_octave", 1.0f},
        {"osc3_level",     0.3f}, {"osc3_pulse_width", 0.5f},
        // Filter: warm lowpass
        {"filter_mode",    0.0f}, {"filter_slope", 1.0f}, {"filter_cutoff", 2000.0f},
        {"filter_resonance", 0.2f}, {"filter_drive", 1.0f}, {"filter_env_amount", 0.3f},
        {"filter_key_tracking", 0.5f},
        // Amp envelope: slow attack for pad feel
        {"amp_env_attack", 1.5f}, {"amp_env_decay", 1.0f}, {"amp_env_sustain", 0.8f},
        {"amp_env_release", 2.0f},
        // Filter envelope: slow opening
        {"filter_env_attack", 1.0f}, {"filter_env_decay", 2.0f}, {"filter_env_sustain", 0.5f},
        {"filter_env_release", 1.5f},
        // Mod envelope
        {"mod_env_attack", 0.5f}, {"mod_env_decay", 2.0f}, {"mod_env_sustain", 0.0f},
        {"mod_env_release", 1.0f},
        // LFOs: subtle vibrato via LFO1
        {"lfo1_shape", 0.0f}, {"lfo1_rate", 5.0f}, {"lfo1_depth", 0.15f},
        {"lfo2_shape", 0.0f}, {"lfo2_rate", 0.2f}, {"lfo2_depth", 0.2f},
        // Master
        {"master_level", 0.6f}
    }));

    // --- 3. Bass: single saw, low octave, lowpass with env ---
    factoryPresets_.add(makePreset("Bass", {
        // Osc 1: Sawtooth, one octave down
        {"osc1_waveform",  2.0f}, {"osc1_detune", 0.0f}, {"osc1_octave", 1.0f},
        {"osc1_level",     1.0f}, {"osc1_pulse_width", 0.5f},
        // Osc 2: Square, two octaves down for sub
        {"osc2_waveform",  3.0f}, {"osc2_detune", 0.0f}, {"osc2_octave", 0.0f},
        {"osc2_level",     0.4f}, {"osc2_pulse_width", 0.5f},
        // Osc 3: off
        {"osc3_waveform",  0.0f}, {"osc3_detune", 0.0f}, {"osc3_octave", 2.0f},
        {"osc3_level",     0.0f}, {"osc3_pulse_width", 0.5f},
        // Filter: tight lowpass with envelope
        {"filter_mode",    0.0f}, {"filter_slope", 1.0f}, {"filter_cutoff", 500.0f},
        {"filter_resonance", 0.3f}, {"filter_drive", 1.5f}, {"filter_env_amount", 0.7f},
        {"filter_key_tracking", 0.3f},
        // Amp envelope: punchy
        {"amp_env_attack", 0.005f}, {"amp_env_decay", 0.4f}, {"amp_env_sustain", 0.4f},
        {"amp_env_release", 0.2f},
        // Filter envelope: plucky
        {"filter_env_attack", 0.005f}, {"filter_env_decay", 0.3f}, {"filter_env_sustain", 0.1f},
        {"filter_env_release", 0.2f},
        // Mod envelope
        {"mod_env_attack", 0.01f}, {"mod_env_decay", 0.5f}, {"mod_env_sustain", 0.0f},
        {"mod_env_release", 0.3f},
        // LFOs
        {"lfo1_shape", 0.0f}, {"lfo1_rate", 1.0f}, {"lfo1_depth", 0.0f},
        {"lfo2_shape", 0.0f}, {"lfo2_rate", 1.0f}, {"lfo2_depth", 0.0f},
        // Master
        {"master_level", 0.8f}
    }));

    // --- 4. Lead: saw + square, filter with resonance ---
    factoryPresets_.add(makePreset("Lead", {
        // Osc 1: Sawtooth
        {"osc1_waveform",  2.0f}, {"osc1_detune", 0.0f}, {"osc1_octave", 2.0f},
        {"osc1_level",     0.8f}, {"osc1_pulse_width", 0.5f},
        // Osc 2: Square, slight detune for thickness
        {"osc2_waveform",  3.0f}, {"osc2_detune", 5.0f}, {"osc2_octave", 2.0f},
        {"osc2_level",     0.5f}, {"osc2_pulse_width", 0.5f},
        // Osc 3: Sine one octave up for brightness
        {"osc3_waveform",  0.0f}, {"osc3_detune", 0.0f}, {"osc3_octave", 3.0f},
        {"osc3_level",     0.2f}, {"osc3_pulse_width", 0.5f},
        // Filter: resonant lowpass
        {"filter_mode",    0.0f}, {"filter_slope", 1.0f}, {"filter_cutoff", 3000.0f},
        {"filter_resonance", 0.6f}, {"filter_drive", 1.2f}, {"filter_env_amount", 0.4f},
        {"filter_key_tracking", 0.7f},
        // Amp envelope: snappy
        {"amp_env_attack", 0.01f}, {"amp_env_decay", 0.2f}, {"amp_env_sustain", 0.8f},
        {"amp_env_release", 0.15f},
        // Filter envelope
        {"filter_env_attack", 0.01f}, {"filter_env_decay", 0.4f}, {"filter_env_sustain", 0.3f},
        {"filter_env_release", 0.2f},
        // Mod envelope
        {"mod_env_attack", 0.01f}, {"mod_env_decay", 0.3f}, {"mod_env_sustain", 0.0f},
        {"mod_env_release", 0.2f},
        // LFOs: vibrato
        {"lfo1_shape", 0.0f}, {"lfo1_rate", 6.0f}, {"lfo1_depth", 0.1f},
        {"lfo2_shape", 0.0f}, {"lfo2_rate", 1.0f}, {"lfo2_depth", 0.0f},
        // Master
        {"master_level", 0.7f}
    }));

    // --- 5. Ambient: sine + triangle, reverb feel, slow attack ---
    factoryPresets_.add(makePreset("Ambient", {
        // Osc 1: Sine
        {"osc1_waveform",  0.0f}, {"osc1_detune", 0.0f}, {"osc1_octave", 2.0f},
        {"osc1_level",     0.8f}, {"osc1_pulse_width", 0.5f},
        // Osc 2: Triangle, slightly detuned
        {"osc2_waveform",  1.0f}, {"osc2_detune", 3.0f}, {"osc2_octave", 2.0f},
        {"osc2_level",     0.5f}, {"osc2_pulse_width", 0.5f},
        // Osc 3: Sine, octave up for shimmer
        {"osc3_waveform",  0.0f}, {"osc3_detune", -5.0f}, {"osc3_octave", 3.0f},
        {"osc3_level",     0.25f}, {"osc3_pulse_width", 0.5f},
        // Filter: gentle lowpass, wide open
        {"filter_mode",    0.0f}, {"filter_slope", 0.0f}, {"filter_cutoff", 8000.0f},
        {"filter_resonance", 0.1f}, {"filter_drive", 1.0f}, {"filter_env_amount", 0.1f},
        {"filter_key_tracking", 0.2f},
        // Amp envelope: very slow attack and release
        {"amp_env_attack", 3.0f}, {"amp_env_decay", 2.0f}, {"amp_env_sustain", 0.6f},
        {"amp_env_release", 4.0f},
        // Filter envelope
        {"filter_env_attack", 2.0f}, {"filter_env_decay", 3.0f}, {"filter_env_sustain", 0.4f},
        {"filter_env_release", 3.0f},
        // Mod envelope
        {"mod_env_attack", 1.0f}, {"mod_env_decay", 4.0f}, {"mod_env_sustain", 0.3f},
        {"mod_env_release", 3.0f},
        // LFOs: slow modulation
        {"lfo1_shape", 0.0f}, {"lfo1_rate", 0.3f}, {"lfo1_depth", 0.3f},
        {"lfo2_shape", 1.0f}, {"lfo2_rate", 0.15f}, {"lfo2_depth", 0.25f},
        // Master
        {"master_level", 0.6f}
    }));

    // Populate name list
    for (const auto& preset : factoryPresets_)
        presetNames_.add(preset.getProperty("name").toString());
}

// ---------------------------------------------------------------
// Program / preset methods
// ---------------------------------------------------------------

int LittleSynthProcessor::getNumPrograms()
{
    return factoryPresets_.size();
}

int LittleSynthProcessor::getCurrentProgram()
{
    return currentProgram_;
}

void LittleSynthProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < factoryPresets_.size())
    {
        currentProgram_ = index;

        const auto& preset = factoryPresets_[index];

        // Apply each preset value by setting the parameter directly via APVTS
        for (int i = 0; i < preset.getNumProperties(); ++i)
        {
            auto propID = preset.getPropertyName(i);
            if (propID.toString() == "name")
                continue;

            auto paramID = propID.toString();
            if (auto* param = apvts_->getParameter(paramID))
            {
                auto value = preset.getProperty(propID);
                // Use the parameter's normalisable range to convert the raw value
                // to the 0..1 normalised value that setValue expects
                auto range = param->getNormalisableRange();
                float normalisedValue = range.convertTo0to1(static_cast<float>(value));
                param->setValueNotifyingHost(normalisedValue);
            }
        }
    }
}

const juce::String LittleSynthProcessor::getProgramName(int index)
{
    if (index >= 0 && index < presetNames_.size())
        return presetNames_[index];
    return {};
}

void LittleSynthProcessor::changeProgramName(int index, const juce::String& newName)
{
    if (index >= 0 && index < presetNames_.size())
    {
        presetNames_.set(index, newName);
        factoryPresets_.getReference(index).setProperty("name", newName, nullptr);
    }
}

// ---------------------------------------------------------------
// Plugin entry point
// ---------------------------------------------------------------

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LittleSynthProcessor();
}
