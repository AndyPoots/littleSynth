// Source/Parameters.cpp
#include "Parameters.h"

// Helper: create a float parameter with a skewed range
static std::unique_ptr<juce::AudioParameterFloat> makeSkewedFloat(
    const juce::String& id, const juce::String& name,
    float min, float max, float defaultVal, float skew)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(id, 1), name,
        juce::NormalisableRange<float>(min, max, 0.001f, skew),
        defaultVal);
}

// Helper: create a float parameter with linear range
static std::unique_ptr<juce::AudioParameterFloat> makeLinearFloat(
    const juce::String& id, const juce::String& name,
    float min, float max, float defaultVal)
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(id, 1), name,
        juce::NormalisableRange<float>(min, max, 0.001f),
        defaultVal);
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // -----------------------------------------------------------------------
    // Oscillator parameters (3 oscillators: osc1_, osc2_, osc3_)
    // -----------------------------------------------------------------------

    const char* oscPrefixes[] = { "osc1_", "osc2_", "osc3_" };
    const float oscDefaultLevels[] = { 1.0f, 0.0f, 0.0f };

    for (int i = 0; i < 3; ++i)
    {
        juce::String p = oscPrefixes[i];

        // Waveform: choice
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(p + "waveform", 1),
            p + "Waveform",
            juce::StringArray({ "Sine", "Triangle", "Sawtooth", "Square", "Noise" }),
            0));  // default: Sine

        // Detune: -100 to +100 cents
        params.push_back(makeLinearFloat(
            p + "detune", p + "Detune",
            -100.0f, 100.0f, 0.0f));

        // Octave: -2 to +2
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(p + "octave", 1),
            p + "Octave",
            juce::StringArray({ "-2", "-1", "0", "+1", "+2" }),
            2));  // default: 0 (index 2 in the array)

        // Level: 0 to 1
        params.push_back(makeLinearFloat(
            p + "level", p + "Level",
            0.0f, 1.0f, oscDefaultLevels[i]));

        // Pulse width: 0.01 to 0.99
        params.push_back(makeLinearFloat(
            p + "pulse_width", p + "Pulse Width",
            0.01f, 0.99f, 0.5f));

        // On/Off toggle
        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(p + "on", 1),
            p + "On",
            i == 0));  // default: osc1 on, osc2/3 off
    }

    // -----------------------------------------------------------------------
    // Filter parameters
    // -----------------------------------------------------------------------

    // Filter mode
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter_mode", 1), "Filter Mode",
        juce::StringArray({ "Lowpass", "Highpass", "Bandpass", "Notch" }),
        0));

    // Filter slope
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("filter_slope", 1), "Filter Slope",
        juce::StringArray({ "12 dB/oct", "24 dB/oct" }),
        1));  // default: 24 dB/oct

    // Filter cutoff: 20 to 20000 Hz, skewed (log-like)
    params.push_back(makeSkewedFloat(
        "filter_cutoff", "Filter Cutoff",
        20.0f, 20000.0f, 5000.0f, 0.5f));

    // Filter resonance: 0 to 1
    params.push_back(makeLinearFloat(
        "filter_resonance", "Filter Resonance",
        0.0f, 1.0f, 0.0f));

    // Filter drive: 0.1 to 5
    params.push_back(makeLinearFloat(
        "filter_drive", "Filter Drive",
        0.1f, 5.0f, 1.0f));

    // Filter envelope amount: -1 to 1
    params.push_back(makeLinearFloat(
        "filter_env_amount", "Filter Env Amount",
        -1.0f, 1.0f, 0.0f));

    // Filter key tracking: 0 to 1
    params.push_back(makeLinearFloat(
        "filter_key_tracking", "Filter Key Tracking",
        0.0f, 1.0f, 0.0f));

    // -----------------------------------------------------------------------
    // Envelope parameters (3 envelopes: amp_env_, filter_env_, mod_env_)
    // -----------------------------------------------------------------------

    struct EnvDef
    {
        const char* prefix;
        float defaultSustain;
    };

    const EnvDef envelopes[] = {
        { "amp_env_",    0.7f },
        { "filter_env_", 0.0f },
        { "mod_env_",    0.0f }
    };

    for (const auto& env : envelopes)
    {
        auto p = juce::String(env.prefix);

        // Attack: 0.001 to 10 seconds, skewed
        params.push_back(makeSkewedFloat(
            p + "attack", p + "Attack",
            0.001f, 10.0f, 0.01f, 0.5f));

        // Decay: 0.001 to 10 seconds, skewed
        params.push_back(makeSkewedFloat(
            p + "decay", p + "Decay",
            0.001f, 10.0f, 0.1f, 0.5f));

        // Sustain: 0 to 1
        params.push_back(makeLinearFloat(
            p + "sustain", p + "Sustain",
            0.0f, 1.0f, env.defaultSustain));

        // Release: 0.001 to 10 seconds, skewed
        params.push_back(makeSkewedFloat(
            p + "release", p + "Release",
            0.001f, 10.0f, 0.3f, 0.5f));
    }

    // -----------------------------------------------------------------------
    // LFO parameters (2 LFOs: lfo1_, lfo2_)
    // -----------------------------------------------------------------------

    const char* lfoPrefixes[] = { "lfo1_", "lfo2_" };

    for (int i = 0; i < 2; ++i)
    {
        auto p = juce::String(lfoPrefixes[i]);

        // Shape
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(p + "shape", 1),
            p + "Shape",
            juce::StringArray({ "Sine", "Triangle", "Sawtooth", "Square", "S&H", "Random" }),
            0));

        // Rate: 0.01 to 30 Hz, skewed
        params.push_back(makeSkewedFloat(
            p + "rate", p + "Rate",
            0.01f, 30.0f, 1.0f, 0.5f));

        // Depth: 0 to 1
        params.push_back(makeLinearFloat(
            p + "depth", p + "Depth",
            0.0f, 1.0f, 0.0f));
    }

    // -----------------------------------------------------------------------
    // Master parameters
    // -----------------------------------------------------------------------

    params.push_back(makeLinearFloat(
        "master_level", "Master Level",
        0.0f, 1.0f, 0.7f));

    return { params.begin(), params.end() };
}
