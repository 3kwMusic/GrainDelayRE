# SPDX-FileCopyrightText: 2026 TriM
# SPDX-License-Identifier: AGPL-3.0-only

cmake_minimum_required(VERSION 3.22)
project(GrainParameterAudit VERSION 1.0.0)
set(JUCE_MODULES_ONLY ON CACHE BOOL "Only JUCE modules are required for this console audit" FORCE)
include(../../cmake/JuceDependency.cmake)
grain_add_juce()
add_executable(GrainParameterAudit)
target_sources(GrainParameterAudit PRIVATE
    ParameterAudit.cpp
    ../../Source/Parameters.cpp
    ../../Source/RingBuffer.cpp
    ../../Source/GrainEngine.cpp
    ../../Source/PluginProcessor.cpp
    ../../Source/ModernLookAndFeel.cpp
    ../../Source/XYPad.cpp
    ../../Source/PluginEditor.cpp)
target_compile_features(GrainParameterAudit PRIVATE cxx_std_17)

include(../../cmake/LegalNotices.cmake)
grain_target_legal_notices(GrainParameterAudit)
target_compile_definitions(GrainParameterAudit PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 JUCE_STANDALONE_APPLICATION=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Grain Delay RE")
target_link_libraries(GrainParameterAudit PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
