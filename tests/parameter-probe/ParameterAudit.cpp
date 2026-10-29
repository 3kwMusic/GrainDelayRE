#include "../../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>

namespace
{
void setRaw(GrainDelayProcessor& processor, const juce::String& id, float value)
{
    auto* parameter = processor.apvts.getParameter(id);
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

int impulsePeak(bool selectDelayTarget)
{
    GrainDelayProcessor processor;
    setRaw(processor, Parameters::syncEnabled, 1.0f);
    setRaw(processor, Parameters::beatDivision, 0.0f);
    setRaw(processor, Parameters::delayMs, 100.0f);
    setRaw(processor, Parameters::pitch, 0.0f);
    setRaw(processor, Parameters::randomPitch, 0.0f);
    setRaw(processor, Parameters::spray, 0.0f);
    setRaw(processor, Parameters::feedback, 0.0f);
    setRaw(processor, Parameters::dryWet, 1.0f);
    if (selectDelayTarget)
        setRaw(processor, Parameters::xTarget, static_cast<float>(Parameters::Target::delay));
    // No host playhead: processor explicitly defaults to 120 BPM.
    processor.prepareToPlay(48000.0, 64);
    constexpr int total = 16384;
    juce::AudioBuffer<float> buffer(2, total);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);
    juce::MidiBuffer midi;
    for (int start = 0; start < total; start += 64)
    {
        juce::AudioBuffer<float> block(buffer.getArrayOfWritePointers(), 2, start, 64);
        processor.processBlock(block, midi);
    }
    float peak = 0.0f;
    int peakIndex = -1;
    for (int i = 0; i < total; ++i)
        if (std::abs(buffer.getSample(0, i)) > peak)
        {
            peak = std::abs(buffer.getSample(0, i));
            peakIndex = i;
        }
    return peakIndex;
}
}

int main()
{
    int failures = 0;
    const auto syncPeak = impulsePeak(false);
    const auto selectedPeak = impulsePeak(true);
    const auto check = [&failures](const char* name, bool passed)
    {
        std::cout << name << ": " << (passed ? "PASS" : "FAIL") << '\n';
        failures += passed ? 0 : 1;
    };
    std::cout << "sync_120bpm_division1: expected_peak=6000 actual_peak=" << syncPeak << '\n';
    check("sync_seconds_to_milliseconds", syncPeak == 6000);
    std::cout << "select_delay_target_without_gesture: before_peak=" << syncPeak
              << " after_peak=" << selectedPeak << " expected_unchanged=1\n";
    check("target_selection_only_highlights", syncPeak == selectedPeak);
    GrainDelayProcessor processor;
    for (const auto& id : { Parameters::feedback, Parameters::dryWet })
    {
        const auto* parameter = processor.apvts.getParameter(id);
        const auto raw = parameter->convertFrom0to1(parameter->getValueForText("50%"));
        std::cout << "percent_text_" << id << ": expected_raw=0.5 actual_raw=" << raw << '\n';
        check("percent_text_round_trip", std::abs(raw - 0.5f) < 0.0001f);
        for (const auto value : { 0.0f, 0.123f, 0.5f, 0.95f })
        {
            const auto text = parameter->getText(parameter->convertTo0to1(value), 64);
            const auto parsed = parameter->convertFrom0to1(parameter->getValueForText(text));
            check("percent_display_parse_round_trip", std::abs(parsed - value) < 0.0001f);
        }
        const auto bare = parameter->convertFrom0to1(parameter->getValueForText("50"));
        check("percent_without_symbol", std::abs(bare - 0.5f) < 0.0001f);
    }
    std::cout << "assertion_failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
