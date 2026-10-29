#include "../../Source/PluginProcessor.h"
#include "../../Source/PluginEditor.h"
#include <functional>
#include <iostream>

void setValue(GrainDelayProcessor& processor, const juce::String& id, float value)
{
    auto* parameter = processor.apvts.getParameter(id);
    parameter->setValueNotifyingHost(parameter->getNormalisableRange().convertTo0to1(value));
}

void setChoice(GrainDelayProcessor& processor, const juce::String& id, int index, int count)
{
    processor.apvts.getParameter(id)->setValueNotifyingHost(static_cast<float>(index) / static_cast<float>(count - 1));
}

void runCase(const char* name, const std::function<void(GrainDelayProcessor&)>& configure,
             int expectedPeak = -1, bool expectDry = false)
{
    GrainDelayProcessor processor;
    configure(processor);
    processor.prepareToPlay(48000.0, 64);
    constexpr int total = 48000;
    juce::AudioBuffer<float> buffer(2, total);
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    juce::MidiBuffer midi;
    for (int start = 0; start < total; start += 64)
    {
        juce::AudioBuffer<float> block(buffer.getArrayOfWritePointers(), 2, start, 64);
        processor.processBlock(block, midi);
    }

    auto peak = 0.0f;
    auto peakIndex = -1;
    for (int i = 1; i < total; ++i)
    {
        const auto value = std::abs(buffer.getSample(0, i));
        if (value > peak)
        {
            peak = value;
            peakIndex = i;
        }
    }

    std::cout << name << ": dry0=" << buffer.getSample(0, 0)
              << " peak=" << peak << " index=" << peakIndex
              << " at1200=" << buffer.getSample(0, 1200)
              << " at2400=" << buffer.getSample(0, 2400)
              << " at4800=" << buffer.getSample(0, 4800) << '\n';
    if (expectDry)
    {
        if (buffer.getSample(0, 0) != 1.0f || peak > 0.00001f)
            throw std::runtime_error("Dry path changed or leaked wet audio");
    }
    else if (expectedPeak >= 0
             && (peakIndex != expectedPeak || std::abs(peak - 1.0f) > 0.001f
                 || std::abs(buffer.getSample(0, 0)) > 0.00001f))
        throw std::runtime_error("Processor delay position or wet gain is incorrect");
}

juce::MouseEvent mouseEvent(XYPad& pad, juce::Point<float> position, bool dragged = false)
{
    const auto now = juce::Time::getCurrentTime();
    return { juce::Desktop::getInstance().getMainMouseSource(), position,
             juce::ModifierKeys::leftButtonModifier, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
             &pad, &pad, now, position, now, 1, dragged };
}

void runUiProbe()
{
    const juce::ScopedJuceInitialiser_GUI initialiseGui;
    GrainDelayProcessor processor;
    setChoice(processor, Parameters::xTarget, static_cast<int>(Parameters::Target::delay), 8);
    setValue(processor, Parameters::delayMs, 100.0f);
    setValue(processor, Parameters::syncEnabled, 1.0f);
    std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());

    XYPad* pad = nullptr;
    juce::Slider* delaySlider = nullptr;
    juce::ComboBox* division = nullptr;
    for (auto* child : editor->getChildren())
    {
        if (auto* candidate = dynamic_cast<XYPad*>(child))
            pad = candidate;
        if (delaySlider == nullptr)
            delaySlider = dynamic_cast<juce::Slider*>(child);
        if (auto* combo = dynamic_cast<juce::ComboBox*>(child); combo != nullptr && combo->getItemText(0) == "1")
            division = combo;
    }

    if (pad == nullptr || delaySlider == nullptr || division == nullptr)
        throw std::runtime_error("UI controls not found");

    if (delaySlider->isEnabled() || !division->isEnabled()
        || processor.apvts.getRawParameterValue(Parameters::syncEnabled)->load() < 0.5f)
        throw std::runtime_error("Selecting Delay target changed Sync controls");

    pad->mouseDown(mouseEvent(*pad, { pad->getWidth() * 0.25f, pad->getHeight() * 0.5f }));
    if (processor.apvts.getRawParameterValue(Parameters::syncEnabled)->load() >= 0.5f
        || !delaySlider->isEnabled() || division->isEnabled())
        throw std::runtime_error("Actual Delay pad gesture did not select free delay");
    auto actual = processor.apvts.getRawParameterValue(Parameters::delayMs)->load();
    for (int i = 0; i < 24; ++i)
    {
        const auto position = 0.25f + static_cast<float>(i) * 0.02f;
        pad->mouseDrag(mouseEvent(*pad, { pad->getWidth() * position, pad->getHeight() * 0.5f }, true));

        if (delaySlider->onValueChange != nullptr)
            delaySlider->onValueChange();

        const auto expected = Parameters::mappedValue(processor.apvts, Parameters::Target::delay,
                                                      processor.apvts.getRawParameterValue(Parameters::xyX)->load());
        actual = processor.apvts.getRawParameterValue(Parameters::delayMs)->load();
        if (std::abs(actual - expected) > 0.02f || std::abs(delaySlider->getValue() - expected) > 0.02)
            throw std::runtime_error("Continuous XY drag stopped following after host echo");
    }
    if (!static_cast<bool>(delaySlider->getProperties().getWithDefault("xyRouted", false)))
        throw std::runtime_error("Routed delay knob is not highlighted");
    pad->mouseUp(mouseEvent(*pad, { pad->getWidth() * 0.70f, pad->getHeight() * 0.5f }));
    if (processor.apvts.getRawParameterValue(Parameters::syncEnabled)->load() >= 0.5f
        || std::abs(processor.apvts.getRawParameterValue(Parameters::delayMs)->load() - actual) > 0.02f)
        throw std::runtime_error("Releasing Pad changed mode or the written Delay");

    delaySlider->setValue(50.0, juce::sendNotificationSync);
    if (std::abs(processor.apvts.getRawParameterValue(Parameters::delayMs)->load() - 50.0f) > 0.02f)
        throw std::runtime_error("Manual knob value was not retained");

    pad->mouseDown(mouseEvent(*pad, { pad->getWidth() * 0.75f, pad->getHeight() * 0.5f }));
    delaySlider->onDragStart();
    delaySlider->setValue(75.0, juce::sendNotificationSync);
    pad->mouseDrag(mouseEvent(*pad, { pad->getWidth() * 0.90f, pad->getHeight() * 0.5f }, true));
    actual = processor.apvts.getRawParameterValue(Parameters::delayMs)->load();
    if (std::abs(actual - 75.0f) > 0.02f)
        throw std::runtime_error("XY drag overrode the manual value");
    pad->mouseUp(mouseEvent(*pad, { pad->getWidth() * 0.90f, pad->getHeight() * 0.5f }));

    std::cout << "ui_xy_highlight_follow_manual_priority: PASS\n";
}

