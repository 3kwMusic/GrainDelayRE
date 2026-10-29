// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#include "RingBuffer.h"

void RingBuffer::prepare(int maximumSamples)
{
    data.setSize(1, juce::jmax(8, maximumSamples));
    reset();
}

void RingBuffer::reset()
{
    data.clear();
    writePosition = 0;
}

void RingBuffer::write(float sample) noexcept
{
    data.setSample(0, writePosition, sample);
}

void RingBuffer::advance() noexcept
{
    writePosition = wrap(writePosition + 1);
}

float RingBuffer::readLinear(float delaySamples) const noexcept
{
    const auto delay = juce::jmax(0.0f, delaySamples);
    const auto integerDelay = static_cast<int>(delay);
    const auto integerPosition = writePosition - integerDelay;
    const auto fraction = delay - static_cast<float>(integerDelay);

    const auto newer = data.getSample(0, wrap(integerPosition));
    const auto older = data.getSample(0, wrap(integerPosition - 1));
    return newer + fraction * (older - newer);
}

int RingBuffer::wrap(int index) const noexcept
{
    const auto size = data.getNumSamples();
    index %= size;
    return index < 0 ? index + size : index;
}
