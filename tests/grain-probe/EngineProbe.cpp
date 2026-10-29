// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#include "../../Source/GrainEngine.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct GrainEngineProbeAccess
{
    static auto& table(GrainEngine& engine) { return engine.phaseTable; }
    static auto& taps(GrainEngine& engine) { return engine.taps; }
    static auto& position(GrainEngine& engine) { return engine.phasePosition; }
    static auto& stage(GrainEngine& engine) { return engine.stage; }
    static auto& random(GrainEngine& engine) { return engine.random; }
};

void expect(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

void processOne(GrainEngine& engine)
{
    const auto left = 0.0f;
    const auto right = 0.0f;
    auto outputLeft = 0.0f;
    auto outputRight = 0.0f;
    engine.process(&left, &right, &outputLeft, &outputRight, 1);
}

void testRandomPitchSlope()
{
    GrainEngine engine;
    GrainEngine::Settings settings;
    settings.frequency = 10.0f;
    settings.pitch = 0.0f;
    settings.randomPitch = 161.2f;
    settings.delayMs = 5.0f;
    settings.sprayMs = 0.0f;
    settings.feedback = 0.0f;
    settings.dryWet = 1.0f;
    engine.prepare(48000.0, settings);

    auto expectedRandom = GrainEngineProbeAccess::random(engine);
    const auto randomSigned = expectedRandom.nextFloat() * 2.0f - 1.0f;
    GrainEngineProbeAccess::position(engine) = 511.9999f;
    processOne(engine);

    const auto expected = randomSigned * settings.randomPitch / settings.frequency;
    const auto actual = GrainEngineProbeAccess::taps(engine)[1].slopeMs;
    std::cout << "random_pitch_slope expected=" << expected << " actual=" << actual
              << " stage=" << GrainEngineProbeAccess::stage(engine) << '\n';
    expect(std::abs(actual - expected) < 0.0001f,
           "Random Pitch must add directly to the slope numerator without a 1000x scale");
}

void testPhaseTableAndRotation()
{
    GrainEngine engine;
    GrainEngine::Settings settings;
    settings.frequency = 200.0f;
    settings.sprayMs = 250.0f;
    settings.feedback = 0.0f;
    settings.dryWet = 1.0f;
    engine.prepare(48000.0, settings);

    const auto& table = GrainEngineProbeAccess::table(engine);
    expect(table.size() == 513, "Phase table must contain 512 entries plus one sentinel");
    for (size_t i = 0; i < table.size(); ++i)
    {
        const auto expected = static_cast<float>(i) * 0.00048828125f - 0.5f;
        expect(table[i] == expected, "Phase table value mismatch");
    }
    expect(table[0] == -0.5f, "Phase table start mismatch");
    expect(table[512] == -0.25f, "Phase table sentinel mismatch");

    for (int wrap = 0; wrap < 4; ++wrap)
    {
        const auto expectedStage = static_cast<juce::uint32>((wrap + 1) & 3);
        GrainEngineProbeAccess::position(engine) = 511.99f;
        std::array<float, 4> offsets;
        for (size_t i = 0; i < offsets.size(); ++i)
            offsets[i] = GrainEngineProbeAccess::taps(engine)[i].offsetMs;

        processOne(engine);
        expect(GrainEngineProbeAccess::stage(engine) == expectedStage, "Stage rotation mismatch");
        const auto expectedPosition = 511.99f + 4.0f * settings.frequency * (512.0f / 48000.0f) - 512.0f;
        expect(std::abs(GrainEngineProbeAccess::position(engine) - expectedPosition) < 0.0001f,
               "Wrapped table index did not retain the fractional position");

        for (size_t i = 0; i < offsets.size(); ++i)
        {
            if (i == expectedStage)
                expect(GrainEngineProbeAccess::taps(engine)[i].offsetMs != offsets[i],
                       "Current stage tap was not randomized");
            else
                expect(GrainEngineProbeAccess::taps(engine)[i].offsetMs == offsets[i],
                       "A different tap was randomized");
        }
    }
    expect(GrainEngineProbeAccess::stage(engine) == 0, "Stage did not wrap to zero");
}

void testFractionalReadsAndWrap()
{
    RingBuffer ring;
    ring.prepare(32);
    for (int i = 0; i <= 64; ++i)
    {
        ring.write(static_cast<float>(i));
        if (i != 64)
            ring.advance();
    }
    for (const auto delay : { 0.0f, 0.01f, 0.25f, 0.5f, 0.99f, 1.0f, 2.25f, 2.5f, 2.75f, 3.0f, 30.75f })
        expect(std::abs(ring.readLinear(delay) - (64.0f - delay)) < 0.00001f,
               "Fractional read or ring wrap selected the wrong samples");
}

void testNaturalStageRateAndPitch()
{
    for (const auto rate : { 44100.0, 48000.0, 96000.0 })
    {
        GrainEngine::Settings settings;
        settings.frequency = 20.0f;
        settings.delayMs = 200.0f;
        settings.sprayMs = 0.0f;
        settings.randomPitch = 0.0f;
        settings.feedback = 0.0f;
        settings.dryWet = 1.0f;
        GrainEngine stageEngine;
        stageEngine.prepare(rate, settings);
        int transitions = 0;
        auto previous = GrainEngineProbeAccess::stage(stageEngine);
        for (int i = 0; i < static_cast<int>(rate); ++i)
        {
            processOne(stageEngine);
            const auto current = GrainEngineProbeAccess::stage(stageEngine);
            transitions += current != previous ? 1 : 0;
            previous = current;
        }
        expect(std::abs(transitions - 80) <= 1, "Natural grain rotation rate is incorrect");

        for (const auto pitch : { -12.0f, 12.0f })
        {
            settings.pitch = pitch;
            GrainEngine engine;
            engine.prepare(rate, settings);
            const auto total = static_cast<int>(rate * 3.0);
            std::vector<float> input(total), left(total), right(total);
            for (int i = 0; i < total; ++i)
                input[i] = static_cast<float>(std::sin(juce::MathConstants<double>::twoPi * 480.0 * i / rate));
            // An input with integer tap-period spacing avoids phase cancellation in this test.
            for (int start = 0; start < total; start += 257)
            {
                const auto size = std::min(257, total - start);
                engine.process(input.data() + start, input.data() + start,
                               left.data() + start, right.data() + start, size);
            }
            const auto expectedHz = 480.0 * std::pow(2.0, pitch / 12.0);
            for (const auto* output : { &left, &right })
            {
                int count = 0;
                double first = 0.0, last = 0.0;
                for (int i = static_cast<int>(rate) + 1; i < total; ++i)
                    if ((*output)[i - 1] <= 0.0f && (*output)[i] > 0.0f)
                    {
                        last = i - 1 + static_cast<double>(-(*output)[i - 1]) / ((*output)[i] - (*output)[i - 1]);
                        if (count++ == 0)
                            first = last;
                    }
                expect(count > 2, "Pitch test produced no periodic output");
                const auto measuredHz = (count - 1) * rate / (last - first);
                std::cout << "pitch rate=" << rate << " semitones=" << pitch
                          << " expectedHz=" << expectedHz << " actualHz=" << measuredHz << '\n';
                expect(std::abs(measuredHz - expectedHz) < 0.1, "Actual audio pitch ratio is incorrect");
            }
        }
        std::cout << "natural_stage_rate sampleRate=" << rate << " transitions=" << transitions << '\n';
    }
}

void testImpulseAndDryPath()
{
    GrainEngine engine;
    GrainEngine::Settings settings;
    settings.frequency = 20.0f;
    settings.delayMs = 100.0f;
    settings.sprayMs = 0.0f;
    settings.feedback = 0.0f;
    settings.dryWet = 1.0f;
    engine.prepare(48000.0, settings);

    juce::AudioBuffer<float> buffer(2, 10000);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);
    engine.process(buffer.getReadPointer(0), buffer.getReadPointer(1),
                   buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());

    auto peak = 0.0f;
    auto peakIndex = -1;
    for (int i = 1; i < buffer.getNumSamples(); ++i)
    {
        const auto value = std::abs(buffer.getSample(0, i));
        if (value > peak)
        {
            peak = value;
            peakIndex = i;
        }
    }
    expect(peakIndex == 4800, "100 ms impulse did not arrive at sample 4800");
    expect(std::abs(peak - 1.0f) < 0.0001f, "Wet impulse gain mismatch");
    expect(buffer.getSample(0, 0) == 0.0f, "100% wet leaked dry input");

    auto rightPeak = 0.0f;
    auto rightPeakIndex = -1;
    for (int i = 1; i < buffer.getNumSamples(); ++i)
    {
        const auto value = std::abs(buffer.getSample(1, i));
        if (value > rightPeak)
        {
            rightPeak = value;
            rightPeakIndex = i;
        }
    }
    expect(rightPeakIndex == 4800, "Right-channel impulse did not arrive at sample 4800");
    expect(std::abs(rightPeak - 1.0f) < 0.0001f, "Right-channel wet impulse gain mismatch");

    GrainEngine dryEngine;
    settings.dryWet = 0.0f;
    dryEngine.prepare(48000.0, settings);
    juce::AudioBuffer<float> dryBuffer(2, 64);
    for (int channel = 0; channel < 2; ++channel)
        for (int i = 0; i < dryBuffer.getNumSamples(); ++i)
            dryBuffer.setSample(channel, i, std::sin(0.1f * static_cast<float>(i)));
    dryEngine.process(dryBuffer.getReadPointer(0), dryBuffer.getReadPointer(1),
                      dryBuffer.getWritePointer(0), dryBuffer.getWritePointer(1), dryBuffer.getNumSamples());
    for (int channel = 0; channel < 2; ++channel)
        for (int i = 0; i < dryBuffer.getNumSamples(); ++i)
            expect(dryBuffer.getSample(channel, i) == std::sin(0.1f * static_cast<float>(i)),
                   "0% wet changed the dry signal");
}

