#pragma once

#include <JuceHeader.h>
#include <cmath>

class OutputSafetyLimiter
{
public:
    static constexpr float ceilingDecibels = -0.3f;

    void prepare (double sampleRate) noexcept
    {
        releaseCoefficient = std::exp (-1.0f
            / static_cast<float> (juce::jmax (1.0, sampleRate) * releaseSeconds));
        reset();
    }

    void reset() noexcept { gain = 1.0f; }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const auto ceiling = juce::Decibels::decibelsToGain (ceilingDecibels);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float peak = 0.0f;
            bool finite = true;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const auto value = buffer.getSample (ch, sample);
                finite = finite && std::isfinite (value);
                peak = juce::jmax (peak, std::abs (value));
            }
            if (! finite)
            {
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                    buffer.setSample (ch, sample, 0.0f);
                gain = 0.0f;
                continue;
            }

            const auto requiredGain = peak > ceiling ? ceiling / peak : 1.0f;
            gain = requiredGain < gain
                ? requiredGain
                : requiredGain + releaseCoefficient * (gain - requiredGain);
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.setSample (ch, sample, buffer.getSample (ch, sample) * gain);
        }
    }

private:
    static constexpr double releaseSeconds = 0.120;
    float releaseCoefficient = 0.0f;
    float gain = 1.0f;
};
