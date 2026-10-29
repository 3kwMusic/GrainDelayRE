#include "PluginEditor.h"

GrainDelayEditor::GrainDelayEditor(GrainDelayProcessor& p)
    : AudioProcessorEditor(&p), processor(p), xyPad(p.apvts)
{
    setLookAndFeel(&modernLookAndFeel);

    delayKnob = &addKnob(Parameters::delayMs, "Delay Time", Parameters::Target::delay);
    swingKnob = &addKnob(Parameters::beatSwing, "Beat Swing", Parameters::Target::none);
    addKnob(Parameters::frequency, "Frequency", Parameters::Target::frequency);
    addKnob(Parameters::spray, "Spray", Parameters::Target::spray);
    addKnob(Parameters::pitch, "Pitch", Parameters::Target::pitch);
    addKnob(Parameters::randomPitch, "Random Pitch", Parameters::Target::randomPitch);
    addKnob(Parameters::feedback, "Feedback", Parameters::Target::feedback);
    addKnob(Parameters::dryWet, "Dry / Wet", Parameters::Target::dryWet);

    addAndMakeVisible(syncButton);
    syncButton.setColour(juce::ToggleButton::textColourId, GrainPalette::text);
    syncAttachment = std::make_unique<ButtonAttachment>(processor.apvts, Parameters::syncEnabled, syncButton);

    addChoice(divisionBox, Parameters::divisionChoices());
    addChoice(xTargetBox, Parameters::targetChoices());
    addChoice(yTargetBox, Parameters::targetChoices());
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(processor.apvts, Parameters::beatDivision, divisionBox));
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(processor.apvts, Parameters::xTarget, xTargetBox));
    comboAttachments.push_back(std::make_unique<ComboBoxAttachment>(processor.apvts, Parameters::yTarget, yTargetBox));

    divisionLabel.setText("DIV", juce::dontSendNotification);
    xLabel.setText("X TARGET", juce::dontSendNotification);
    yLabel.setText("Y TARGET", juce::dontSendNotification);
    xyHint.setText("Drag XY to modulate. Turn a knob to take control.", juce::dontSendNotification);

    for (auto* label : { &divisionLabel, &xLabel, &yLabel, &xyHint })
    {
        label->setColour(juce::Label::textColourId, GrainPalette::mutedText);
        label->setFont(juce::FontOptions(10.5f, juce::Font::bold));
        addAndMakeVisible(label);
    }

    xyHint.setFont(juce::FontOptions(11.5f));
    xyHint.setJustificationType(juce::Justification::centredRight);

    addAndMakeVisible(divisionBox);
    addAndMakeVisible(xTargetBox);
    addAndMakeVisible(yTargetBox);
    addAndMakeVisible(xyPad);

    xyPad.onGestureChanged = [this](bool active)
    {
        xyGestureActive = active;
        xAxisActive = active;
        yAxisActive = active;
        processor.setXYGestureActive(true, active);
        processor.setXYGestureActive(false, active);

        if (active)
            applyXYToRoutedParameters();
    };
    xyPad.onPositionChanged = [this]
    {
        applyXYToRoutedParameters();
    };

    setSize(760, 458);
    startTimerHz(15);
    updateRoutedControls();
}

GrainDelayEditor::~GrainDelayEditor()
{
    processor.setXYGestureActive(true, false);
    processor.setXYGestureActive(false, false);
    setLookAndFeel(nullptr);
}

juce::Slider& GrainDelayEditor::addKnob(const juce::String& id,
                                        const juce::String& name,
                                        Parameters::Target routeTarget)
{
    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 20);
    slider->setRotaryParameters(juce::MathConstants<float>::pi * 1.22f,
                                juce::MathConstants<float>::pi * 2.78f, true);
    slider->setColour(juce::Slider::rotarySliderFillColourId, GrainPalette::accent);
    slider->setColour(juce::Slider::thumbColourId, GrainPalette::text);
    auto& result = *slider;

    knobs.push_back(std::move(slider));
    addAndMakeVisible(result);

    auto label = std::make_unique<juce::Label>();
    label->setText(name, juce::dontSendNotification);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId, GrainPalette::mutedText);
    label->setFont(juce::FontOptions(11.5f));
    labels.push_back(std::move(label));
    addAndMakeVisible(*labels.back());

    sliderAttachments.push_back(std::make_unique<SliderAttachment>(processor.apvts, id, result));

    if (routeTarget != Parameters::Target::none)
    {
        routedKnobs[routeTarget] = &result;
        routedLabels[routeTarget] = labels.back().get();
    }

    result.onDragStart = [this, routeTarget]
    {
        handleManualChange(routeTarget);
    };

    return result;
}

void GrainDelayEditor::addChoice(juce::ComboBox& combo, const juce::StringArray& items)
{
    combo.addItemList(items, 1);
}

Parameters::Target GrainDelayEditor::selectedTarget(const juce::String& id) const
{
    return static_cast<Parameters::Target>(processor.apvts.getRawParameterValue(id)->load());
}

