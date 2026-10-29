#pragma once

#include "PluginProcessor.h"
#include "ModernLookAndFeel.h"
#include "XYPad.h"

class GrainDelayEditor final : public juce::AudioProcessorEditor,
                               private juce::Timer
{
public:
    explicit GrainDelayEditor(GrainDelayProcessor& processor);
    ~GrainDelayEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    juce::Slider& addKnob(const juce::String& id,
                          const juce::String& name,
                          Parameters::Target routeTarget);
    void addChoice(juce::ComboBox& combo, const juce::StringArray& items);
    void timerCallback() override;
    void updateRoutedControls();
    void applyXYToRoutedParameters();
    void handleManualChange(Parameters::Target target);
    Parameters::Target selectedTarget(const juce::String& id) const;

    GrainDelayProcessor& processor;
    ModernLookAndFeel modernLookAndFeel;

    std::vector<std::unique_ptr<juce::Slider>> knobs;
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboBoxAttachment>> comboAttachments;

    juce::Slider* delayKnob = nullptr;
    juce::Slider* swingKnob = nullptr;
    std::map<Parameters::Target, juce::Slider*> routedKnobs;
    std::map<Parameters::Target, juce::Label*> routedLabels;

    juce::ToggleButton syncButton { "Sync" };
    std::unique_ptr<ButtonAttachment> syncAttachment;

    juce::ComboBox divisionBox;
    juce::ComboBox xTargetBox;
    juce::ComboBox yTargetBox;
    juce::Label divisionLabel;
    juce::Label xLabel;
    juce::Label yLabel;
    juce::Label xyHint;

    XYPad xyPad;
    bool xyGestureActive = false;
    bool xAxisActive = false;
    bool yAxisActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainDelayEditor)
};
