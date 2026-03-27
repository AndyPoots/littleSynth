# littleSynth — VST3 Subtractive Synthesizer Design

## Overview

A fully-featured polyphonic subtractive synthesizer built with JUCE 8 and DaisySP, targeting VST3 format with a custom animated GUI, full modulation matrix, and comprehensive effects suite.

## Tech Stack

- **Framework:** JUCE 8 (CMake build system)
- **DSP Library:** DaisySP (MIT license)
- **Language:** C++17
- **Plugin Formats:** VST3 (primary), Standalone (for testing)
- **Build System:** CMake

## Architecture

```
PluginProcessor (JUCE AudioProcessor)
├── SynthEngine
│   ├── VoiceManager (polyphony + voice stealing)
│   │   └── SynthVoice (16-32 instances)
│   │       ├── Oscillator x3 (DaisySP wrappers)
│   │       ├── Filter (multi-mode, DaisySP)
│   │       ├── Envelope x3 (ADSR, DaisySP)
│   │       └── LFO x2
│   ├── ModMatrix (16-slot source->dest routing)
│   └── EffectsChain
│       ├── EQ (3-band parametric)
│       ├── Compressor
│       ├── Chorus
│       ├── Phaser
│       ├── Flanger
│       ├── Delay (tempo-sync, ping-pong)
│       ├── Reverb
│       └── Distortion (multi-mode)
└── PluginEditor (GUI)
    ├── MainPanel
    ├── OscillatorPanel x3
    ├── FilterPanel
    ├── EnvelopePanel x3
    ├── LFOPanel x2
    ├── ModMatrixPanel
    ├── EffectsPanel
    ├── Visualizer (oscilloscope + spectrum analyzer)
    └── CustomLookAndFeel (animated knobs, dark theme)
```

## DSP Engine

### Oscillators (3 per voice)

- Waveforms: sine, triangle, sawtooth, square, noise
- Per-osc controls: waveform select, detune (cents), octave shift, level
- Osc 2 & 3: sync-to-osc1 option, pulse width for square
- Anti-aliased via DaisySP (PolyBLEP/BLIT)

### Multi-mode Filter

- Modes: lowpass, highpass, bandpass, notch
- Slopes: 12 dB/oct, 24 dB/oct
- Controls: cutoff, resonance, drive, env amount, key tracking
- Algorithm: Moog-style ladder (DaisySP)

### Envelopes

- 3 ADSR envelopes: amp, filter, free/mod
- Controls per envelope: attack, decay, sustain, release, curve shape
- DaisySP ADSR implementation

### LFOs

- 2 LFOs with shapes: sine, triangle, saw, square, S&H, random
- Rate, depth, sync-to-tempo options

### Modulation Matrix

- 16 modulation slots
- Sources: 3 ADSRs, 2 LFOs, velocity, mod wheel, aftertouch, pitch bend, breath controller
- Destinations: osc pitch, osc level, filter cutoff, filter resonance, amp level, LFO rate, LFO depth, pan
- Per-slot: source, destination, depth, polarity (unipolar/bipolar)

### Voice Management

- 16 voices default (configurable up to 32)
- Voice stealing: oldest-active-first with 5ms fast release
- Priority: last-note

## Signal Flow

```
Osc1 + Osc2 + Osc3 → Pre-filter mix → Filter → Amp Envelope → Voice output
                                                                         ↓
All voices summed → EQ → Compressor → Chorus → Phaser → Flanger → Delay → Reverb → Distortion → Output
```

Effects are toggleable and reorderable in the chain.

## Effects

| Effect | Key Parameters |
|--------|---------------|
| EQ | 3-band parametric (freq, gain, Q) |
| Compressor | Threshold, ratio, attack, release, makeup gain |
| Chorus | Rate, depth, mix, voice count |
| Phaser | Rate, depth, feedback, stages (4/6/8) |
| Flanger | Rate, depth, feedback, mix |
| Delay | Time (tempo-sync), feedback, ping-pong, mix |
| Reverb | Room size, damping, wet/dry, pre-delay |
| Distortion | Drive, tone, mix (soft clip, hard clip, waveshaper) |

## GUI

### Layout

- **Top bar:** Preset browser, settings
- **Left:** 3 oscillator panels (stacked)
- **Center:** Filter display + mod matrix
- **Right:** Envelope displays + LFO panels
- **Bottom:** Effects rack (horizontal strip)
- **Background:** Full-width oscilloscope + spectrum analyzer

### Visual Elements

- Custom knobs with value arc and animated rotation
- Real-time ADSR curve displays
- LFO waveform previews
- FFT spectrum analyzer
- Oscilloscope waveform display
- Animated filter response curve
- Modulation routing visualization
- Dark theme with accent colors per section
- Smooth 60fps animations
- Resizeable window with minimum size constraint

## Project Structure

```
littleSynth/
├── CMakeLists.txt
├── Source/
│   ├── PluginProcessor.h/cpp
│   ├── PluginEditor.h/cpp
│   ├── SynthEngine.h/cpp
│   ├── VoiceManager.h/cpp
│   ├── SynthVoice.h/cpp
│   ├── Oscillator.h/cpp
│   ├── Filter.h/cpp
│   ├── Envelope.h/cpp
│   ├── LFO.h/cpp
│   ├── ModMatrix.h/cpp
│   ├── Effects/
│   │   ├── Chorus.h/cpp
│   │   ├── Phaser.h/cpp
│   │   ├── Flanger.h/cpp
│   │   ├── Delay.h/cpp
│   │   ├── Reverb.h/cpp
│   │   ├── Distortion.h/cpp
│   │   ├── EQ.h/cpp
│   │   └── Compressor.h/cpp
│   ├── GUI/
│   │   ├── MainPanel.h/cpp
│   │   ├── OscillatorPanel.h/cpp
│   │   ├── FilterPanel.h/cpp
│   │   ├── EnvelopePanel.h/cpp
│   │   ├── ModMatrixPanel.h/cpp
│   │   ├── EffectsPanel.h/cpp
│   │   ├── Visualizer.h/cpp
│   │   └── CustomLookAndFeel.h/cpp
│   └── Parameters.h/cpp
└── ThirdParty/
    ├── JUCE/
    └── DaisySP/
```

## Development Approach

JUCE + existing DSP libraries (DaisySP). Build modularly — core sound engine first, then effects, then GUI polish. Use DaisySP for proven DSP building blocks while building the modulation matrix, voice management, and GUI from scratch with JUCE.
