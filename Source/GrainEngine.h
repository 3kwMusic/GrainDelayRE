// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include "RingBuffer.h"

class GrainEngine
{
public:
    struct Settings
    {
        float frequency = 20.0f;
        float pitch = 0.0f;
        float randomPitch = 0.0f;
        float delayMs = 125.0f;
        float sprayMs = 20.0f;
        float feedback = 0.5f;
        float dryWet = 0.5f;
        float swing = 0.0f;
    };

    void prepare(double newSampleRate, const Settings& initialSettings);
    void reset();
    void setSettings(const Settings& newSettings);

    void process(const float* inputLeft,
                 const float* inputRight,
                 float* outputLeft,
                 float* outputRight,
                 int numSamples);

private:
    struct GrainTap
    {
        float slopeMs = 0.0f;
        float offsetMs = 0.0f;
    };

    void initialisePhaseTable();
    float randomizeSlope(float frequency, float pitchRatio);
    void randomizeTap(GrainTap& tap, float frequency, float pitchRatio);
    float renderTap(const GrainTap& tap, const RingBuffer& buffer, float phase) const;
    float renderCrossfade(float phase, float a, float b) const;

    Settings settings;
    RingBuffer leftBuffer;
    RingBuffer rightBuffer;
    std::array<GrainTap, 4> taps;
    std::array<float, 513> phaseTable;

    juce::SmoothedValue<float> frequencySmoothed;
    juce::SmoothedValue<float> feedbackSmoothed;
    juce::SmoothedValue<float> mixSmoothed;
    juce::Random random;

    double sampleRate = 44100.0;
    float phasePosition = 0.0f;
    float previousWetRight = 0.0f;
    juce::uint32 stage = 0;
    juce::uint32 swingCounter = 0;

    friend struct GrainEngineProbeAccess;
};