void GrainDelayEditor::applyXYToRoutedParameters()
{
    if (!xyGestureActive)
        return;

    const auto xRoute = selectedTarget(Parameters::xTarget);
    const auto yRoute = selectedTarget(Parameters::yTarget);
    const auto x = processor.apvts.getRawParameterValue(Parameters::xyX)->load();
    const auto y = processor.apvts.getRawParameterValue(Parameters::xyY)->load();

    auto apply = [this](Parameters::Target target, float position)
    {
        if (target == Parameters::Target::none)
            return;

        // A real Delay gesture selects free time; routing selection alone never changes Sync.
        if (target == Parameters::Target::delay
            && processor.apvts.getRawParameterValue(Parameters::syncEnabled)->load() >= 0.5f)
        {
            auto* sync = processor.apvts.getParameter(Parameters::syncEnabled);
            sync->beginChangeGesture();
            sync->setValueNotifyingHost(0.0f);
            sync->endChangeGesture();
        }

        auto* parameter = processor.apvts.getParameter(Parameters::parameterId(target));
        const auto value = Parameters::mappedValue(processor.apvts, target, position);
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };

    if (xAxisActive)
        apply(xRoute, x);
    if (yAxisActive && yRoute != xRoute)
        apply(yRoute, y);

    updateRoutedControls();
}

void GrainDelayEditor::handleManualChange(Parameters::Target target)
{
    if (!xyGestureActive || target == Parameters::Target::none)
        return;

    if (xAxisActive && selectedTarget(Parameters::xTarget) == target)
    {
        xAxisActive = false;
        processor.setXYGestureActive(true, false);
    }

    if (yAxisActive && selectedTarget(Parameters::yTarget) == target)
    {
        yAxisActive = false;
        processor.setXYGestureActive(false, false);
    }
}

void GrainDelayEditor::timerCallback()
{
    updateRoutedControls();
}

void GrainDelayEditor::updateRoutedControls()
{
    const auto x = selectedTarget(Parameters::xTarget);
    const auto y = selectedTarget(Parameters::yTarget);
    const auto sync = processor.apvts.getRawParameterValue(Parameters::syncEnabled)->load() >= 0.5f;

    for (const auto& [target, knob] : routedKnobs)
    {
        const auto routed = target == x || target == y;
        knob->setEnabled(true);

        if (static_cast<bool>(knob->getProperties().getWithDefault("xyRouted", false)) != routed)
        {
            knob->getProperties().set("xyRouted", routed);
            knob->repaint();
        }

        routedLabels.at(target)->setColour(juce::Label::textColourId,
                                           routed ? GrainPalette::xyAccent : GrainPalette::mutedText);
    }

    delayKnob->setEnabled(!sync);
    divisionBox.setEnabled(sync);
    swingKnob->setEnabled(sync);
}

void GrainDelayEditor::paint(juce::Graphics& g)
{
    g.fillAll(GrainPalette::background);

    g.setColour(GrainPalette::text);
    g.setFont(juce::FontOptions(21.0f, juce::Font::bold));
    g.drawText("GRAIN DELAY", 22, 16, 250, 30, juce::Justification::left);

    g.setColour(GrainPalette::xyAccent);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("RE", 174, 22, 28, 18, juce::Justification::centred);

    g.setColour(GrainPalette::mutedText);
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("FOUR-TAP GRANULAR DELAY", 450, 22, 286, 18, juce::Justification::right);

    for (const auto& area : { juce::Rectangle<int>(16, 64, 464, 326),
                              juce::Rectangle<int>(496, 64, 248, 326) })
    {
        g.setColour(GrainPalette::panel);
        g.fillRoundedRectangle(area.toFloat(), 8.0f);
        g.setColour(GrainPalette::border.withAlpha(0.7f));
        g.drawRoundedRectangle(area.toFloat().reduced(0.5f), 8.0f, 1.0f);
    }

    g.setColour(GrainPalette::mutedText);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("PARAMETERS", 30, 74, 200, 18, juce::Justification::left);
    g.drawText("XY MODULATION", 512, 74, 200, 18, juce::Justification::left);

    g.setColour(GrainPalette::border);
    g.drawLine(20.0f, 404.0f, 740.0f, 404.0f, 1.0f);
}

void GrainDelayEditor::resized()
{
    const std::array<int, 8> order { 0, 2, 3, 4, 5, 6, 7, 1 };
    for (size_t i = 0; i < order.size(); ++i)
    {
        const auto column = static_cast<int>(i % 4);
        const auto row = static_cast<int>(i / 4);
        const auto x = 28 + column * 110;
        const auto labelY = 100 + row * 132;
        const auto index = static_cast<size_t>(order[i]);
        labels[index]->setBounds(x - 2, labelY, 104, 18);
        knobs[index]->setBounds(x, labelY + 20, 100, 104);
    }

    xyPad.setBounds(512, 100, 216, 174);
    xLabel.setBounds(512, 284, 100, 16);
    yLabel.setBounds(512, 336, 100, 16);
    xTargetBox.setBounds(512, 302, 216, 26);
    yTargetBox.setBounds(512, 354, 216, 26);

    syncButton.setBounds(20, 416, 68, 26);
    divisionLabel.setBounds(102, 416, 30, 26);
    divisionBox.setBounds(136, 416, 66, 26);
    xyHint.setBounds(218, 416, 520, 26);
}
