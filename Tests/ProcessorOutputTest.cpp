#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <cmath>
#include <iostream>

namespace
{
constexpr double sampleRate = 48000.0;
constexpr int blockSize = 256;

void setPlainValue (StarpathAudioProcessor& processor, const char* id, float value)
{
    if (auto* parameter = processor.parameters.getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}
}

int main()
{
    juce::MidiBuffer midi;
    const auto minusSix = juce::Decibels::decibelsToGain (-6.0f);

    StarpathAudioProcessor defaultProcessor;
    setPlainValue (defaultProcessor, "mix", 0.0f);
    defaultProcessor.prepareToPlay (sampleRate, blockSize);
    juce::AudioBuffer<float> buffer (2, blockSize);
    for (int ch = 0; ch < 2; ++ch)
        for (int sample = 0; sample < blockSize; ++sample)
            buffer.setSample (ch, sample, 0.20f);
    defaultProcessor.processBlock (buffer, midi);
    const auto defaultError = std::abs (buffer.getSample (0, 0) - 0.20f * minusSix);
    std::cout << "Default -6 dB input error: " << defaultError << '\n';
    if (defaultError > 1.0e-7f) return 1;

    StarpathAudioProcessor trimProcessor;
    setPlainValue (trimProcessor, "mix", 0.0f);
    setPlainValue (trimProcessor, "outputTrim", -6.0f);
    trimProcessor.prepareToPlay (sampleRate, blockSize);
    buffer.clear();
    for (int ch = 0; ch < 2; ++ch)
        for (int sample = 0; sample < blockSize; ++sample)
            buffer.setSample (ch, sample, 0.20f);
    trimProcessor.processBlock (buffer, midi);
    const auto trimError = std::abs (buffer.getSample (0, 0) - 0.20f * minusSix * minusSix);
    std::cout << "Combined trim error: " << trimError << '\n';
    if (trimError > 1.0e-7f) return 2;

    StarpathAudioProcessor driveProcessor;
    setPlainValue (driveProcessor, "mix", 0.0f);
    setPlainValue (driveProcessor, "inputTrim", 6.0f);
    driveProcessor.prepareToPlay (sampleRate, blockSize);
    buffer.clear();
    buffer.setSample (0, 0, 0.10f);
    buffer.setSample (1, 0, 0.10f);
    driveProcessor.processBlock (buffer, midi);
    const auto driveError = std::abs (buffer.getSample (0, 0)
        - 0.10f * juce::Decibels::decibelsToGain (6.0f));
    std::cout << "+6 dB drive error: " << driveError << '\n';
    if (driveError > 1.0e-7f) return 3;

    StarpathAudioProcessor limiterProcessor;
    setPlainValue (limiterProcessor, "mix", 0.0f);
    setPlainValue (limiterProcessor, "inputTrim", 0.0f);
    limiterProcessor.prepareToPlay (sampleRate, blockSize);
    for (int ch = 0; ch < 2; ++ch)
        for (int sample = 0; sample < blockSize; ++sample)
            buffer.setSample (ch, sample, ch == 0 ? 1.2f : -1.2f);
    limiterProcessor.processBlock (buffer, midi);
    const auto peak = juce::jmax (buffer.getMagnitude (0, 0, blockSize),
                                  buffer.getMagnitude (1, 0, blockSize));
    std::cout << "Safety-limited peak: " << peak << '\n';
    if (peak > 0.9670f) return 4;
    return 0;
}
