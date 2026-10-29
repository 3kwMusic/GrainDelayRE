#pragma once

#include "Parameters.h"

class XYPad final : public juce::Component,
                    private juce::Timer
{
public:
    explicit XYPad(juce::AudioProcessorValueTreeState& state);

    std::function<void(bool)> onGestureChanged;
    std::function<void()> onPositionChanged;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    void setFromMouse(const juce::MouseEvent& event);
    float axisValue(const juce::String& id) const;
    juce::Rectangle<float> padBounds() const;

    juce::AudioProcessorValueTreeState& apvts;
    float displayedX = 0.5f;
    float displayedY = 0.5f;
    bool gestureActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPad)
};
