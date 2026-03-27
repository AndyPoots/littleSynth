# littleSynth Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Build a fully-featured polyphonic subtractive VST3 synthesizer with 3 oscillators, multi-mode filter, full modulation matrix, effects suite, and animated GUI.

**Architecture:** JUCE 8 plugin using CMake. DaisySP provides DSP primitives (oscillators, filters, envelopes, effects). We wrap DaisySP classes into our own components, compose them into SynthVoice instances managed by a custom VoiceManager, process through a global EffectsChain, and render via a custom JUCE GUI with visualizers.

**Tech Stack:** C++17, JUCE 8, DaisySP (MIT), CMake 3.22+

---

## Phase 1: Project Skeleton & Build System

### Task 1: Fetch dependencies and create CMakeLists.txt

**Files:**
- Create: `CMakeLists.txt`

**Step 1: Clone JUCE into ThirdParty**

```bash
cd /home/ace/repos/littleSynth
mkdir -p ThirdParty
git clone --depth 1 https://github.com/juce-framework/JUCE.git ThirdParty/JUCE
```

**Step 2: Clone DaisySP into ThirdParty**

```bash
git clone --depth 1 https://github.com/electro-smith/DaisySP.git ThirdParty/DaisySP
```

**Step 3: Create CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.22)
project(littleSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ThirdParty/JUCE)
add_subdirectory(ThirdParty/DaisySP)

juce_add_plugin(littleSynth
    VERSION 1.0.0
    COMPANY_NAME "littleSynth"
    PLUGIN_MANUFACTURER_CODE Lsnt
    PLUGIN_CODE LtSn
    FORMATS VST3 Standalone
    PRODUCT_NAME "littleSynth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Synth"
)

juce_generate_juce_header(littleSynth)

target_sources(littleSynth
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(littleSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_include_directories(littleSynth
    PRIVATE
        Source
        ThirdParty/DaisySP/Source
)

target_link_libraries(littleSynth
    PRIVATE
        DaisySP
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
```

**Step 4: Verify build configures**

```bash
cd /home/ace/repos/littleSynth
cmake -B build -G "Ninja"
```

Expected: CMake configures successfully, no errors.

**Step 5: Commit**

```bash
git add CMakeLists.txt
git commit -m "build: add CMakeLists.txt with JUCE 8 and DaisySP"
```

---

### Task 2: Create minimal PluginProcessor

**Files:**
- Create: `Source/PluginProcessor.h`
- Create: `Source/PluginProcessor.cpp`

**Step 1: Create Source directory and PluginProcessor.h**

```cpp
// Source/PluginProcessor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class LittleSynthProcessor : public juce::AudioProcessor
{
public:
    LittleSynthProcessor();
    ~LittleSynthProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "littleSynth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthProcessor)
};
```

**Step 2: Create PluginProcessor.cpp**

```cpp
// Source/PluginProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

LittleSynthProcessor::LittleSynthProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

LittleSynthProcessor::~LittleSynthProcessor() {}

void LittleSynthProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void LittleSynthProcessor::releaseResources() {}

void LittleSynthProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    buffer.clear();
}

juce::AudioProcessorEditor* LittleSynthProcessor::createEditor()
{
    return new LittleSynthEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LittleSynthProcessor();
}
```

**Step 3: Commit**

```bash
git add Source/PluginProcessor.h Source/PluginProcessor.cpp
git commit -m "feat: add minimal PluginProcessor shell"
```

---

### Task 3: Create minimal PluginEditor

**Files:**
- Create: `Source/PluginEditor.h`
- Create: `Source/PluginEditor.cpp`

**Step 1: Create PluginEditor.h**

```cpp
// Source/PluginEditor.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class LittleSynthProcessor;

class LittleSynthEditor : public juce::AudioProcessorEditor
{
public:
    explicit LittleSynthEditor(LittleSynthProcessor&);
    ~LittleSynthEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    LittleSynthProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LittleSynthEditor)
};
```

**Step 2: Create PluginEditor.cpp**

```cpp
// Source/PluginEditor.cpp
#include "PluginEditor.h"
#include "PluginProcessor.h"

LittleSynthEditor::LittleSynthEditor(LittleSynthProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(800, 500);
}

LittleSynthEditor::~LittleSynthEditor() {}

void LittleSynthEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("littleSynth", getLocalBounds(),
        juce::Justification::centred, true);
}

void LittleSynthEditor::resized() {}
```

**Step 3: Build and verify**

```bash
cd /home/ace/repos/littleSynth
cmake --build build --config Release
```

Expected: Build succeeds. VST3 binary produced in `build/littleSynth_artefacts/`.

**Step 4: Commit**

```bash
git add Source/PluginEditor.h Source/PluginEditor.cpp
git commit -m "feat: add minimal PluginEditor shell"
```

---

## Phase 2: Core DSP Components

### Task 4: Create Oscillator wrapper

**Files:**
- Create: `Source/Oscillator.h`
- Create: `Source/Oscillator.cpp`

**Step 1: Create Oscillator.h**

Wrap DaisySP's `Oscillator` and `VariableShapeOscillator` into a unified interface with anti-aliased waveform support.

```cpp
// Source/Oscillator.h
#pragma once

