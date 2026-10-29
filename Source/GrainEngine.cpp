#include "GrainEngine.h"

void GrainEngine::prepare(double newSampleRate, const Settings& initialSettings)
{
    sampleRate = newSampleRate;
    settings = initialSettings;

    leftBuffer.prepare(static_cast<int>(std::ceil(sampleRate * 8.0)));
    rightBuffer.prepare(static_cast<int>(std::ceil(sampleRate * 8.0)));

    frequencySmoothed.reset(sampleRate, 0.02);
    feedbackSmoothed.reset(sampleRate, 0.02);
    mixSmoothed.reset(sampleRate, 0.02);
    initialisePhaseTable();
    reset();
}

void GrainEngine::initialisePhaseTable()
{
    for (size_t i = 0; i < phaseTable.size(); ++i)
        phaseTable[i] = static_cast<float>(i) * 0.00048828125f - 0.5f;
}

void GrainEngine::reset()
{
    leftBuffer.reset();
    rightBuffer.reset();

    frequencySmoothed.setCurrentAndTargetValue(settings.frequency);
    feedbackSmoothed.setCurrentAndTargetValue(settings.feedback);
    mixSmoothed.setCurrentAndTargetValue(settings.dryWet);

    phasePosition = 0.0f;
    stage = 0;
    previousWetRight = 0.0f;
    swingCounter = 0;

    const auto pitchRatio = std::pow(2.0f, settings.pitch / 12.0f);
    const auto initialSlope = randomizeSlope(settings.frequency, pitchRatio);
    for (auto& tap : taps)
    {
        const auto swingSign = (swingCounter++ & 1U) == 0 ? 1.0f : -1.0f;
        const auto swungDelay = settings.delayMs * (1.0f + swingSign * settings.swing);
        tap.slopeMs = initialSlope;
        tap.offsetMs = juce::jmax(0.0f, swungDelay) + random.nextFloat() * settings.sprayMs;
    }
}

void GrainEngine::setSettings(const Settings& newSettings)
{
    settings = newSettings;
    frequencySmoothed.setTargetValue(newSettings.frequency);
    feedbackSmoothed.setTargetValue(newSettings.feedback);
    mixSmoothed.setTargetValue(newSettings.dryWet);
}

float GrainEngine::randomizeSlope(float frequency, float pitchRatio)
{
    const auto randomSigned = random.nextFloat() * 2.0f - 1.0f;
    return (1000.0f * (1.0f - pitchRatio) + randomSigned * settings.randomPitch)
        / juce::jmax(0.01f, frequency);
}

void GrainEngine::randomizeTap(GrainTap& tap, float frequency, float pitchRatio)
{
    tap.slopeMs = randomizeSlope(frequency, pitchRatio);

    const auto swingSign = (swingCounter++ & 1U) == 0 ? 1.0f : -1.0f;
    const auto swungDelay = settings.delayMs * (1.0f + swingSign * settings.swing);
    tap.offsetMs = juce::jmax(0.0f, swungDelay) + random.nextFloat() * settings.sprayMs;
}

float GrainEngine::renderTap(const GrainTap& tap, const RingBuffer& buffer, float phase) const
{
    const auto delaySamples = juce::jmax(
        0.0f, (tap.slopeMs * phase + tap.offsetMs) * static_cast<float>(sampleRate) * 0.001f);
    return buffer.readLinear(delaySamples);
}

float GrainEngine::renderCrossfade(float phase, float a, float b) const
{
    const auto weight = 0.5f + 0.5f * std::cos(juce::MathConstants<float>::twoPi * phase);
    return weight * a + (1.0f - weight) * b;
}

void GrainEngine::process(const float* inputLeft,
                          const float* inputRight,
                          float* outputLeft,
                          float* outputRight,
                          int numSamples)
{
    const auto pitchRatio = std::pow(2.0f, settings.pitch / 12.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        const auto frequency = frequencySmoothed.getNextValue();
        const auto feedback = feedbackSmoothed.getNextValue();
        const auto mix = mixSmoothed.getNextValue();

        // Advance the phase position in table-index units.
        phasePosition += 4.0f * frequency * (512.0f / static_cast<float>(sampleRate));
        const auto integerPosition = static_cast<juce::uint32>(phasePosition);
        const auto fraction = phasePosition - std::floor(phasePosition);
        const auto wrapped = integerPosition >= 512;
        const auto tableIndex = wrapped ? integerPosition & 511U : integerPosition;

        if (wrapped)
        {
            stage = (stage + 1U) & 3U;
            phasePosition = static_cast<float>(tableIndex) + fraction;
            randomizeTap(taps[stage], frequency, pitchRatio);
        }

        const auto phase = phaseTable[tableIndex]
            + fraction * (phaseTable[tableIndex + 1] - phaseTable[tableIndex]);
        const auto phase25 = phase + 0.25f;
        const auto phase50 = phase + 0.5f;
        const auto phase75 = phase + 0.75f;

        const auto dry = inputLeft[i];
        const auto dryRight = inputRight[i];
        leftBuffer.write(dry + feedback * previousWetRight);

        float wetLeft = 0.0f;
        float wetRight = 0.0f;

        switch (stage)
        {
            case 0:
                wetLeft = renderCrossfade(phase,
                                          renderTap(taps[0], leftBuffer, phase),
                                          renderTap(taps[2], leftBuffer, phase50));
                rightBuffer.write(dryRight + feedback * wetLeft);
                wetRight = renderCrossfade(phase75,
                                           renderTap(taps[1], rightBuffer, phase75),
                                           renderTap(taps[3], rightBuffer, phase25));
                break;
            case 1:
                wetLeft = renderCrossfade(phase25,
                                          renderTap(taps[0], leftBuffer, phase25),
                                          renderTap(taps[2], leftBuffer, phase75));
                rightBuffer.write(dryRight + feedback * wetLeft);
                wetRight = renderCrossfade(phase,
                                           renderTap(taps[1], rightBuffer, phase),
                                           renderTap(taps[3], rightBuffer, phase50));
                break;
            case 2:
                wetLeft = renderCrossfade(phase50,
                                          renderTap(taps[0], leftBuffer, phase50),
                                          renderTap(taps[2], leftBuffer, phase));
                rightBuffer.write(dryRight + feedback * wetLeft);
                wetRight = renderCrossfade(phase25,
                                           renderTap(taps[1], rightBuffer, phase25),
                                           renderTap(taps[3], rightBuffer, phase75));
                break;
            default:
                wetLeft = renderCrossfade(phase75,
                                          renderTap(taps[0], leftBuffer, phase75),
                                          renderTap(taps[2], leftBuffer, phase25));
                rightBuffer.write(dryRight + feedback * wetLeft);
                wetRight = renderCrossfade(phase50,
                                           renderTap(taps[1], rightBuffer, phase50),
                                           renderTap(taps[3], rightBuffer, phase));
                break;
        }

        previousWetRight = wetRight;

        const auto mixAngle = mix * juce::MathConstants<float>::pi;
        const auto dryGain = 0.5f + 0.5f * std::cos(mixAngle);
        const auto wetGain = 0.5f - 0.5f * std::cos(mixAngle);

        outputLeft[i] = dryGain * dry + wetGain * wetLeft;
        outputRight[i] = dryGain * dryRight + wetGain * wetRight;

        leftBuffer.advance();
        rightBuffer.advance();
    }
}
