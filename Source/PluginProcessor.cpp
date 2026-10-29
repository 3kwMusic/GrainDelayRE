#include "PluginProcessor.h"
#include "PluginEditor.h"

GrainDelayProcessor::GrainDelayProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", Parameters::createLayout())
{
}

void GrainDelayProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate, currentSettings());
}

void GrainDelayProcessor::releaseResources()
{
}

bool GrainDelayProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

float GrainDelayProcessor::raw(const juce::String& id) const
{
    return apvts.getRawParameterValue(id)->load();
}

Parameters::Target GrainDelayProcessor::target(const juce::String& id) const
{
    return static_cast<Parameters::Target>(juce::jlimit(
        0, static_cast<int>(Parameters::Target::count) - 1,
        static_cast<int>(raw(id))));
}

void GrainDelayProcessor::setXYGestureActive(bool xAxis, bool active)
{
    (xAxis ? xyGestureActiveX : xyGestureActiveY).store(active, std::memory_order_relaxed);
}

GrainEngine::Settings GrainDelayProcessor::currentSettings()
{
    const auto x = raw(Parameters::xyX);
    const auto y = raw(Parameters::xyY);
    const auto xRoute = target(Parameters::xTarget);
    const auto yRoute = target(Parameters::yTarget);

    auto value = [&](Parameters::Target parameter)
    {
        if (xRoute == parameter && xyGestureActiveX.load(std::memory_order_relaxed))
            return Parameters::mappedValue(apvts, parameter, x);
        if (yRoute == parameter && xyGestureActiveY.load(std::memory_order_relaxed))
            return Parameters::mappedValue(apvts, parameter, y);
        return raw(Parameters::parameterId(parameter));
    };

    GrainEngine::Settings result;
    result.frequency = value(Parameters::Target::frequency);
    result.pitch = value(Parameters::Target::pitch);
    result.randomPitch = value(Parameters::Target::randomPitch);
    result.sprayMs = value(Parameters::Target::spray);
    result.feedback = value(Parameters::Target::feedback);
    result.dryWet = value(Parameters::Target::dryWet);

    const auto sync = raw(Parameters::syncEnabled) >= 0.5f;

    if (sync)
    {
        auto bpm = 120.0;
        if (auto* hostPlayHead = getPlayHead())
        {
            if (const auto position = hostPlayHead->getPosition(); position.hasValue())
            {
                if (const auto hostBpm = position->getBpm(); hostBpm.hasValue())
                    bpm = *hostBpm;
            }
        }

        const auto division = Parameters::divisionSixteenths(static_cast<int>(raw(Parameters::beatDivision)));
        result.delayMs = static_cast<float>(division * 60000.0 / (4.0 * bpm));
        result.swing = raw(Parameters::beatSwing) * 0.01f;
    }
    else
    {
        result.delayMs = value(Parameters::Target::delay);
        result.swing = 0.0f;
    }

    return result;
}

void GrainDelayProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (getTotalNumInputChannels() != 2 || getTotalNumOutputChannels() != 2)
        return;

    engine.setSettings(currentSettings());

    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    engine.process(left, right, left, right, buffer.getNumSamples());
}

juce::AudioProcessorEditor* GrainDelayProcessor::createEditor()
{
    return new GrainDelayEditor(*this);
}

void GrainDelayProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void GrainDelayProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrainDelayProcessor();
}