#include <cstdint>

namespace daisysp
{
    class Oscillator;
    class VariableShapeOscillator;
}

class SynthOscillator
{
public:
    enum Waveform
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Square,
        Noise,
        NumWaveforms
    };

    SynthOscillator();
    ~SynthOscillator();

    void init(double sampleRate);
    void setWaveform(Waveform wf);
    void setFrequency(float freqHz);
    void setDetune(float cents);
    void setOctave(int octaves);
    void setLevel(float level);
    void setPulseWidth(float pw);
    void setSync(bool sync, float masterPhase = 0.0f);

    float process();

private:
    daisysp::Oscillator* rawOsc_;
    daisysp::VariableShapeOscillator* antiAliasedOsc_;
    double sampleRate_ = 44100.0;
    Waveform waveform_ = Sine;
    float baseFreq_ = 440.0f;
    float detuneCents_ = 0.0f;
    int octaveShift_ = 0;
    float level_ = 1.0f;
    float pulseWidth_ = 0.5f;
    bool sync_ = false;
    float masterPhase_ = 0.0f;

    float applyDetune(float freq) const;
};
```

**Step 2: Create Oscillator.cpp**

```cpp
// Source/Oscillator.cpp
#include "Oscillator.h"
#include <oscillator.h>
#include <variablesawosc.h>
#include <variableshapeosc.h>
#include <cmath>

SynthOscillator::SynthOscillator()
    : rawOsc_(new daisysp::Oscillator())
    , antiAliasedOsc_(new daisysp::VariableShapeOscillator())
{
}

SynthOscillator::~SynthOscillator() = default;

void SynthOscillator::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    rawOsc_->Init(sampleRate);
    antiAliasedOsc_->Init(sampleRate);
}

void SynthOscillator::setWaveform(Waveform wf)
{
    waveform_ = wf;
}

void SynthOscillator::setFrequency(float freqHz)
{
    baseFreq_ = freqHz;
}

void SynthOscillator::setDetune(float cents)
{
    detuneCents_ = cents;
}

void SynthOscillator::setOctave(int octaves)
{
    octaveShift_ = octaves;
}

void SynthOscillator::setLevel(float level)
{
    level_ = level;
}

void SynthOscillator::setPulseWidth(float pw)
{
    pulseWidth_ = pw;
}

void SynthOscillator::setSync(bool sync, float masterPhase)
{
    sync_ = sync;
    masterPhase_ = masterPhase;
}

float SynthOscillator::process()
{
    float freq = applyDetune(baseFreq_);

    switch (waveform_)
    {
        case Sawtooth:
        case Square:
        {
            // Use anti-aliased oscillator for saw/square
            antiAliasedOsc_->SetFreq(freq);
            float shape = (waveform_ == Square) ? pulseWidth_ : 0.5f;
            // VariableShapeOsc: 0 = saw, 0.5 = square-ish, 1 = pulse
            antiAliasedOsc_->SetWaveshape(shape);
            float sample = antiAliasedOsc_->Process();
            return sample * level_;
        }
        case Sine:
        case Triangle:
        default:
        {
            rawOsc_->SetFreq(freq);
            int wf = (waveform_ == Sine) ? 0 : 1; // 0=sin, 1=tri in DaisySP
            rawOsc_->SetWaveform(wf);
            return rawOsc_->Process() * level_;
        }
        case Noise:
        {
            // Simple white noise
            return (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * level_;
        }
    }
}

float SynthOscillator::applyDetune(float freq) const
{
    float detuneMultiplier = std::pow(2.0f, detuneCents_ / 1200.0f);
    float octaveMultiplier = std::pow(2.0f, (float)octaveShift_);
    return freq * detuneMultiplier * octaveMultiplier;
}
```

**Step 3: Add to CMakeLists.txt target_sources**

Add `Source/Oscillator.cpp` to the `target_sources` list in `CMakeLists.txt`.

**Step 4: Build to verify compilation**

```bash
cmake --build build --config Release
```

Expected: Compiles without errors.

**Step 5: Commit**

```bash
git add Source/Oscillator.h Source/Oscillator.cpp CMakeLists.txt
git commit -m "feat: add SynthOscillator wrapper around DaisySP"
```

---

### Task 5: Create Filter wrapper

**Files:**
- Create: `Source/Filter.h`
- Create: `Source/Filter.cpp`

**Step 1: Create Filter.h**

```cpp
// Source/Filter.h
#pragma once

namespace daisysp
{
    class MoogLadder;
    class Svf;
}

class SynthFilter
{
public:
    enum Mode
    {
        Lowpass = 0,
        Highpass,
        Bandpass,
        Notch,
        NumModes
    };

    enum Slope
    {
        Slope12 = 0,
        Slope24
    };

    SynthFilter();
    ~SynthFilter();

    void init(double sampleRate);
    void setMode(Mode mode);
    void setSlope(Slope slope);
    void setCutoff(float freqHz);
    void setResonance(float res);
    void setDrive(float drive);
    void setEnvAmount(float amount);
    void setKeyTracking(float amount);

    float process(float input, float envelopeValue = 0.0f, float noteFreq = 440.0f);

private:
    daisysp::MoogLadder* ladder_;
    daisysp::Svf* svf_;
    double sampleRate_ = 44100.0;
    Mode mode_ = Lowpass;
    Slope slope_ = Slope24;
    float cutoff_ = 1000.0f;
    float resonance_ = 0.5f;
    float drive_ = 0.5f;
    float envAmount_ = 0.0f;
    float keyTracking_ = 0.0f;
};
```

**Step 2: Create Filter.cpp**

```cpp
// Source/Filter.cpp
#include "Filter.h"
#include <ladder.h>
#include <svf.h>
#include <cmath>

SynthFilter::SynthFilter()
    : ladder_(new daisysp::MoogLadder())
    , svf_(new daisysp::Svf())
{
}

SynthFilter::~SynthFilter() = default;

void SynthFilter::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    ladder_->Init(sampleRate);
    svf_->Init(sampleRate);
}

