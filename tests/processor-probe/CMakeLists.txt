cmake_minimum_required(VERSION 3.22)
project(GrainAudioProbe VERSION 1.0.0)
include(../../cmake/JuceDependency.cmake)
grain_add_juce()
juce_add_console_app(GrainAudioProbe)
target_sources(GrainAudioProbe PRIVATE
    AudioProbe.cpp
    ../../Source/Parameters.cpp
    ../../Source/RingBuffer.cpp
    ../../Source/GrainEngine.cpp
    ../../Source/PluginProcessor.cpp
    ../../Source/ModernLookAndFeel.cpp
    ../../Source/XYPad.cpp
    ../../Source/PluginEditor.cpp)
target_compile_features(GrainAudioProbe PRIVATE cxx_std_17)
target_compile_definitions(GrainAudioProbe PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_MODAL_LOOPS_PERMITTED=1 JucePlugin_Name="Grain Delay RE")
target_link_libraries(GrainAudioProbe PRIVATE juce::juce_audio_utils juce::juce_recommended_config_flags)
