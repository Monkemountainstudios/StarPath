#include "PluginProcessor.h"
#include "PluginEditor.h"

StarpathAudioProcessor::StarpathAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "STARPATH", createLayout())
{}

juce::AudioProcessorValueTreeState::ParameterLayout StarpathAudioProcessor::createLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<P> ("warp", "Warp", R (0.0f, 1.0f), 0.2884615f));
    layout.add (std::make_unique<P> ("traffic", "Traffic", R (0.0f, 1.0f), 0.6484375f));
    layout.add (std::make_unique<P> ("mass", "Mass", R (-1.0f, 1.0f), 1.0f));
    layout.add (std::make_unique<P> ("trajectory", "Trajectory", R (0.0f, 1.0f), 1.0f));
    layout.add (std::make_unique<P> ("scatter", "Scatter", R (0.0f, 1.0f), 0.3164062f));
    layout.add (std::make_unique<P> ("doppler", "Doppler", R (0.0f, 1.0f), 0.0703125f));
    layout.add (std::make_unique<P> ("dissolve", "Dissolve", R (-1.0f, 1.0f), 0.3604375f));
    layout.add (std::make_unique<P> ("wake", "Wake", R (0.0f, 1.0f), 0.7890625f));
    layout.add (std::make_unique<P> ("atmosphere", "Atmosphere", R (0.0f, 1.0f), 0.988f));
    layout.add (std::make_unique<P> ("shield", "Shield", R (0.0f, 1.0f), 0.3867188f));
    layout.add (std::make_unique<P> ("impact", "Impact", R (0.0f, 1.0f), 1.0f));
    layout.add (std::make_unique<P> ("mix", "Mix", R (0.0f, 1.0f), 0.636f));
    // Append gain staging so existing host automation keeps its parameter order.
    layout.add (std::make_unique<P> ("inputTrim", "Input", R (-18.0f, 6.0f, 0.1f), -6.0f, "dB"));
    layout.add (std::make_unique<P> ("outputTrim", "Output", R (-18.0f, 6.0f, 0.1f), 0.0f, "dB"));
    return layout;
}

void StarpathAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    dry.setSize (2, samplesPerBlock);
    wet.setSize (2, samplesPerBlock);
    inputGain.reset (sampleRate, 0.035);
    inputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("inputTrim")->load()));
    outputGain.reset (sampleRate, 0.035);
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("outputTrim")->load()));
    outputSafetyLimiter.prepare (sampleRate);
}

void StarpathAudioProcessor::releaseResources()
{
    engine.reset();
    outputSafetyLimiter.reset();
}

bool StarpathAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
        && (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
            || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void StarpathAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (buffer.getNumChannels() == 1)
    {
        dry.setSize (2, buffer.getNumSamples(), false, false, true);
        dry.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
        dry.copyFrom (1, 0, buffer, 0, 0, buffer.getNumSamples());
    }
    else
    {
        dry.makeCopyOf (buffer, true);
    }

    inputGain.setTargetValue (juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("inputTrim")->load()));
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto gain = inputGain.getNextValue();
        for (int ch = 0; ch < dry.getNumChannels(); ++ch)
            dry.setSample (ch, sample, dry.getSample (ch, sample) * gain);
    }

    StarpathEngine::Parameters p;
    const auto value = [this] (const char* id) { return parameters.getRawParameterValue (id)->load(); };
    p.warp = value ("warp"); p.traffic = value ("traffic"); p.mass = value ("mass");
    p.trajectory = value ("trajectory"); p.scatter = value ("scatter"); p.doppler = value ("doppler");
    p.dissolve = value ("dissolve"); p.wake = value ("wake"); p.atmosphere = value ("atmosphere");
    p.shield = value ("shield"); p.impact = value ("impact");
    engine.process (dry, wet, p);

    const auto mix = value ("mix");
    const auto dryGain = std::cos (mix * juce::MathConstants<float>::halfPi);
    const auto wetGain = std::sin (mix * juce::MathConstants<float>::halfPi);
    const auto outputChannels = buffer.getNumChannels();
    for (int ch = 0; ch < outputChannels; ++ch)
    {
        buffer.copyFrom (ch, 0, dry, ch, 0, buffer.getNumSamples());
        buffer.applyGain (ch, 0, buffer.getNumSamples(), dryGain);
        buffer.addFrom (ch, 0, wet, ch, 0, buffer.getNumSamples(), wetGain);
    }

    outputGain.setTargetValue (juce::Decibels::decibelsToGain (
        parameters.getRawParameterValue ("outputTrim")->load()));
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto gain = outputGain.getNextValue();
        for (int ch = 0; ch < outputChannels; ++ch)
            buffer.setSample (ch, sample, buffer.getSample (ch, sample) * gain);
    }
    outputSafetyLimiter.process (buffer);
}

void StarpathAudioProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml()) copyXmlToBinary (*xml, destination);
}

void StarpathAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size); xml != nullptr)
        if (xml->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* StarpathAudioProcessor::createEditor() { return new StarpathAudioProcessorEditor (*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new StarpathAudioProcessor(); }

