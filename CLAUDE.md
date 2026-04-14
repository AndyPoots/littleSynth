# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

Third-party dependencies (JUCE 8, DaisySP) must be cloned into `ThirdParty/` before building:

```bash
git clone https://github.com/juce-framework/JUCE.git ThirdParty/JUCE
git clone https://github.com/electro-smith/DaisySP.git ThirdParty/DaisySP
```

Build the plugin (requires CMake 3.22+, C++17):

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: VST3 plugin and Standalone app, auto-copied after build (`COPY_PLUGIN_AFTER_BUILD`).

## Architecture

littleSynth is a polyphonic subtractive synthesizer VST3 plugin built on JUCE 8 with DaisySP for DSP primitives.

### Signal Flow

```
3 Oscillators → Mix → Filter → Amp Envelope → Voice Output
                                                    ↓
All voices summed → EQ → Compressor → Chorus → Phaser → Flanger → Delay → Reverb → Distortion → Master Out
```

### Core DSP Chain

- **PluginProcessor** (`Source/PluginProcessor.h`) — JUCE `AudioProcessor` subclass. Owns `VoiceManager`, `EffectsChain`, and the `AudioProcessorValueTreeState` for all parameters. Handles preset save/load.
- **VoiceManager** (`Source/VoiceManager.h`) — Extends `juce::Synthesiser`. 16 voices with oldest-active-first stealing.
- **SynthVoice** (`Source/SynthVoice.h`) — Single voice: 3 oscillators, multi-mode filter, 3 ADSR envelopes, 2 LFOs, 16-slot modulation matrix.
- **Parameters** (`Source/Parameters.cpp`) — Defines all ~50+ `AudioProcessorValueTreeState` parameters with IDs and ranges.

### DSP Wrappers (DaisySP → JUCE parameter bindings)

- **Oscillator** — Anti-aliased waveforms (Sine/Tri/Saw/Square/Noise), detune, octave shift, pulse width.
- **Filter** — Ladder (LP) and SVF (HP/BP/Notch) modes, 12/24 dB/oct slopes, drive, key tracking.
- **Envelope** — ADSR wrapper over DaisySP's `Adsr`.
- **LFO** — Multiple shapes including S&H and random.

### Modulation Matrix (`Source/ModMatrix.h`)

16 slots mapping sources (3 envelopes, 2 LFOs, velocity, mod wheel, aftertouch, pitch bend) to destinations (osc pitch/level, filter cutoff/res, amp, LFO rate/depth, pan). Bipolar/unipolar support.

### Effects (`Source/Effects/`)

8 effects in fixed order managed by `EffectsChain`. JUCE `dsp` module used for EQ, Compressor, Delay, Reverb. DaisySP used for Chorus, Phaser, Flanger. Distortion has multiple modes (soft/hard clip, waveshaper).

### GUI (`Source/GUI/`)

- **MainPanel** — Top-level layout orchestrator: osc stack (left), filter + mod matrix (center), envelopes + LFOs (right), effects rack (bottom), oscilloscope/FFT background.
- **CustomLookAndFeel** — Dark theme with animated rotary sliders and value arcs.
- **Visualizer** — Oscilloscope + FFT spectrum analyzer.
- Each panel (OscillatorPanel, FilterPanel, EnvelopePanel, LFOPanel, ModMatrixPanel, EffectsPanel) handles its own attachment to the parameter tree.

### Compatibility

`Source/glibc_compat.c` provides shims for C23 string functions required on systems with GLIBC < 2.38 (e.g., Ardour snap). The plugin is statically linked against the C++ runtime (`-static-libgcc -static-libstdc++`) to avoid host libstdc++ version conflicts.
