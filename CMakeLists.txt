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
        Source/Oscillator.cpp
        Source/Filter.cpp
        Source/Envelope.cpp
        Source/LFO.cpp
        Source/SynthVoice.cpp
        Source/VoiceManager.cpp
        Source/Parameters.cpp
        Source/ModMatrix.cpp
        Source/Effects/Chorus.cpp
        Source/Effects/Phaser.cpp
        Source/Effects/Flanger.cpp
        Source/Effects/Distortion.cpp
        Source/Effects/Delay.cpp
        Source/Effects/Reverb.cpp
        Source/Effects/EQ.cpp
        Source/Effects/Compressor.cpp
        Source/EffectsChain.cpp
        Source/GUI/CustomLookAndFeel.cpp
        Source/GUI/Visualizer.cpp
        Source/GUI/OscillatorPanel.cpp
        Source/GUI/FilterPanel.cpp
        Source/GUI/EnvelopePanel.cpp
        Source/GUI/LFOPanel.cpp
        Source/GUI/ModMatrixPanel.cpp
        Source/GUI/EffectsPanel.cpp
        Source/GUI/MainPanel.cpp
)

target_compile_definitions(littleSynth
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ALSA=0
        JUCE_JACK=0
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