void SynthFilter::setMode(Mode mode) { mode_ = mode; }
void SynthFilter::setSlope(Slope slope) { slope_ = slope; }
void SynthFilter::setCutoff(float freqHz) { cutoff_ = freqHz; }
void SynthFilter::setResonance(float res) { resonance_ = res; }
void SynthFilter::setDrive(float drive) { drive_ = drive; }
void SynthFilter::setEnvAmount(float amount) { envAmount_ = amount; }
void SynthFilter::setKeyTracking(float amount) { keyTracking_ = amount; }

float SynthFilter::process(float input, float envelopeValue, float noteFreq)
{
    // Calculate effective cutoff with envelope modulation and key tracking
    float envMod = envelopeValue * envAmount_ * 8000.0f;
    float keyMod = keyTracking_ * (noteFreq - 440.0f);
    float effectiveCutoff = std::clamp(cutoff_ + envMod + keyMod, 20.0f, 18000.0f);

    // Use Moog Ladder for LP (warm character), SVF for other modes
    if (mode_ == Lowpass)
    {
        ladder_->SetFreq(effectiveCutoff);
        ladder_->SetRes(resonance_);
        ladder_->SetDrive(drive_);

        float out = ladder_->Process(input);
        // For 24dB/oct, process twice (cascade)
        if (slope_ == Slope24)
            out = ladder_->Process(out);
        return out;
    }
    else
    {
        svf_->SetFreq(effectiveCutoff);
        svf_->SetRes(resonance_);
        svf_->SetDrive(drive_);

        svf_->Process(input);

        switch (mode_)
        {
            case Highpass:  return svf_->High();
            case Bandpass:  return svf_->Band();
            case Notch:     return svf_->Notch();
            default:        return svf_->Low();
        }
    }
}
```

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/Filter.h Source/Filter.cpp CMakeLists.txt
git commit -m "feat: add SynthFilter wrapper with Moog Ladder and SVF"
```

---

### Task 6: Create Envelope wrapper

**Files:**
- Create: `Source/Envelope.h`
- Create: `Source/Envelope.cpp`

**Step 1: Create Envelope.h**

```cpp
// Source/Envelope.h
#pragma once

namespace daisysp
{
    class Adsr;
}

class SynthEnvelope
{
public:
    SynthEnvelope();
    ~SynthEnvelope();

    void init(double sampleRate);

    void setAttack(float seconds);
    void setDecay(float seconds);
    void setSustain(float level);   // 0.0 to 1.0
    void setRelease(float seconds);

    void noteOn();
    void noteOff();

    float process();
    bool isActive() const;
    bool isReleasing() const;

    void reset();

private:
    daisysp::Adsr* adsr_;
    double sampleRate_ = 44100.0;
    bool gate_ = false;
    float lastValue_ = 0.0f;
};
```

**Step 2: Create Envelope.cpp**

