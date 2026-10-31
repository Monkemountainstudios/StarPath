#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

class StarpathEngine
{
public:
    static constexpr int maxFlybys = 96;
    static constexpr int maxGhosts = 32;

    struct Parameters
    {
        float warp = 0.48f;
        float traffic = 0.42f;
        float mass = 0.0f;
        float trajectory = 0.55f;
        float scatter = 0.35f;
        float doppler = 0.35f;
        float dissolve = 0.0f;
        float wake = 0.22f;
        float atmosphere = 0.18f;
        float shield = 0.0f;
        float impact = 0.45f;
    };

    struct Snapshot
    {
        bool active = false;
        float progress = 0.0f;
        float headingX = 0.0f;
        float headingY = 0.0f;
        float originalHeadingX = 0.0f;
        float originalHeadingY = 0.0f;
        float impactPoint = 0.55f;
        float size = 0.0f;
        float brightness = 0.0f;
        float impactGlow = 0.0f;
        int colourIndex = 0;
        bool ghost = false;
        bool impacted = false;
    };

    void prepare (double newSampleRate, int maximumBlockSize);
    void reset() noexcept;
    void process (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& wet,
                  const Parameters& parameters) noexcept;
    [[nodiscard]] Snapshot getSnapshot (int index) const noexcept;
    [[nodiscard]] int getActiveFlybyCount() const noexcept;

private:
    struct Flyby
    {
        bool active = false;
        bool impacted = false;
        bool shieldConsidered = false;
        bool longGrain = false;
        bool reversed = false;
        int age = 0;
        int duration = 1;
        float sourceStart = 0.0f;
        float sourcePosition = 0.0f;
        float sourceLength = 128.0f;
        float baseRate = 1.0f;
        float headingX = 1.0f;
        float headingY = 0.0f;
        float originalHeadingX = 1.0f;
        float originalHeadingY = 0.0f;
        float closest = 0.55f;
        float proximity = 0.5f;
        float size = 0.5f;
        float gain = 1.0f;
        float dissolve = 0.0f;
        int texture = 0;
        int pulseCount = 3;
        int colourIndex = 0;
        float filterStateL = 0.0f;
        float filterStateR = 0.0f;
        float hpInputL = 0.0f;
        float hpInputR = 0.0f;
        float hpStateL = 0.0f;
        float hpStateR = 0.0f;
    };

    struct Ghost
    {
        bool active = false;
        int delay = 0;
        int age = 0;
        int duration = 1;
        int repeats = 0;
        float sourcePosition = 0.0f;
        float rate = 1.0f;
        float pan = 0.0f;
        float headingY = 0.0f;
        float gain = 0.0f;
        float filterStateL = 0.0f;
        float filterStateR = 0.0f;
    };

    struct AtomicSnapshot
    {
        std::atomic<bool> active { false };
        std::atomic<float> progress { 0.0f };
        std::atomic<float> headingX { 0.0f };
        std::atomic<float> headingY { 0.0f };
        std::atomic<float> originalHeadingX { 0.0f };
        std::atomic<float> originalHeadingY { 0.0f };
        std::atomic<float> impactPoint { 0.55f };
        std::atomic<float> size { 0.0f };
        std::atomic<float> brightness { 0.0f };
        std::atomic<float> impactGlow { 0.0f };
        std::atomic<int> colourIndex { 0 };
        std::atomic<bool> ghost { false };
        std::atomic<bool> impacted { false };
    };

    void spawnFlyby (const Parameters& p) noexcept;
    void spawnGhost (const Flyby& parent, const Parameters& p) noexcept;
    void renderFlyby (Flyby& voice, float& left, float& right,
                      const Parameters& p) noexcept;
    void renderGhost (Ghost& voice, float& left, float& right) noexcept;
    [[nodiscard]] float readCapture (int channel, float position) const noexcept;
    [[nodiscard]] float random01() noexcept;
    [[nodiscard]] float randomSigned() noexcept { return random01() * 2.0f - 1.0f; }
    [[nodiscard]] float biased (float bias) noexcept;
    [[nodiscard]] bool rare (float probability) noexcept { return random01() < probability; }
    void publishSnapshots() noexcept;

    std::array<Flyby, maxFlybys> flybys {};
    std::array<Ghost, maxGhosts> ghosts {};
    std::array<AtomicSnapshot, maxFlybys + maxGhosts> snapshots {};
    juce::AudioBuffer<float> capture;
    juce::AudioBuffer<float> atmosphereBuffer;
    juce::AudioBuffer<float> wakeDelay;
    juce::AudioBuffer<float> shieldDelay;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    int captureWrite = 0;
    int wakeWrite = 0;
    int shieldDelayWrite = 0;
    int capturedSamples = 0;
    double samplesUntilSpawn = 1.0;
    double sampleRate = 48000.0;
    uint32_t randomState = 0x53544152u;
    std::array<float, 2> inputDcX {};
    std::array<float, 2> inputDcY {};
    std::array<float, 2> inputLimiterEnvelope {};
    std::array<float, 2> inputLimiterGain { 1.0f, 1.0f };
    float inputActivityEnvelope = 0.0f;
    int inputActivityHoldSamples = 0;
    float shieldDelayAmount = 0.0f;
    float shieldDelaySamples = 33600.0f;
};

