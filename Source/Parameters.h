#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace Parameters
{
inline const juce::String delayMs { "delayMs" };
inline const juce::String syncEnabled { "syncEnabled" };
inline const juce::String beatDivision { "beatDivision" };
inline const juce::String beatSwing { "beatSwing" };
inline const juce::String frequency { "frequency" };
inline const juce::String pitch { "pitch" };
inline const juce::String randomPitch { "randomPitch" };
inline const juce::String spray { "spray" };
inline const juce::String feedback { "feedback" };
inline const juce::String dryWet { "dryWet" };
inline const juce::String xyX { "xyX" };
inline const juce::String xyY { "xyY" };
inline const juce::String xTarget { "xTarget" };
inline const juce::String yTarget { "yTarget" };

enum class Target
{
    none = 0,
    delay,
    spray,
    frequency,
    pitch,
    randomPitch,
    feedback,
    dryWet,
    count
};

juce::StringArray targetChoices();
juce::StringArray divisionChoices();
const juce::String& parameterId(Target target);
juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
float mappedValue(juce::AudioProcessorValueTreeState& state, Target target, float axisPosition);
int divisionSixteenths(int choiceIndex);
}
