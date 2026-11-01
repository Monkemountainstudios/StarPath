#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class StarpathAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit StarpathAudioProcessorEditor (StarpathAudioProcessor&);
    ~StarpathAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ConsoleLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;
    };

    struct Control
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    void addControl (const char* id, const char* name);
    std::unique_ptr<Control> createSideControl (const char* id, const char* name,
                                                juce::Colour colour);
    void timerCallback() override;

    StarpathAudioProcessor& processor;
    ConsoleLookAndFeel consoleLookAndFeel;
    std::vector<std::unique_ptr<Control>> controls;
    std::unique_ptr<Control> inputTrim, outputTrim;
    juce::Image starfieldBackdrop;
    float ambienceTime = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StarpathAudioProcessorEditor)
};

