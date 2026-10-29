#include "ModernLookAndFeel.h"

ModernLookAndFeel::ModernLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, GrainPalette::text);
    setColour(juce::Slider::textBoxBackgroundColourId, GrainPalette::control);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxHighlightColourId, GrainPalette::accent.withAlpha(0.25f));
    setColour(juce::ComboBox::backgroundColourId, GrainPalette::control);
    setColour(juce::ComboBox::textColourId, GrainPalette::text);
    setColour(juce::ComboBox::outlineColourId, GrainPalette::border);
    setColour(juce::ComboBox::arrowColourId, GrainPalette::mutedText);
    setColour(juce::PopupMenu::backgroundColourId, GrainPalette::control);
    setColour(juce::PopupMenu::textColourId, GrainPalette::text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, GrainPalette::accent.withAlpha(0.22f));
    setColour(juce::PopupMenu::highlightedTextColourId, GrainPalette::text);
    setColour(juce::TextEditor::backgroundColourId, GrainPalette::control);
    setColour(juce::TextEditor::textColourId, GrainPalette::text);
    setColour(juce::TextEditor::outlineColourId, GrainPalette::border);
    setColour(juce::TextEditor::focusedOutlineColourId, GrainPalette::accent);
    setColour(juce::CaretComponent::caretColourId, GrainPalette::text);
}

void ModernLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& slider)
{
    const auto routed = static_cast<bool>(slider.getProperties().getWithDefault("xyRouted", false));
    const auto accent = routed ? GrainPalette::xyAccent : GrainPalette::accent;
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height));
    const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight()) - 12.0f;
    const auto radius = juce::jmax(8.0f, diameter * 0.5f);
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    g.setOpacity(slider.isEnabled() ? 1.0f : 0.42f);

    if (routed)
    {
        g.setColour(accent.withAlpha(0.10f));
        g.fillEllipse(centre.x - radius - 4.0f, centre.y - radius - 4.0f,
                      (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f);
        g.setColour(accent.withAlpha(0.65f));
        g.drawEllipse(centre.x - radius - 3.0f, centre.y - radius - 3.0f,
                      (radius + 3.0f) * 2.0f, (radius + 3.0f) * 2.0f, 1.2f);
    }

    g.setColour(GrainPalette::control);
    g.fillEllipse(centre.x - radius + 3.0f, centre.y - radius + 3.0f,
                  (radius - 3.0f) * 2.0f, (radius - 3.0f) * 2.0f);

    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius - 2.0f, radius - 2.0f, 0.0f,
                        rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(GrainPalette::border);
    g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc(centre.x, centre.y, radius - 2.0f, radius - 2.0f, 0.0f,
                           rotaryStartAngle, angle, true);
    g.setColour(accent);
    g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    const auto pointerLength = radius * 0.58f;
    const auto pointerStart = centre.getPointOnCircumference(radius * 0.18f, angle);
    const auto pointerEnd = centre.getPointOnCircumference(pointerLength, angle);
    g.setColour(GrainPalette::text);
    g.drawLine({ pointerStart, pointerEnd }, 2.0f);

    g.setColour(accent);
    g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    g.setOpacity(1.0f);
}

juce::Label* ModernLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* label = LookAndFeel_V4::createSliderTextBox(slider);
    label->setFont(juce::FontOptions(11.5f));
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::backgroundColourId, GrainPalette::control);
    label->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    label->setColour(juce::Label::textColourId, GrainPalette::text);
    return label;
}

void ModernLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                     int, int, int, int, juce::ComboBox& box)
{
    const auto bounds = juce::Rectangle<float>(0.5f, 0.5f, static_cast<float>(width) - 1.0f,
                                               static_cast<float>(height) - 1.0f);
    g.setColour(box.isEnabled() ? GrainPalette::control : GrainPalette::panel);
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(box.hasKeyboardFocus(true) ? GrainPalette::accent : GrainPalette::border);
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    juce::Path arrow;
    const auto cx = static_cast<float>(width) - 14.0f;
    const auto cy = static_cast<float>(height) * 0.5f - 1.0f;
    arrow.startNewSubPath(cx - 3.0f, cy);
    arrow.lineTo(cx + 3.0f, cy);
    arrow.lineTo(cx, cy + 3.5f);
    arrow.closeSubPath();
    g.setColour(GrainPalette::mutedText);
    g.fillPath(arrow);
}

void ModernLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(9, 0, juce::jmax(0, box.getWidth() - 28), box.getHeight());
    label.setFont(getComboBoxFont(box));
}

juce::Font ModernLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return juce::FontOptions(12.0f);
}

void ModernLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                         bool shouldDrawButtonAsHighlighted, bool)
{
    const auto bounds = button.getLocalBounds().toFloat();
    const auto box = juce::Rectangle<float>(16.0f, 16.0f)
        .withCentre({ 10.0f, bounds.getCentreY() });

    g.setOpacity(button.isEnabled() ? 1.0f : 0.45f);
    g.setColour(button.getToggleState() ? GrainPalette::accent : GrainPalette::control);
    g.fillRoundedRectangle(box, 4.0f);
    g.setColour(shouldDrawButtonAsHighlighted ? GrainPalette::accent : GrainPalette::border);
    g.drawRoundedRectangle(box, 4.0f, 1.0f);

    if (button.getToggleState())
    {
        juce::Path tick;
        tick.startNewSubPath(box.getX() + 3.5f, box.getCentreY());
        tick.lineTo(box.getCentreX() - 0.5f, box.getBottom() - 4.0f);
        tick.lineTo(box.getRight() - 3.0f, box.getY() + 4.0f);
        g.setColour(GrainPalette::background);
        g.strokePath(tick, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    g.drawText(button.getButtonText(), 26, 0, juce::jmax(0, button.getWidth() - 26),
               button.getHeight(), juce::Justification::centredLeft);
    g.setOpacity(1.0f);
}
