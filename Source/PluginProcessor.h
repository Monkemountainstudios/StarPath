#pragma once

#include <JuceHeader.h>
#include "OutputSafetyLimiter.h"
#include "StarpathEngine.h"

class StarpathAudioProcessor final : public juce::AudioProcessor
{
public:
    StarpathAudioProcessor();
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 60.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Factory Preset"; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;
    [[nodiscard]] StarpathEngine::Snapshot getSnapshot (int index) const noexcept { return engine.getSnapshot (index); }

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    StarpathEngine engine;
    OutputSafetyLimiter outputSafetyLimiter;
    juce::AudioBuffer<float> dry, wet;
    juce::SmoothedValue<float> inputGain, outputGain;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StarpathAudioProcessor)
};