int runProbes()
{
    GrainDelayProcessor parameterProcessor;
    const auto randomPitchRange = parameterProcessor.apvts.getParameter(Parameters::randomPitch)
                                      ->getNormalisableRange();
    if (randomPitchRange.start != 0.0f || std::abs(randomPitchRange.end - 161.2f) > 0.0001f)
    {
        std::cerr << "Random Pitch range must be 0..161.2\n";
        return 1;
    }
    std::cout << "random_pitch_range_0_161_2: PASS\n";

    RingBuffer ring;
    ring.prepare(10000);
    ring.write(1.0f);
    for (int i = 0; i < 4800; ++i)
    {
        ring.advance();
        ring.write(0.0f);
    }
    std::cout << "ring direct at4800=" << ring.readLinear(4800.0f) << '\n';

    runCase("delay100_wet100", [] (GrainDelayProcessor& p)
    {
        setValue(p, Parameters::delayMs, 100.0f);
        setValue(p, Parameters::dryWet, 1.0f);
        setValue(p, Parameters::feedback, 0.0f);
        setValue(p, Parameters::spray, 0.0f);
    }, 4800);

    runCase("delay50_wet100", [] (GrainDelayProcessor& p)
    {
        setValue(p, Parameters::delayMs, 50.0f);
        setValue(p, Parameters::dryWet, 1.0f);
        setValue(p, Parameters::feedback, 0.0f);
        setValue(p, Parameters::spray, 0.0f);
    }, 2400);

    runCase("delay100_dry0", [] (GrainDelayProcessor& p)
    {
        setValue(p, Parameters::delayMs, 100.0f);
        setValue(p, Parameters::dryWet, 0.0f);
        setValue(p, Parameters::feedback, 0.0f);
        setValue(p, Parameters::spray, 0.0f);
    }, -1, true);

    runCase("xy_inactive_uses_knob100", [] (GrainDelayProcessor& p)
    {
        setChoice(p, Parameters::xTarget, static_cast<int>(Parameters::Target::delay), 8);
        auto* delay = p.apvts.getParameter(Parameters::delayMs);
        p.apvts.getParameter(Parameters::xyX)->setValueNotifyingHost(
            delay->getNormalisableRange().convertTo0to1(25.0f));
        setValue(p, Parameters::delayMs, 100.0f);
        setValue(p, Parameters::dryWet, 1.0f);
        setValue(p, Parameters::feedback, 0.0f);
        setValue(p, Parameters::spray, 0.0f);
    }, 4800);

    runCase("xy_active_delay25_wet100", [] (GrainDelayProcessor& p)
    {
        setChoice(p, Parameters::xTarget, static_cast<int>(Parameters::Target::delay), 8);
        auto* delay = p.apvts.getParameter(Parameters::delayMs);
        p.apvts.getParameter(Parameters::xyX)->setValueNotifyingHost(
            delay->getNormalisableRange().convertTo0to1(25.0f));
        setValue(p, Parameters::dryWet, 1.0f);
        setValue(p, Parameters::feedback, 0.0f);
        setValue(p, Parameters::spray, 0.0f);
        p.setXYGestureActive(true, true);
    });

    for (const auto route : { Parameters::Target::none, Parameters::Target::delay })
        runCase("sync125_even_with_delay_target", [route] (GrainDelayProcessor& p)
        {
            setValue(p, Parameters::syncEnabled, 1.0f);
            setChoice(p, Parameters::xTarget, static_cast<int>(route), 8);
            setValue(p, Parameters::dryWet, 1.0f);
            setValue(p, Parameters::feedback, 0.0f);
            setValue(p, Parameters::spray, 0.0f);
        }, 6000);

    try
    {
        runUiProbe();
    }
    catch (const std::exception& error)
    {
        std::cerr << "UI probe failed: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

int main()
{
    try
    {
        return runProbes();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Processor probe failed: " << error.what() << '\n';
        return 1;
    }
}