void testExtremeSettingsRemainFinite()
{
    GrainEngine engine;
    GrainEngine::Settings settings;
    settings.frequency = 200.0f;
    settings.pitch = 24.0f;
    settings.randomPitch = 2.0f;
    settings.sprayMs = 250.0f;
    settings.feedback = 0.0f;
    settings.dryWet = 1.0f;
    engine.prepare(48000.0, settings);

    juce::AudioBuffer<float> buffer(2, 48000);
    for (int channel = 0; channel < 2; ++channel)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample(channel, i, std::sin(juce::MathConstants<float>::twoPi * 440.0f
                                                  * static_cast<float>(i) / 48000.0f));
    engine.process(buffer.getReadPointer(0), buffer.getReadPointer(1),
                   buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());

    for (int channel = 0; channel < 2; ++channel)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const auto sample = buffer.getSample(channel, i);
            expect(std::isfinite(sample), "Extreme settings produced a non-finite sample");
            expect(std::abs(sample) <= 1.0001f, "Extreme settings produced unexpected gain");
        }
}

int main()
{
    try
    {
        testPhaseTableAndRotation();
        testRandomPitchSlope();
        testFractionalReadsAndWrap();
        testNaturalStageRateAndPitch();
        testImpulseAndDryPath();
        testExtremeSettingsRemainFinite();
        std::cout << "phase_table_513_rotation_random_pitch_impulse_dry_extremes: PASS\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Grain engine probe failed: " << error.what() << '\n';
        return 1;
    }
}
