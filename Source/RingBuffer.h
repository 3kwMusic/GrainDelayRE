// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

class RingBuffer
{
public:
    void prepare(int maximumSamples);
    void reset();

    void write(float sample) noexcept;
    void advance() noexcept;
    float readLinear(float delaySamples) const noexcept;

private:
    int wrap(int index) const noexcept;

    juce::AudioBuffer<float> data;
    int writePosition = 0;
};
