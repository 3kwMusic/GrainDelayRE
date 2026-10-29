#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace GrainPalette
{
inline const juce::Colour background { 0xff0e1117 };
inline const juce::Colour panel { 0xff141922 };
inline const juce::Colour control { 0xff1b222d };
inline const juce::Colour border { 0xff29323f };
inline const juce::Colour text { 0xffe7ebf2 };
inline const juce::Colour mutedText { 0xff8792a3 };
inline const juce::Colour accent { 0xff8191ff };
inline const juce::Colour xyAccent { 0xff4ee6b0 };
}

class ModernLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
};
