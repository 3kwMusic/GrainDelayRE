# SPDX-FileCopyrightText: 2026 TriM
# SPDX-License-Identifier: AGPL-3.0-only

cmake_minimum_required(VERSION 3.22)

project(GrainDelayRE VERSION 1.0.3)

include(cmake/JuceDependency.cmake)
grain_add_juce()

juce_add_plugin(GrainDelayRE
    COMPANY_NAME "TriM"
    COMPANY_WEBSITE "https://github.com/3kwMusic/GrainDelayRE"
    BUNDLE_ID "com.yourcompany.GrainDelayRE"
    PLUGIN_MANUFACTURER_CODE ReaD
    PLUGIN_CODE Gdr1
    FORMATS VST3 Standalone
    PRODUCT_NAME "Grain Delay RE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE)

target_sources(GrainDelayRE
    PRIVATE
        Source/Parameters.cpp
        Source/RingBuffer.cpp
        Source/GrainEngine.cpp
        Source/PluginProcessor.cpp
        Source/ModernLookAndFeel.cpp
        Source/XYPad.cpp
        Source/PluginEditor.cpp)

target_compile_features(GrainDelayRE PRIVATE cxx_std_17)

include(cmake/LegalNotices.cmake)
grain_target_legal_notices(GrainDelayRE)

target_compile_definitions(GrainDelayRE
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(GrainDelayRE
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
