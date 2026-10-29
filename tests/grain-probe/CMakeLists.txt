cmake_minimum_required(VERSION 3.22)
project(GrainEngineProbe VERSION 1.0.0)
include(../../cmake/JuceDependency.cmake)
grain_add_juce()
juce_add_console_app(GrainEngineProbe)
target_sources(GrainEngineProbe PRIVATE
    EngineProbe.cpp
    ../../Source/GrainEngine.cpp
    ../../Source/RingBuffer.cpp)
target_compile_features(GrainEngineProbe PRIVATE cxx_std_17)
target_compile_definitions(GrainEngineProbe PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(GrainEngineProbe PRIVATE juce::juce_audio_basics juce::juce_recommended_config_flags)