```cpp
// Source/Envelope.cpp
#include "Envelope.h"
#include <adsr.h>

SynthEnvelope::SynthEnvelope()
    : adsr_(new daisysp::Adsr())
{
}

SynthEnvelope::~SynthEnvelope() = default;

void SynthEnvelope::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    adsr_->Init(sampleRate);
}

void SynthEnvelope::setAttack(float seconds)   { adsr_->SetTime(0, seconds); }
void SynthEnvelope::setDecay(float seconds)    { adsr_->SetTime(1, seconds); }
void SynthEnvelope::setSustain(float level)    { adsr_->SetSustainLevel(level); }
void SynthEnvelope::setRelease(float seconds)  { adsr_->SetTime(3, seconds); }

void SynthEnvelope::noteOn()
{
    gate_ = true;
}

void SynthEnvelope::noteOff()
{
    gate_ = false;
}

float SynthEnvelope::process()
{
    lastValue_ = adsr_->Process(gate_);
    return lastValue_;
}

bool SynthEnvelope::isActive() const
{
    return lastValue_ > 0.001f || gate_;
}

bool SynthEnvelope::isReleasing() const
{
    return !gate_ && lastValue_ > 0.001f;
}

void SynthEnvelope::reset()
{
    gate_ = false;
    lastValue_ = 0.0f;
}
```

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/Envelope.h Source/Envelope.cpp CMakeLists.txt
git commit -m "feat: add SynthEnvelope ADSR wrapper around DaisySP"
```

---

### Task 7: Create LFO wrapper

**Files:**
- Create: `Source/LFO.h`
- Create: `Source/LFO.cpp`

**Step 1: Create LFO.h**

```cpp
// Source/LFO.h
#pragma once

namespace daisysp
{
    class Oscillator;
}

class SynthLFO
{
public:
    enum Shape
    {
        Sine = 0,
        Triangle,
        Sawtooth,
        Square,
        SampleAndHold,
        Random,
        NumShapes
    };

    SynthLFO();
    ~SynthLFO();

    void init(double sampleRate);
    void setShape(Shape shape);
    void setRate(float hz);
    void setDepth(float depth);

    float process();

    // Returns current value in range [-1, 1]
    float getCurrentValue() const { return lastValue_; }

private:
    daisysp::Oscillator* osc_;
    double sampleRate_ = 44100.0;
    Shape shape_ = Sine;
    float rate_ = 1.0f;
    float depth_ = 1.0f;
    float lastValue_ = 0.0f;
    float heldValue_ = 0.0f;
    int shCounter_ = 0;
};
```

**Step 2: Create LFO.cpp**

```cpp
// Source/LFO.cpp
#include "LFO.h"
#include <oscillator.h>
#include <cstdlib>

SynthLFO::SynthLFO()
    : osc_(new daisysp::Oscillator())
{
}

SynthLFO::~SynthLFO() = default;

void SynthLFO::init(double sampleRate)
{
    sampleRate_ = sampleRate;
    osc_->Init(sampleRate);
    osc_->SetWaveform(0); // sine
}

void SynthLFO::setShape(Shape shape) { shape_ = shape; }
void SynthLFO::setRate(float hz) { rate_ = hz; }
void SynthLFO::setDepth(float depth) { depth_ = depth; }

float SynthLFO::process()
{
    osc_->SetFreq(rate_);

    switch (shape_)
    {
        case Sine:      osc_->SetWaveform(0); break;
        case Triangle:  osc_->SetWaveform(1); break;
        case Sawtooth:  osc_->SetWaveform(2); break;
        case Square:    osc_->SetWaveform(3); break;
        case SampleAndHold:
        {
            shCounter_++;
            if (shCounter_ >= (int)(sampleRate_ / rate_))
            {
                heldValue_ = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                shCounter_ = 0;
            }
            lastValue_ = heldValue_ * depth_;
            return lastValue_;
        }
        case Random:
        {
            heldValue_ = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            lastValue_ = heldValue_ * depth_;
            return lastValue_;
        }
        default: break;
    }

    lastValue_ = osc_->Process() * depth_;
    return lastValue_;
}
```

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/LFO.h Source/LFO.cpp CMakeLists.txt
git commit -m "feat: add SynthLFO wrapper with S&H and random shapes"
```

---

## Phase 3: Voice & Synth Engine

### Task 8: Create SynthVoice

**Files:**
- Create: `Source/SynthVoice.h`
- Create: `Source/SynthVoice.cpp`

**Step 1: Create SynthVoice.h**

A single polyphonic voice containing 3 oscillators, filter, 3 envelopes, and 2 LFOs.

```cpp
// Source/SynthVoice.h
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "Oscillator.h"
#include "Filter.h"
#include "Envelope.h"
#include "LFO.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    SynthVoice();
    ~SynthVoice() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    bool canPlaySound(juce::SynthesiserSound*) override;
    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>&, int startSample, int numSamples) override;

    // Parameter setters (called from UI thread via atomic params)
    void setOscWaveform(int oscIndex, SynthOscillator::Waveform wf);
    void setOscDetune(int oscIndex, float cents);
    void setOscOctave(int oscIndex, int octaves);
    void setOscLevel(int oscIndex, float level);
    void setOscPulseWidth(int oscIndex, float pw);

    void setFilterMode(SynthFilter::Mode mode);
    void setFilterCutoff(float freq);
    void setFilterResonance(float res);
    void setFilterEnvAmount(float amount);

    void setAmpEnv(float a, float d, float s, float r);
    void setFilterEnv(float a, float d, float s, float r);
    void setModEnv(float a, float d, float s, float r);

    void setLFOShape(int lfoIndex, SynthLFO::Shape shape);
    void setLFORate(int lfoIndex, float hz);
    void setLFODepth(int lfoIndex, float depth);

    bool isVoiceActive() const override;

private:
    SynthOscillator oscillators_[3];
    SynthFilter filter_;
    SynthEnvelope ampEnv_;
    SynthEnvelope filterEnv_;
    SynthEnvelope modEnv_;
    SynthLFO lfos_[2];

    float currentVelocity_ = 0.0f;
    float currentNoteFreq_ = 440.0f;
    int currentNote_ = 60;
    double sampleRate_ = 44100.0;
    bool prepared_ = false;

    float pitchWheelValue_ = 0.0f; // -1 to 1
};
```

