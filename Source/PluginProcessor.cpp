// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"
#include "Oscillator.h"
#include "Filter.h"
#include "LFO.h"
#include "BinaryData.h"

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
    presetManager_ = std::make_unique<PresetManager>(*apvts_, factoryPresets_, presetNames_);
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

    // --- Push parameters to voices only when changed ---
    const char* oscPrefixes[] = { "osc1_", "osc2_", "osc3_" };
    const char* lfoPrefixes[] = { "lfo1_", "lfo2_" };
    const char* envPrefixes[] = { "amp_env_", "filter_env_", "mod_env_" };

    auto pushToVoices = [this](auto fn) {
        for (int v = 0; v < synth_.getNumVoices(); ++v)
            if (auto* voice = dynamic_cast<SynthVoice*>(synth_.getVoice(v)))
                fn(voice);
    };

    // Oscillator parameters
    for (int osc = 0; osc < 3; ++osc)
    {
        auto p = juce::String(oscPrefixes[osc]);

        int wf = readChoice(*apvts_, p + "waveform");
        if (wf != paramCache_.oscWaveform[osc])
        {
            paramCache_.oscWaveform[osc] = wf;
            auto waveform = oscWaveformFromChoice(wf);
            pushToVoices([=](SynthVoice* v) { v->setOscWaveform(osc, waveform); });
        }

        float detune = readFloat(*apvts_, p + "detune");
        if (detune != paramCache_.oscDetune[osc])
        {
            paramCache_.oscDetune[osc] = detune;
            pushToVoices([=](SynthVoice* v) { v->setOscDetune(osc, detune); });
        }

        int octChoice = readChoice(*apvts_, p + "octave");
        if (octChoice != paramCache_.oscOctave[osc])
        {
            paramCache_.oscOctave[osc] = octChoice;
            int octVal = octChoice - 2;
            pushToVoices([=](SynthVoice* v) { v->setOscOctave(osc, octVal); });
        }

        float level = readFloat(*apvts_, p + "level");
        if (level != paramCache_.oscLevel[osc])
        {
            paramCache_.oscLevel[osc] = level;
            pushToVoices([=](SynthVoice* v) { v->setOscLevel(osc, level); });
        }

        float pw = readFloat(*apvts_, p + "pulse_width");
        if (pw != paramCache_.oscPulseWidth[osc])
        {
            paramCache_.oscPulseWidth[osc] = pw;
            pushToVoices([=](SynthVoice* v) { v->setOscPulseWidth(osc, pw); });
        }

        bool on = readFloat(*apvts_, p + "on") > 0.5f;
        if (on != paramCache_.oscOn[osc])
        {
            paramCache_.oscOn[osc] = on;
            pushToVoices([=](SynthVoice* v) { v->setOscEnabled(osc, on); });
        }
    }

    // Filter parameters
    {
        int mode = readChoice(*apvts_, "filter_mode");
        if (mode != paramCache_.filterMode)
        {
            paramCache_.filterMode = mode;
            auto fm = filterModeFromChoice(mode);
            pushToVoices([fm](SynthVoice* v) { v->setFilterMode(fm); });
        }

        int slope = readChoice(*apvts_, "filter_slope");
        if (slope != paramCache_.filterSlope)
        {
            paramCache_.filterSlope = slope;
            auto fs = filterSlopeFromChoice(slope);
            pushToVoices([fs](SynthVoice* v) { v->setFilterSlope(fs); });
        }

        float cutoff = readFloat(*apvts_, "filter_cutoff");
        if (cutoff != paramCache_.filterCutoff)
        {
            paramCache_.filterCutoff = cutoff;
            pushToVoices([cutoff](SynthVoice* v) { v->setFilterCutoff(cutoff); });
        }

        float res = readFloat(*apvts_, "filter_resonance");
        if (res != paramCache_.filterResonance)
        {
            paramCache_.filterResonance = res;
            pushToVoices([res](SynthVoice* v) { v->setFilterResonance(res); });
        }

        float drive = readFloat(*apvts_, "filter_drive");
        if (drive != paramCache_.filterDrive)
        {
            paramCache_.filterDrive = drive;
            pushToVoices([drive](SynthVoice* v) { v->setFilterDrive(drive); });
        }

        float envAmt = readFloat(*apvts_, "filter_env_amount");
        if (envAmt != paramCache_.filterEnvAmount)
        {
            paramCache_.filterEnvAmount = envAmt;
            pushToVoices([envAmt](SynthVoice* v) { v->setFilterEnvAmount(envAmt); });
        }

        float kt = readFloat(*apvts_, "filter_key_tracking");
        if (kt != paramCache_.filterKeyTracking)
        {
            paramCache_.filterKeyTracking = kt;
            pushToVoices([kt](SynthVoice* v) { v->setFilterKeyTracking(kt); });
        }
    }

    // Envelope parameters (amp, filter, mod)
    for (int e = 0; e < 3; ++e)
    {
        auto p = juce::String(envPrefixes[e]);

        float atk = readFloat(*apvts_, p + "attack");
        if (atk != paramCache_.envAttack[e])
        {
            paramCache_.envAttack[e] = atk;
            pushToVoices([e, atk](SynthVoice* v) {
                if (e == 0)      v->setAmpAttack(atk);
                else if (e == 1) v->setFilterAttack(atk);
                else             v->setModAttack(atk);
            });
        }

        float dec = readFloat(*apvts_, p + "decay");
        if (dec != paramCache_.envDecay[e])
        {
            paramCache_.envDecay[e] = dec;
            pushToVoices([e, dec](SynthVoice* v) {
                if (e == 0)      v->setAmpDecay(dec);
                else if (e == 1) v->setFilterDecay(dec);
                else             v->setModDecay(dec);
            });
        }

        float sus = readFloat(*apvts_, p + "sustain");
        if (sus != paramCache_.envSustain[e])
        {
            paramCache_.envSustain[e] = sus;
            pushToVoices([e, sus](SynthVoice* v) {
                if (e == 0)      v->setAmpSustain(sus);
                else if (e == 1) v->setFilterSustain(sus);
                else             v->setModSustain(sus);
            });
        }

        float rel = readFloat(*apvts_, p + "release");
        if (rel != paramCache_.envRelease[e])
        {
            paramCache_.envRelease[e] = rel;
            pushToVoices([e, rel](SynthVoice* v) {
                if (e == 0)      v->setAmpRelease(rel);
                else if (e == 1) v->setFilterRelease(rel);
                else             v->setModRelease(rel);
            });
        }
    }

    // LFO parameters
    for (int lfo = 0; lfo < 2; ++lfo)
    {
        auto p = juce::String(lfoPrefixes[lfo]);

        int shape = readChoice(*apvts_, p + "shape");
        if (shape != paramCache_.lfoShape[lfo])
        {
            paramCache_.lfoShape[lfo] = shape;
            auto s = lfoShapeFromChoice(shape);
            pushToVoices([lfo, s](SynthVoice* v) { v->setLFOShape(lfo, s); });
        }

        float rate = readFloat(*apvts_, p + "rate");
        if (rate != paramCache_.lfoRate[lfo])
        {
            paramCache_.lfoRate[lfo] = rate;
            pushToVoices([lfo, rate](SynthVoice* v) { v->setLFORate(lfo, rate); });
        }

        float depth = readFloat(*apvts_, p + "depth");
        if (depth != paramCache_.lfoDepth[lfo])
        {
            paramCache_.lfoDepth[lfo] = depth;
            pushToVoices([lfo, depth](SynthVoice* v) { v->setLFODepth(lfo, depth); });
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

    // --- Capture output for visualizer (mono mix) ---
    const int numSamples = buffer.getNumSamples();
    for (int i = 0; i < numSamples; ++i)
    {
        float mono = 0.0f;
        if (buffer.getNumChannels() >= 2)
            mono = (buffer.getSample(0, i) + buffer.getSample(1, i)) * 0.5f;
        else if (buffer.getNumChannels() == 1)
            mono = buffer.getSample(0, i);

        pushOutputSample(mono);
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
// Output FIFO for visualizer
// ---------------------------------------------------------------

void LittleSynthProcessor::pushOutputSample(float sample)
{
    const int pos = outputWritePos_.load(std::memory_order_relaxed);
    outputFifo_[static_cast<size_t>(pos) % kOutputFifoSize] = sample;
    outputWritePos_.store((pos + 1) % kOutputFifoSize, std::memory_order_relaxed);
}

int LittleSynthProcessor::readOutputSamples(float* dest, int maxSamples)
{
    const int wp = outputWritePos_.load(std::memory_order_acquire);
    const int available = juce::jmin(maxSamples, kOutputFifoSize);
    for (int i = 0; i < available; ++i)
    {
        int idx = (wp - available + i + kOutputFifoSize) % kOutputFifoSize;
        dest[i] = outputFifo_[static_cast<size_t>(idx)];
    }
    return available;
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
// Factory presets (loaded from embedded BinaryData)
// ---------------------------------------------------------------

void LittleSynthProcessor::createFactoryPresets()
{
    // Load factory presets from the embedded .preset files in BinaryData.
    // Each file is an XML document with <Preset name="..." category="..."> root
    // and <PARAM id="..." value="..."/> children. We build a ValueTree with
    // properties (not children) so the existing loading code works unchanged.
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
    {
        int dataSize = 0;
        const char* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], dataSize);
        const char* originalPath = BinaryData::getNamedResourceOriginalFilename(BinaryData::namedResourceList[i]);

        if (data == nullptr || dataSize <= 0)
            continue;

        auto xml = juce::XmlDocument::parse(juce::String(data, dataSize));
        if (xml == nullptr || xml->getTagName() != "Preset")
            continue;

        juce::ValueTree preset("Preset");
        preset.setProperty("name", xml->getStringAttribute("name"), nullptr);

        // Get category from the "category" XML attribute, or from the file path
        juce::String category = xml->getStringAttribute("category");
        if (category.isEmpty())
        {
            juce::String path(originalPath);
            auto slashIdx = path.indexOfChar('/');
            if (slashIdx >= 0)
                category = path.substring(0, slashIdx);
        }
        preset.setProperty("category", category, nullptr);

        // Convert each <PARAM id="..." value="..."/> into a ValueTree property
        for (auto* paramEl : xml->getChildIterator())
        {
            if (paramEl->getTagName() == "PARAM")
            {
                auto id = paramEl->getStringAttribute("id");
                auto val = static_cast<float>(paramEl->getDoubleAttribute("value"));
                if (id.isNotEmpty())
                    preset.setProperty(juce::Identifier(id), val, nullptr);
            }
        }

        factoryPresets_.add(preset);
        presetNames_.add(preset.getProperty("name", "Unknown").toString());
    }
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
            if (propID.toString() == "name" || propID.toString() == "category")
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

        // Invalidate parameter cache so all changes propagate on next processBlock
        paramCache_ = ParamCache{};
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
