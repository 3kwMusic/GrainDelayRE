#include "Parameters.h"

namespace Parameters
{
juce::StringArray targetChoices()
{
    return { "None", "Delay Time", "Spray", "Frequency", "Pitch", "Random Pitch", "Feedback", "Dry/Wet" };
}

juce::StringArray divisionChoices()
{
    return { "1", "2", "3", "4", "5", "6", "8", "16" };
}

const juce::String& parameterId(Target target)
{
    static const std::array<const juce::String*, 8> ids {
        nullptr, &delayMs, &spray, &frequency, &pitch, &randomPitch, &feedback, &dryWet
    };

    const auto index = juce::jlimit(0, 7, static_cast<int>(target));
    return index == 0 ? delayMs : *ids[static_cast<size_t>(index)];
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        delayMs, "Delay Time", juce::NormalisableRange<float>(1.0f, 1000.0f, 0.01f, 0.35f), 125.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    layout.add(std::make_unique<juce::AudioParameterBool>(syncEnabled, "Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(beatDivision, "Beat Division", divisionChoices(), 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        beatSwing, "Beat Swing", juce::NormalisableRange<float>(-100.0f, 100.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        frequency, "Frequency", juce::NormalisableRange<float>(1.0f, 200.0f, 0.01f, 0.3f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        pitch, "Pitch", juce::NormalisableRange<float>(-48.0f, 48.0f, 0.01f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        randomPitch, "Random Pitch", juce::NormalisableRange<float>(0.0f, 161.2f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        spray, "Spray", juce::NormalisableRange<float>(0.0f, 250.0f, 0.01f, 0.5f), 20.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));
    const auto percent = juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([] (float value, int)
        {
            return juce::String(value * 100.0f, 1) + "%";
        })
        .withValueFromStringFunction([] (const juce::String& text)
        {
            return text.trim().getFloatValue() * 0.01f;
        });

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        feedback, "Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.001f), 0.5f, percent));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        dryWet, "Dry/Wet", juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.5f, percent));

    layout.add(std::make_unique<juce::AudioParameterFloat>(xyX, "XY X", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(xyY, "XY Y", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(xTarget, "X Target", targetChoices(), 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>(yTarget, "Y Target", targetChoices(), 0));

    return layout;
}

float mappedValue(juce::AudioProcessorValueTreeState& state, Target target, float axisPosition)
{
    const auto& id = parameterId(target);
    const auto baseValue = state.getRawParameterValue(id)->load();

    if (target == Target::none)
        return baseValue;

    const auto axis = juce::jlimit(0.0f, 1.0f, axisPosition);
    return state.getParameter(id)->getNormalisableRange().convertFrom0to1(axis);
}

int divisionSixteenths(int choiceIndex)
{
    static constexpr int divisions[] { 1, 2, 3, 4, 5, 6, 8, 16 };
    return divisions[juce::jlimit(0, 7, choiceIndex)];
}
}