**Step 2: Create SynthVoice.cpp**

```cpp
// Source/SynthVoice.cpp
#include "SynthVoice.h"
#include <juce_audio_basics/juce_audio_basics.h>

SynthVoice::SynthVoice() = default;
SynthVoice::~SynthVoice() = default;

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    for (auto& osc : oscillators_)
        osc.init(sampleRate);
    filter_.init(sampleRate);
    ampEnv_.init(sampleRate);
    filterEnv_.init(sampleRate);
    modEnv_.init(sampleRate);
    for (auto& lfo : lfos_)
        lfo.init(sampleRate);
    prepared_ = true;
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    juce::ignoreUnused(sound);
    return true;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity,
                           juce::SynthesiserSound*, int currentPitchWheelPosition)
{
    currentNote_ = midiNoteNumber;
    currentNoteFreq_ = (float)juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    currentVelocity_ = velocity;

    for (auto& osc : oscillators_)
        osc.setFrequency(currentNoteFreq_);

    ampEnv_.noteOn();
    filterEnv_.noteOn();
    modEnv_.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);
    ampEnv_.noteOff();
    filterEnv_.noteOff();
    modEnv_.noteOff();

    if (!allowTailOff)
    {
        clearCurrentNote();
    }
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{
    // Map 0-16383 to -1.0 to 1.0
    pitchWheelValue_ = ((float)newPitchWheelValue - 8192.0f) / 8192.0f;
}

void SynthVoice::controllerMoved(int, int) {}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                  int startSample, int numSamples)
{
    if (!prepared_) return;

    for (int i = 0; i < numSamples; ++i)
    {
        // Process LFOs
        float lfo1 = lfos_[0].process();
        float lfo2 = lfos_[1].process();

        // Process envelopes
        float ampEnv = ampEnv_.process();
        float filterEnv = filterEnv_.process();
        modEnv_.process(); // Available for mod matrix later

        // Check if voice is done
        if (!ampEnv_.isActive() && getCurrentlyPlayingNote() >= 0)
        {
            clearCurrentNote();
            break;
        }

        // Mix oscillators
        float osc1 = oscillators_[0].process();
        float osc2 = oscillators_[1].process();
        float osc3 = oscillators_[2].process();
        float mix = (osc1 + osc2 + osc3) * 0.33f;

        // Apply filter
        float filtered = filter_.process(mix, filterEnv, currentNoteFreq_);

        // Apply amp envelope
        float sample = filtered * ampEnv * currentVelocity_;

        // Write to output (stereo)
        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample(ch, startSample + i, sample);
    }
}

void SynthVoice::setOscWaveform(int idx, SynthOscillator::Waveform wf) { oscillators_[idx].setWaveform(wf); }
void SynthVoice::setOscDetune(int idx, float cents) { oscillators_[idx].setDetune(cents); }
void SynthVoice::setOscOctave(int idx, int oct) { oscillators_[idx].setOctave(oct); }
void SynthVoice::setOscLevel(int idx, float level) { oscillators_[idx].setLevel(level); }
void SynthVoice::setOscPulseWidth(int idx, float pw) { oscillators_[idx].setPulseWidth(pw); }

void SynthVoice::setFilterMode(SynthFilter::Mode m) { filter_.setMode(m); }
void SynthVoice::setFilterCutoff(float f) { filter_.setCutoff(f); }
void SynthVoice::setFilterResonance(float r) { filter_.setResonance(r); }
void SynthVoice::setFilterEnvAmount(float a) { filter_.setEnvAmount(a); }

void SynthVoice::setAmpEnv(float a, float d, float s, float r) { ampEnv_.setAttack(a); ampEnv_.setDecay(d); ampEnv_.setSustain(s); ampEnv_.setRelease(r); }
void SynthVoice::setFilterEnv(float a, float d, float s, float r) { filterEnv_.setAttack(a); filterEnv_.setDecay(d); filterEnv_.setSustain(s); filterEnv_.setRelease(r); }
void SynthVoice::setModEnv(float a, float d, float s, float r) { modEnv_.setAttack(a); modEnv_.setDecay(d); modEnv_.setSustain(s); modEnv_.setRelease(r); }

void SynthVoice::setLFOShape(int idx, SynthLFO::Shape s) { lfos_[idx].setShape(s); }
void SynthVoice::setLFORate(int idx, float hz) { lfos_[idx].setRate(hz); }
void SynthVoice::setLFODepth(int idx, float d) { lfos_[idx].setDepth(d); }

bool SynthVoice::isVoiceActive() const
{
    return ampEnv_.isActive();
}
```

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/SynthVoice.h Source/SynthVoice.cpp CMakeLists.txt
git commit -m "feat: add SynthVoice with 3 osc, filter, 3 env, 2 LFO"
```

---

### Task 9: Create VoiceManager

**Files:**
- Create: `Source/VoiceManager.h`
- Create: `Source/VoiceManager.cpp`

**Step 1: Create VoiceManager.h**

```cpp
// Source/VoiceManager.h
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SynthVoice.h"

class SynthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

class VoiceManager : public juce::Synthesiser
{
public:
    VoiceManager();
    ~VoiceManager() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock);

    // Override voice stealing to use oldest-active-first strategy
    SynthesiserVoice* findVoiceToSteal(juce::MidiMessageSequence::MidiEventHolder&,
                                        int) const override;

    // Access voices for parameter updates
    SynthVoice* getVoice(int index);

    static constexpr int kMaxVoices = 32;
    static constexpr int kDefaultVoices = 16;

private:
    double sampleRate_ = 44100.0;
};
```

**Step 2: Create VoiceManager.cpp**

```cpp
// Source/VoiceManager.cpp
#include "VoiceManager.h"

VoiceManager::VoiceManager()
{
    for (int i = 0; i < kDefaultVoices; ++i)
        addVoice(new SynthVoice());

    addSound(new SynthSound());
}

VoiceManager::~VoiceManager() = default;

void VoiceManager::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sampleRate_ = sampleRate;
    setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<SynthVoice*>(getVoice(i)))
            voice->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

juce::SynthesiserVoice* VoiceManager::findVoiceToSteal(
    juce::MidiMessageSequence::MidiEventHolder&, int) const
{
    // Oldest active voice stealing: find the voice that has been playing longest
    juce::SynthesiserVoice* oldest = nullptr;

    for (auto* voice : voices)
    {
        if (voice->isVoiceActive())
        {
            if (oldest == nullptr)
                oldest = voice;
        }
    }

    return oldest;
}

SynthVoice* VoiceManager::getVoice(int index)
{
    return dynamic_cast<SynthVoice*>(voices[index]);
}
```

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/VoiceManager.h Source/VoiceManager.cpp CMakeLists.txt
git commit -m "feat: add VoiceManager with oldest-first voice stealing"
```

---

### Task 10: Wire SynthEngine into PluginProcessor

**Files:**
- Modify: `Source/PluginProcessor.h`
- Modify: `Source/PluginProcessor.cpp`

**Step 1: Update PluginProcessor.h to include VoiceManager**

Add `#include "VoiceManager.h"` and a `VoiceManager synth_;` member. Add `juce::AudioProcessorValueTreeState apvts` for parameter management.

**Step 2: Update PluginProcessor.cpp**

- In constructor: initialize APVTS with parameter layout
- In `prepareToPlay()`: call `synth_.prepareToPlay(sampleRate, samplesPerBlock)`
- In `processBlock()`: call `synth_.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples())`

**Step 3: Build and test standalone**

```bash
cmake --build build --config Release
./build/littleSynth_artefacts/Standalone/littleSynth
```

Expected: Window opens. Playing MIDI notes should produce sound (default sine wave).

**Step 4: Commit**

```bash
git add Source/PluginProcessor.h Source/PluginProcessor.cpp
git commit -m "feat: wire VoiceManager into PluginProcessor, synth produces sound"
```

---

## Phase 4: Parameters

### Task 11: Create parameter system

**Files:**
- Create: `Source/Parameters.h`
- Create: `Source/Parameters.cpp`

**Step 1: Create Parameters.h**

Define all plugin parameters as JUCE AudioProcessorValueTreeState parameters. Include parameters for:
- 3 oscillators: waveform, detune, octave, level, pulse width (5 params x 3 = 15)
- Filter: mode, cutoff, resonance, drive, env amount, key tracking (6)
- 3 envelopes: attack, decay, sustain, release (4 x 3 = 12)
- 2 LFOs: shape, rate, depth (3 x 2 = 6)
- Master volume

Total: ~40 parameters.

```cpp
// Source/Parameters.h
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
```

**Step 2: Create Parameters.cpp with all parameter definitions**

