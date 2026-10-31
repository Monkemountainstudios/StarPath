#include <JuceHeader.h>
#include "StarpathEngine.h"
#include <iostream>

int main()
{
    {
        StarpathEngine silentEngine;
        silentEngine.prepare (48000.0, 256);
        StarpathEngine::Parameters silentParameters;
        silentParameters.traffic = 1.0f;
        silentParameters.shield = 1.0f;
        juce::AudioBuffer<float> silence (2, 256), silentOutput;
        silence.clear();
        for (int block = 0; block < 560; ++block)
            silentEngine.process (silence, silentOutput, silentParameters);
        if (silentEngine.getActiveFlybyCount() != 0)
        {
            std::cerr << "silence spawned sound flybys\n";
            return 3;
        }
    }

    StarpathEngine engine;
    engine.prepare (48000.0, 256);
    StarpathEngine::Parameters parameters;
    parameters.traffic = 0.8f;
    parameters.shield = 1.0f;
    parameters.impact = 0.8f;
    juce::AudioBuffer<float> input (2, 256), output;
    double phase = 0.0;
    double energy = 0.0;
    int peakVoices = 0;
    for (int block = 0; block < 900; ++block)
    {
        for (int i = 0; i < input.getNumSamples(); ++i)
        {
            const auto x = static_cast<float> (0.18 * std::sin (phase));
            phase += juce::MathConstants<double>::twoPi * 220.0 / 48000.0;
            input.setSample (0, i, x);
            input.setSample (1, i, x * 0.8f);
        }
        engine.process (input, output, parameters);
        for (int ch = 0; ch < output.getNumChannels(); ++ch)
            for (int i = 0; i < output.getNumSamples(); ++i)
            {
                const auto x = output.getSample (ch, i);
                if (! std::isfinite (x)) { std::cerr << "non-finite output\n"; return 1; }
                energy += x * x;
            }
        peakVoices = juce::jmax (peakVoices, engine.getActiveFlybyCount());
    }
    if (energy <= 0.001 || peakVoices == 0) { std::cerr << "engine produced no flybys\n"; return 2; }
    std::cout << "Starpath DSP smoke test passed; peak flybys=" << peakVoices << ", energy=" << energy << "\n";
    return 0;
}

