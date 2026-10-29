// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include "GrainEngine.h"
#include "Parameters.h"

class GrainDelayProcessor final : public juce::AudioProcessor
{
public:
    GrainDelayProcessor();
    ~GrainDelayProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 60.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    void setXYGestureActive(bool xAxis, bool active);

private:
    float raw(const juce::String& id) const;
    Parameters::Target target(const juce::String& id) const;
    GrainEngine::Settings currentSettings();

    GrainEngine engine;
    std::atomic<bool> xyGestureActiveX { false };
    std::atomic<bool> xyGestureActiveY { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainDelayProcessor)
};