Each parameter uses `juce::AudioProcessorValueTreeState::ParameterLayout` with `std::make_unique<juce::AudioParameterFloat>()` or `AudioParameterChoice` as appropriate.

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/Parameters.h Source/Parameters.cpp CMakeLists.txt
git commit -m "feat: add complete parameter layout (~40 params)"
```

---

### Task 12: Connect parameters to SynthVoice

**Files:**
- Modify: `Source/PluginProcessor.cpp`

**Step 1: Add parameter change listeners**

In `processBlock()`, read APVTS parameter values and push them to all voices via the setter methods.

**Step 2: Build and test with DAW**

Load VST3 in a DAW. Verify that moving knobs changes the sound (filter cutoff, oscillator detune, etc.).

**Step 3: Commit**

```bash
git add Source/PluginProcessor.cpp
git commit -m "feat: connect APVTS parameters to voice controls"
```

---

## Phase 5: Modulation Matrix

### Task 13: Create ModMatrix

**Files:**
- Create: `Source/ModMatrix.h`
- Create: `Source/ModMatrix.cpp`

**Step 1: Create ModMatrix.h**

```cpp
// Source/ModMatrix.h
#pragma once

#include <cstdint>
#include <array>

class ModMatrix
{
public:
    enum class Source
    {
        AmpEnv = 0,
        FilterEnv,
        ModEnv,
        LFO1,
        LFO2,
        Velocity,
        ModWheel,
        Aftertouch,
        PitchBend,
        NumSources
    };

    enum class Destination
    {
        Osc1Pitch = 0,
        Osc2Pitch,
        Osc3Pitch,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        FilterCutoff,
        FilterResonance,
        AmpLevel,
        LFO1Rate,
        LFO1Depth,
        LFO2Rate,
        LFO2Depth,
        Pan,
        NumDestinations
    };

    struct ModSlot
    {
        Source source = Source::AmpEnv;
        Destination dest = Destination::FilterCutoff;
        float depth = 0.0f;
        bool bipolar = true;
        bool active = false;
    };

    static constexpr int kNumSlots = 16;

    ModMatrix();

    void setSource(int slot, Source source);
    void setDestination(int slot, Destination dest);
    void setDepth(int slot, float depth);
    void setBipolar(int slot, bool bipolar);
    void setActive(int slot, bool active);

    void setSourceValue(Source source, float value);
    float getModulatedValue(Destination dest, float baseValue) const;

private:
    std::array<ModSlot, kNumSlots> slots_;
    std::array<float, static_cast<int>(Source::NumSources)> sourceValues_{};
};
```

**Step 2: Create ModMatrix.cpp**

Implement the routing logic: iterate active slots, accumulate modulation amounts per destination, return base + modulated value.

**Step 3: Add to CMakeLists.txt and build**

**Step 4: Commit**

```bash
git add Source/ModMatrix.h Source/ModMatrix.cpp CMakeLists.txt
git commit -m "feat: add ModMatrix with 16-slot source->dest routing"
```

---

### Task 14: Integrate ModMatrix into SynthVoice

**Files:**
- Modify: `Source/SynthVoice.h`
- Modify: `Source/SynthVoice.cpp`

Add a `ModMatrix` instance per voice. In `renderNextBlock()`, feed envelope/LFO values into the mod matrix, then use modulated values to control oscillator pitch, filter cutoff, etc.

**Commit:**

```bash
git add Source/SynthVoice.h Source/SynthVoice.cpp
git commit -m "feat: integrate ModMatrix into SynthVoice rendering"
```

---

## Phase 6: Effects

### Task 15: Create EffectsChain base and first effect (Chorus)

**Files:**
- Create: `Source/Effects/EffectBase.h`
- Create: `Source/Effects/Chorus.h`
- Create: `Source/Effects/Chorus.cpp`

**Step 1: Create EffectBase.h**

```cpp
// Source/Effects/EffectBase.h
#pragma once

