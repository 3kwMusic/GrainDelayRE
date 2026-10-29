// SPDX-FileCopyrightText: 2026 TriM
// SPDX-License-Identifier: AGPL-3.0-only

#include "XYPad.h"
#include "ModernLookAndFeel.h"

XYPad::XYPad(juce::AudioProcessorValueTreeState& state)
    : apvts(state)
{
    displayedX = axisValue(Parameters::xyX);
    displayedY = axisValue(Parameters::xyY);
    startTimerHz(30);
}

float XYPad::axisValue(const juce::String& id) const
{
    return juce::jlimit(0.0f, 1.0f, apvts.getRawParameterValue(id)->load());
}

juce::Rectangle<float> XYPad::padBounds() const
{
    return getLocalBounds().toFloat().reduced(9.0f);
}

void XYPad::setFromMouse(const juce::MouseEvent& event)
{
    const auto bounds = padBounds();
    const auto x = juce::jlimit(0.0f, 1.0f, (event.position.x - bounds.getX()) / bounds.getWidth());
    const auto y = juce::jlimit(0.0f, 1.0f, 1.0f - (event.position.y - bounds.getY()) / bounds.getHeight());

    apvts.getParameter(Parameters::xyX)->setValueNotifyingHost(x);
    apvts.getParameter(Parameters::xyY)->setValueNotifyingHost(y);

    if (onPositionChanged != nullptr)
        onPositionChanged();
}

void XYPad::mouseDown(const juce::MouseEvent& event)
{
    if (!event.mods.isLeftButtonDown())
        return;

    apvts.getParameter(Parameters::xyX)->beginChangeGesture();
    apvts.getParameter(Parameters::xyY)->beginChangeGesture();
    gestureActive = true;

    if (onGestureChanged != nullptr)
        onGestureChanged(true);

    setFromMouse(event);
    repaint();
}

void XYPad::mouseDrag(const juce::MouseEvent& event)
{
    if (gestureActive)
        setFromMouse(event);
}

void XYPad::mouseUp(const juce::MouseEvent&)
{
    if (!gestureActive)
        return;

    apvts.getParameter(Parameters::xyX)->endChangeGesture();
    apvts.getParameter(Parameters::xyY)->endChangeGesture();
    gestureActive = false;

    if (onGestureChanged != nullptr)
        onGestureChanged(false);

    repaint();
}

void XYPad::timerCallback()
{
    const auto x = axisValue(Parameters::xyX);
    const auto y = axisValue(Parameters::xyY);

    if (x != displayedX || y != displayedY)
    {
        displayedX = x;
        displayedY = y;
        repaint();
    }
}

void XYPad::paint(juce::Graphics& g)
{
    const auto outer = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(GrainPalette::background);
    g.fillRoundedRectangle(outer, 7.0f);
    g.setColour(GrainPalette::border);
    g.drawRoundedRectangle(outer, 7.0f, 1.0f);

    const auto bounds = padBounds();
    g.setColour(GrainPalette::border.withAlpha(0.62f));
    for (int i = 1; i < 4; ++i)
    {
        const auto proportion = static_cast<float>(i) * 0.25f;
        const auto x = bounds.getX() + bounds.getWidth() * proportion;
        const auto y = bounds.getY() + bounds.getHeight() * proportion;
        g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 0.65f);
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 0.65f);
    }

    const auto x = bounds.getX() + displayedX * bounds.getWidth();
    const auto y = bounds.getY() + (1.0f - displayedY) * bounds.getHeight();

    g.setColour(GrainPalette::xyAccent.withAlpha(gestureActive ? 0.65f : 0.32f));
    g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 1.0f);
    g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);

    g.setColour(GrainPalette::xyAccent.withAlpha(gestureActive ? 0.20f : 0.10f));
    g.fillEllipse(x - 10.0f, y - 10.0f, 20.0f, 20.0f);
    g.setColour(GrainPalette::xyAccent);
    g.fillEllipse(x - 4.0f, y - 4.0f, 8.0f, 8.0f);
    g.drawEllipse(x - 8.0f, y - 8.0f, 16.0f, 16.0f, gestureActive ? 1.8f : 1.0f);

    g.setColour(GrainPalette::mutedText);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("X", getLocalBounds().removeFromBottom(14), juce::Justification::centredRight);
    g.drawText("Y", getLocalBounds().removeFromLeft(14), juce::Justification::centred);
}