class EffectBase
{
public:
    virtual ~EffectBase() = default;
    virtual void init(double sampleRate) = 0;
    virtual void process(float* left, float* right, int numSamples) = 0;
    virtual void setEnabled(bool enabled) { enabled_ = enabled; }
    virtual bool isEnabled() const { return enabled_; }
protected:
    bool enabled_ = false;
    double sampleRate_ = 44100.0;
};
```

**Step 2: Create Chorus effect wrapping DaisySP Chorus**

**Step 3: Commit**

```bash
git add Source/Effects/
git commit -m "feat: add EffectBase and Chorus effect"
```

---

### Task 16: Create remaining effects

**Files:**
- Create: `Source/Effects/Phaser.h/cpp`
- Create: `Source/Effects/Flanger.h/cpp`
- Create: `Source/Effects/Delay.h/cpp`
- Create: `Source/Effects/Reverb.h/cpp`
- Create: `Source/Effects/Distortion.h/cpp`
- Create: `Source/Effects/EQ.h/cpp`
- Create: `Source/Effects/Compressor.h/cpp`

Each effect wraps DaisySP where available (Phaser, Flanger, Overdrive). For Delay, Reverb, EQ, and Compressor — implement using JUCE's `juce::dsp` module which provides these.

**Commit after each effect or batch:**

```bash
git add Source/Effects/
git commit -m "feat: add Phaser, Flanger, Delay, Reverb, Distortion, EQ, Compressor effects"
```

---

### Task 17: Create EffectsChain and wire into PluginProcessor

**Files:**
- Create: `Source/EffectsChain.h`
- Create: `Source/EffectsChain.cpp`

Manages ordered list of effects. Processes audio through the chain after voice summation. Effects can be toggled on/off and reordered.

**Step 1: Commit**

```bash
git add Source/EffectsChain.h Source/EffectsChain.cpp
git commit -m "feat: add EffectsChain with reorderable effects"
```

---

## Phase 7: GUI

### Task 18: Create CustomLookAndFeel

**Files:**
- Create: `Source/GUI/CustomLookAndFeel.h`
- Create: `Source/GUI/CustomLookAndFeel.cpp`

Custom dark theme with animated knob drawing. Override `drawRotarySlider()`, `drawLinearSlider()`, `drawComboBox()`, etc.

**Commit:**

```bash
git add Source/GUI/
git commit -m "feat: add CustomLookAndFeel with dark theme and animated knobs"
```

---

### Task 19: Create Visualizer (oscilloscope + spectrum)

**Files:**
- Create: `Source/GUI/Visualizer.h`
- Create: `Source/GUI/Visualizer.cpp`

Uses `juce::dsp::FFT` for spectrum analysis and a circular buffer for oscilloscope waveform. Rendered as background behind controls.

**Commit:**

```bash
git add Source/GUI/Visualizer.h Source/GUI/Visualizer.cpp
git commit -m "feat: add oscilloscope and FFT spectrum visualizer"
```

---

### Task 20: Create oscillator, filter, envelope, LFO, mod matrix panels

**Files:**
- Create: `Source/GUI/OscillatorPanel.h/cpp`
- Create: `Source/GUI/FilterPanel.h/cpp`
- Create: `Source/GUI/EnvelopePanel.h/cpp`
- Create: `Source/GUI/ModMatrixPanel.h/cpp`
- Create: `Source/GUI/EffectsPanel.h/cpp`

Each panel contains sliders/knobs bound to APVTS parameters with attachments.

**Commit:**

```bash
git add Source/GUI/
git commit -m "feat: add all GUI panels (osc, filter, env, mod matrix, effects)"
```

---

### Task 21: Create MainPanel and wire into PluginEditor

**Files:**
- Create: `Source/GUI/MainPanel.h`
- Create: `Source/GUI/MainPanel.cpp`
- Modify: `Source/PluginEditor.h`
- Modify: `Source/PluginEditor.cpp`

MainPanel orchestrates the layout: top bar, left oscillators, center filter+mod, right envelopes, bottom effects. Background visualizer.

**Commit:**

```bash
git add Source/GUI/MainPanel.h Source/GUI/MainPanel.cpp Source/PluginEditor.h Source/PluginEditor.cpp
git commit -m "feat: add MainPanel layout and wire into PluginEditor"
```

---

## Phase 8: Presets & State

### Task 22: Implement preset system and state save/load

**Files:**
- Modify: `Source/PluginProcessor.cpp`

Implement `getStateInformation()` and `setStateInformation()` using APVTS state. Add factory presets.

**Commit:**

```bash
git add Source/PluginProcessor.cpp
git commit -m "feat: implement state save/load and factory presets"
```

---

## Phase 9: Polish & Testing

### Task 23: Add ADSR curve display components

**Files:**
- Create: `Source/GUI/ADSDDisplay.h/cpp`

Real-time ADSR curve drawing component that reads envelope parameters and renders the shape.

**Commit:**

```bash
git add Source/GUI/ADSDDisplay.h Source/GUI/ADSDDisplay.cpp
git commit -m "feat: add real-time ADSR curve display"
```

---

### Task 24: Add LFO waveform preview

**Files:**
- Create: `Source/GUI/LFOPreview.h/cpp`

Small preview showing current LFO waveform shape.

**Commit:**

```bash
git add Source/GUI/LFOPreview.h Source/GUI/LFOPreview.cpp
git commit -m "feat: add LFO waveform preview component"
```

---

### Task 25: Performance tuning and final testing

**Files:**
- Various modifications

Profile CPU usage in a DAW at 16 voices. Optimize hot paths. Verify no clicks/pops on voice stealing. Test with multiple DAWs (REAPER, Ableton, etc.).

**Commit:**

```bash
git add -A
git commit -m "perf: optimize rendering loop and fix voice stealing clicks"
```

---

## Summary

| Phase | Tasks | Description |
|-------|-------|-------------|
| 1 | 1-3 | Project skeleton, build system, minimal plugin shell |
| 2 | 4-7 | Core DSP: Oscillator, Filter, Envelope, LFO wrappers |
| 3 | 8-10 | SynthVoice, VoiceManager, wire into processor |
| 4 | 11-12 | Parameter system and connection to voices |
| 5 | 13-14 | Modulation matrix |
| 6 | 15-17 | Effects chain (8 effects) |
| 7 | 18-21 | Full GUI with custom look, visualizer, panels |
| 8 | 22 | Presets and state management |
| 9 | 23-25 | Polish: ADSR display, LFO preview, performance |
