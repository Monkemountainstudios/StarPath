#include "StarpathEngine.h"
#include <cmath>

namespace
{
float smoothstep (float a, float b, float x) noexcept
{
    x = juce::jlimit (0.0f, 1.0f, (x - a) / juce::jmax (0.0001f, b - a));
    return x * x * (3.0f - 2.0f * x);
}

float equalPowerLeft (float pan) noexcept
{
    return std::cos ((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
}

float equalPowerRight (float pan) noexcept
{
    return std::sin ((pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f);
}
}

void StarpathEngine::prepare (double newSampleRate, int maximumBlockSize)
{
    sampleRate = juce::jmax (8000.0, newSampleRate);
    capture.setSize (2, juce::roundToInt (sampleRate * 10.0), false, true, false);
    atmosphereBuffer.setSize (2, juce::jmax (1, maximumBlockSize), false, true, false);
    wakeDelay.setSize (2, juce::roundToInt (sampleRate * 4.0), false, true, false);
    shieldDelay.setSize (2, juce::roundToInt (sampleRate * 2.5), false, true, false);

    juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32> (juce::jmax (1, maximumBlockSize)), 2 };
    chorus.prepare (spec);
    chorus.setCentreDelay (11.0f);
    chorus.setDepth (0.24f);
    chorus.setRate (0.17f);
    chorus.setFeedback (0.08f);
    chorus.setMix (0.5f);
    reset();
}

void StarpathEngine::reset() noexcept
{
    capture.clear();
    wakeDelay.clear();
    shieldDelay.clear();
    captureWrite = 0;
    wakeWrite = 0;
    shieldDelayWrite = 0;
    shieldDelayAmount = 0.0f;
    shieldDelaySamples = static_cast<float> (sampleRate * 0.7);
    capturedSamples = 0;
    samplesUntilSpawn = 1.0;
    for (auto& v : flybys) v = {};
    for (auto& v : ghosts) v = {};
    inputDcX.fill (0.0f);
    inputDcY.fill (0.0f);
    inputLimiterEnvelope.fill (0.0f);
    inputLimiterGain.fill (1.0f);
    inputActivityEnvelope = 0.0f;
    inputActivityHoldSamples = 0;
    chorus.reset();
    reverb.reset();
    publishSnapshots();
}

float StarpathEngine::random01() noexcept
{
    randomState ^= randomState << 13;
    randomState ^= randomState >> 17;
    randomState ^= randomState << 5;
    return static_cast<float> (randomState & 0x00ffffffu) / 16777216.0f;
}

float StarpathEngine::biased (float bias) noexcept
{
    const auto u = random01();
    if (bias > 0.0f)
        return std::pow (u, 1.0f / (1.0f + bias * 3.0f));
    if (bias < 0.0f)
        return std::pow (u, 1.0f + (-bias * 3.0f));
    return u;
}

float StarpathEngine::readCapture (int channel, float position) const noexcept
{
    const auto length = capture.getNumSamples();
    while (position < 0.0f) position += static_cast<float> (length);
    while (position >= static_cast<float> (length)) position -= static_cast<float> (length);
    const auto i0 = static_cast<int> (position);
    const auto i1 = (i0 + 1) % length;
    const auto fraction = position - static_cast<float> (i0);
    const auto* data = capture.getReadPointer (juce::jlimit (0, 1, channel));
    return data[i0] + fraction * (data[i1] - data[i0]);
}

void StarpathEngine::spawnFlyby (const Parameters& p) noexcept
{
    if (capturedSamples < static_cast<int> (sampleRate * 0.08)) return;
    auto it = std::find_if (flybys.begin(), flybys.end(), [] (const Flyby& v) { return ! v.active; });
    if (it == flybys.end()) return;

    auto& v = *it;
    v = {};
    v.active = true;
    const auto mass = biased (juce::jlimit (-1.0f, 1.0f, p.mass));
    v.size = mass;

    const auto nominalSeconds = juce::jmap (p.warp, 4.2f, 0.28f);
    auto speedVariation = std::exp2 (randomSigned() * (0.25f + p.scatter * 0.9f));
    if (rare (0.055f)) speedVariation *= rare (0.5f) ? 0.28f : 2.8f;
    const auto durationSeconds = juce::jlimit (0.12f, 7.0f,
                                               nominalSeconds / speedVariation * juce::jmap (mass, 0.72f, 1.35f));
    v.duration = juce::jmax (32, juce::roundToInt (durationSeconds * sampleRate));
    const auto heading = random01() * juce::MathConstants<float>::twoPi;
    v.headingX = std::cos (heading);
    v.headingY = std::sin (heading);
    v.originalHeadingX = v.headingX;
    v.originalHeadingY = v.headingY;
    v.closest = juce::jlimit (0.38f, 0.7f, 0.54f + randomSigned() * p.scatter * 0.12f);
    v.proximity = juce::jlimit (0.05f, 1.0f, p.trajectory + randomSigned() * (0.12f + p.scatter * 0.25f));
    auto baseSemitones = randomSigned() * (0.04f + p.scatter * 0.42f);
    if (p.doppler > 0.72f && rare (0.012f + 0.045f * p.doppler))
        baseSemitones += (random01() < 0.5f ? -1.0f : 1.0f) * juce::jmap (random01(), 2.0f, 7.0f);
    v.baseRate = std::exp2 (baseSemitones / 12.0f);
    v.gain = juce::jmap (mass, 0.18f, 0.52f) * juce::jmap (v.proximity, 0.58f, 0.92f);
    v.dissolve = juce::jlimit (-1.0f, 1.0f, p.dissolve + randomSigned() * p.scatter * 0.28f);
    const auto textureRoll = random01();
    v.texture = textureRoll < 0.52f ? 0 : (textureRoll < 0.82f ? 1 : 2);
    v.pulseCount = 2 + static_cast<int> (random01() * 6.0f);
    v.colourIndex = juce::jlimit (0, 3, static_cast<int> (random01() * 4.0f));
    v.reversed = rare (0.14f);

    const auto activeLongGrains = static_cast<int> (std::count_if (flybys.begin(), flybys.end(),
        [] (const Flyby& voice) { return voice.active && voice.longGrain; }));
    v.longGrain = activeLongGrains < 2 && capturedSamples > static_cast<int> (sampleRate * 0.9)
               && rare (0.035f);
    const auto desiredLength = v.longGrain
        ? static_cast<int> (sampleRate * juce::jmap (random01(), 0.85f, 1.4f))
        : static_cast<int> (sampleRate * juce::jmap (mass, 0.045f, 0.62f));
    v.sourceLength = static_cast<float> (juce::jlimit (64, capturedSamples - 2, desiredLength));
    const auto offset = static_cast<int> (v.sourceLength) + juce::roundToInt (random01() * sampleRate * 0.18);
    v.sourceStart = static_cast<float> (captureWrite - offset);
    v.sourcePosition = v.sourceStart;
}

void StarpathEngine::spawnGhost (const Flyby& parent, const Parameters& p) noexcept
{
    auto it = std::find_if (ghosts.begin(), ghosts.end(), [] (const Ghost& v) { return ! v.active; });
    if (it == ghosts.end()) return;
    auto& g = *it;
    g = {};
    g.active = true;
    g.delay = juce::roundToInt (sampleRate * juce::jmap (random01(), 0.07f, 0.42f));
    g.duration = juce::roundToInt (sampleRate * juce::jmap (parent.size, 0.18f, 1.35f));
    g.sourcePosition = parent.sourcePosition;
    float ghostSemitones = 0.0f;
    const auto pitchChoice = random01();
    if (pitchChoice > 0.84f && pitchChoice <= 0.94f)
        ghostSemitones = random01() < 0.68f ? 12.0f : -12.0f;
    else if (pitchChoice > 0.94f && p.impact > 0.55f)
        ghostSemitones = random01() < 0.72f ? 7.0f : -7.0f;
    g.rate = parent.baseRate * std::exp2 (ghostSemitones / 12.0f);
    g.pan = juce::jlimit (-1.0f, 1.0f, parent.headingX * (0.55f + random01() * 0.4f));
    g.headingY = parent.headingY;
    g.gain = p.impact * juce::jmap (parent.proximity, 0.18f, 0.58f);
    g.repeats = 1 + juce::roundToInt (p.impact * 3.0f);
}

void StarpathEngine::renderFlyby (Flyby& v, float& left, float& right,
                                  const Parameters& p) noexcept
{
    const auto progress = static_cast<float> (v.age) / static_cast<float> (v.duration);
    if (progress >= 1.0f) { v.active = false; return; }

    if (! v.shieldConsidered && progress >= v.closest - 0.055f * p.shield)
    {
        v.shieldConsidered = true;
        if (p.shield > 0.02f && random01() < p.shield)
        {
            v.impacted = true;
            const auto originalAngle = std::atan2 (v.headingY, v.headingX);
            const auto deflection = (random01() < 0.5f ? -1.0f : 1.0f) * (0.32f + p.shield * 0.92f);
            v.headingX = std::cos (originalAngle + deflection);
            v.headingY = std::sin (originalAngle + deflection);
            if (p.impact > 0.001f) spawnGhost (v, p);
        }
    }

    const auto before = juce::jlimit (0.0f, 1.0f, progress / v.closest);
    const auto after = juce::jlimit (0.0f, 1.0f, (progress - v.closest) / (1.0f - v.closest));
    const auto approach = progress < v.closest ? smoothstep (0.0f, 1.0f, before)
                                               : 1.0f - smoothstep (0.0f, 1.0f, after);
    const auto musicalDoppler = 1.8f * std::pow (p.doppler, 1.35f);
    const auto extremeDoppler = p.doppler > 0.72f
        ? 10.2f * std::pow ((p.doppler - 0.72f) / 0.28f, 2.0f) : 0.0f;
    const auto dopplerShape = progress < v.closest
        ? approach
        : 1.0f - 2.0f * smoothstep (0.0f, 0.72f, after);
    const auto dopplerSemitones = (musicalDoppler + extremeDoppler)
                                * juce::jmap (v.proximity, 0.35f, 1.0f) * dopplerShape;
    const auto playbackRate = v.baseRate * std::exp2 (dopplerSemitones / 12.0f)
                            * (v.reversed ? -1.0f : 1.0f);

    const auto phase = v.sourcePosition - v.sourceStart;
    auto secondPhase = phase + v.sourceLength * 0.5f;
    if (secondPhase >= v.sourceLength) secondPhase -= v.sourceLength;
    const auto windowA = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * phase / v.sourceLength);
    const auto windowB = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * secondPhase / v.sourceLength);
    auto secondPosition = v.sourceStart + secondPhase;
    auto inL = readCapture (0, v.sourcePosition) * windowA + readCapture (0, secondPosition) * windowB;
    auto inR = readCapture (1, v.sourcePosition) * windowA + readCapture (1, secondPosition) * windowB;
    v.sourcePosition += playbackRate;
    while (v.sourcePosition >= v.sourceStart + v.sourceLength) v.sourcePosition -= v.sourceLength;
    while (v.sourcePosition < v.sourceStart) v.sourcePosition += v.sourceLength;

    const auto departure = smoothstep (v.closest, 1.0f, progress);
    if (v.dissolve >= 0.0f)
    {
        const auto cutoff = juce::jmap (departure * v.dissolve, 80.0f, 9500.0f);
        const auto a = std::exp (-juce::MathConstants<float>::twoPi * cutoff / static_cast<float> (sampleRate));
        const auto hpL = a * (v.hpStateL + inL - v.hpInputL);
        const auto hpR = a * (v.hpStateR + inR - v.hpInputR);
        v.hpInputL = inL; v.hpInputR = inR; v.hpStateL = hpL; v.hpStateR = hpR;
        inL = juce::jmap (v.dissolve, inL, hpL);
        inR = juce::jmap (v.dissolve, inR, hpR);
    }
    else
    {
        const auto cutoff = juce::jmap (departure * -v.dissolve, 18000.0f, 260.0f);
        const auto a = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * cutoff / static_cast<float> (sampleRate));
        v.filterStateL += a * (inL - v.filterStateL);
        v.filterStateR += a * (inR - v.filterStateR);
        inL = juce::jmap (-v.dissolve, inL, v.filterStateL);
        inR = juce::jmap (-v.dissolve, inR, v.filterStateR);
    }

    const auto grainWindow = std::sin (juce::MathConstants<float>::pi * progress);
    float textureEnvelope = 1.0f;
    if (v.texture == 1)
        textureEnvelope = 0.12f + 0.88f * std::pow (juce::jmax (0.0f, std::sin (progress * v.pulseCount * juce::MathConstants<float>::pi)), 4.0f);
    else if (v.texture == 2)
        textureEnvelope = 0.56f + 0.44f * std::sin (progress * v.pulseCount * juce::MathConstants<float>::twoPi + v.size * 4.0f);
    const auto distanceGain = 0.18f + 0.82f * approach;
    const auto shieldDeflection = v.impacted ? p.shield * smoothstep (v.closest - 0.08f, v.closest + 0.12f, progress) : 0.0f;
    const auto headingBlend = v.impacted ? smoothstep (v.closest - 0.08f, v.closest + 0.12f, progress) : 0.0f;
    const auto audioHeadingX = juce::jmap (headingBlend, v.originalHeadingX, v.headingX);
    const auto panTravel = smoothstep (0.1f, 0.92f, progress);
    const auto pan = juce::jlimit (-1.0f, 1.0f,
                                   audioHeadingX * (panTravel * (0.42f + 0.58f * v.proximity)
                                                  + shieldDeflection * 0.32f));
    const auto gain = v.gain * grainWindow * textureEnvelope * distanceGain * (1.0f - shieldDeflection * 0.28f);
    const auto mono = 0.5f * (inL + inR);
    const auto side = 0.5f * (inL - inR);
    const auto width = 0.012f + std::pow (approach, 1.6f) * (0.5f + 0.38f * v.proximity);
    left += gain * (mono * equalPowerLeft (pan) + side * width);
    right += gain * (mono * equalPowerRight (pan) - side * width);
    ++v.age;
}

void StarpathEngine::renderGhost (Ghost& g, float& left, float& right) noexcept
{
    if (g.delay-- > 0) return;
    const auto progress = static_cast<float> (g.age) / static_cast<float> (g.duration);
    if (progress >= 1.0f)
    {
        if (g.repeats-- > 0)
        {
            g.age = 0;
            g.delay = juce::roundToInt (sampleRate * (0.11 + 0.09 * random01()));
            g.gain *= 0.56f;
            g.pan *= -0.82f;
            return;
        }
        g.active = false;
        return;
    }

    auto x = 0.5f * (readCapture (0, g.sourcePosition) + readCapture (1, g.sourcePosition));
    g.sourcePosition += g.rate;
    const auto coefficient = 0.025f + 0.16f * (1.0f - progress);
    g.filterStateL += coefficient * (x - g.filterStateL);
    g.filterStateR += coefficient * ((x * (0.7f + 0.3f * std::sin (progress * 31.0f))) - g.filterStateR);
    const auto envelope = std::sin (juce::MathConstants<float>::pi * progress);
    left += g.filterStateL * g.gain * envelope * equalPowerLeft (g.pan);
    right += g.filterStateR * g.gain * envelope * equalPowerRight (g.pan);
    ++g.age;
}

void StarpathEngine::process (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& wet,
                              const Parameters& p) noexcept
{
    const auto samples = input.getNumSamples();
    wet.setSize (2, samples, false, false, true);
    wet.clear();
    atmosphereBuffer.setSize (2, samples, false, false, true);
    atmosphereBuffer.clear();

    const auto inputChannels = input.getNumChannels();
    const auto wakeDelaySamples = juce::jlimit (1, wakeDelay.getNumSamples() - 1,
        juce::roundToInt (sampleRate * juce::jmap (p.wake, 0.16f, 0.52f)));
    const auto wakeFeedback = p.wake > 0.001f
        ? 0.18f + 0.76f * std::pow (p.wake, 1.7f) : 0.0f;
    const auto shieldDelayTarget = juce::jlimit (0.0f, 1.0f, (p.shield - 0.3f) / 0.7f);
    const auto shieldSmoothing = 1.0f - std::exp (-1.0f / static_cast<float> (sampleRate * 0.025));
    for (int i = 0; i < samples; ++i)
    {
        float instantActivity = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
        {
            const auto raw = input.getSample (juce::jmin (ch, inputChannels - 1), i);
            const auto dcBlocked = raw - inputDcX[static_cast<size_t> (ch)]
                                 + 0.995f * inputDcY[static_cast<size_t> (ch)];
            inputDcX[static_cast<size_t> (ch)] = raw;
            inputDcY[static_cast<size_t> (ch)] = dcBlocked;

            auto& envelope = inputLimiterEnvelope[static_cast<size_t> (ch)];
            auto& limiterGain = inputLimiterGain[static_cast<size_t> (ch)];
            const auto magnitude = std::abs (dcBlocked);
            envelope = juce::jmax (magnitude, envelope * 0.9994f);
            const auto targetGain = envelope > 0.86f ? 0.86f / envelope : 1.0f;
            limiterGain = targetGain < limiterGain ? targetGain
                                                   : limiterGain + (targetGain - limiterGain) * 0.00035f;
            auto protectedInput = dcBlocked * limiterGain;
            protectedInput = protectedInput / (1.0f + std::abs (protectedInput) * 0.12f);
            instantActivity = juce::jmax (instantActivity, std::abs (protectedInput));
            capture.setSample (ch, captureWrite, protectedInput);
        }
        inputActivityEnvelope = juce::jmax (instantActivity, inputActivityEnvelope * 0.99935f);
        if (inputActivityEnvelope > 0.0018f)
            inputActivityHoldSamples = juce::roundToInt (sampleRate * 0.42);
        else if (inputActivityHoldSamples > 0)
            --inputActivityHoldSamples;
        captureWrite = (captureWrite + 1) % capture.getNumSamples();
        capturedSamples = juce::jmin (capture.getNumSamples(), capturedSamples + 1);

        if (--samplesUntilSpawn <= 0.0)
        {
            if (inputActivityHoldSamples > 0)
            {
                spawnFlyby (p);
                const auto eventsPerSecond = juce::jmap (p.traffic * p.traffic, 0.18f, 24.0f);
                auto interval = sampleRate / eventsPerSecond;
                interval *= std::exp2 (randomSigned() * (0.15f + p.scatter * 0.75f));
                if (rare (0.06f)) interval *= random01() < 0.5f ? 0.25 : 3.5;
                samplesUntilSpawn = juce::jmax (16.0, interval);
            }
            else
            {
                samplesUntilSpawn = juce::jmax (32.0, sampleRate * 0.0125);
            }
        }

        float l = 0.0f, r = 0.0f;
        for (auto& voice : flybys) if (voice.active) renderFlyby (voice, l, r, p);
        for (auto& voice : ghosts) if (voice.active) renderGhost (voice, l, r);

        shieldDelayAmount += (shieldDelayTarget - shieldDelayAmount) * shieldSmoothing;
        const auto shieldDelayTargetSamples = static_cast<float> (sampleRate)
                                            * juce::jmap (shieldDelayAmount, 0.7f, 1.2f);
        shieldDelaySamples += (shieldDelayTargetSamples - shieldDelaySamples) * shieldSmoothing;
        auto shieldReadPosition = static_cast<float> (shieldDelayWrite) - shieldDelaySamples;
        while (shieldReadPosition < 0.0f) shieldReadPosition += static_cast<float> (shieldDelay.getNumSamples());
        const auto shieldRead0 = static_cast<int> (shieldReadPosition);
        const auto shieldRead1 = (shieldRead0 + 1) % shieldDelay.getNumSamples();
        const auto shieldFraction = shieldReadPosition - static_cast<float> (shieldRead0);
        const auto shieldL = juce::jmap (shieldFraction, shieldDelay.getSample (0, shieldRead0),
                                                        shieldDelay.getSample (0, shieldRead1));
        const auto shieldR = juce::jmap (shieldFraction, shieldDelay.getSample (1, shieldRead0),
                                                        shieldDelay.getSample (1, shieldRead1));
        const auto shieldFeedback = shieldDelayAmount * (0.2f + shieldDelayAmount * 0.2f);
        const auto shieldWriteL = l + shieldL * shieldFeedback;
        const auto shieldWriteR = r + shieldR * shieldFeedback;
        shieldDelay.setSample (0, shieldDelayWrite, shieldWriteL / (1.0f + std::abs (shieldWriteL) * 0.12f));
        shieldDelay.setSample (1, shieldDelayWrite, shieldWriteR / (1.0f + std::abs (shieldWriteR) * 0.12f));
        shieldDelayWrite = (shieldDelayWrite + 1) % shieldDelay.getNumSamples();
        const auto shieldReturn = shieldDelayAmount * (0.22f + shieldDelayAmount * 0.24f);
        l += shieldL * shieldReturn;
        r += shieldR * shieldReturn;

        auto wakeRead = wakeWrite - wakeDelaySamples;
        if (wakeRead < 0) wakeRead += wakeDelay.getNumSamples();
        const auto wakeL = wakeDelay.getSample (0, wakeRead);
        const auto wakeR = wakeDelay.getSample (1, wakeRead);
        const auto writeL = l + wakeFeedback * (wakeL * 0.78f + wakeR * 0.22f);
        const auto writeR = r + wakeFeedback * (wakeR * 0.78f + wakeL * 0.22f);
        wakeDelay.setSample (0, wakeWrite, writeL / (1.0f + std::abs (writeL) * 0.16f));
        wakeDelay.setSample (1, wakeWrite, writeR / (1.0f + std::abs (writeR) * 0.16f));
        wakeWrite = (wakeWrite + 1) % wakeDelay.getNumSamples();
        l += wakeL * p.wake * 0.68f;
        r += wakeR * p.wake * 0.68f;

        l = l / (1.0f + std::abs (l) * 0.32f);
        r = r / (1.0f + std::abs (r) * 0.32f);
        wet.setSample (0, i, l);
        wet.setSample (1, i, r);
        atmosphereBuffer.setSample (0, i, l);
        atmosphereBuffer.setSample (1, i, r);
    }

    if (p.wake > 0.001f)
    {
        juce::dsp::AudioBlock<float> block (atmosphereBuffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        chorus.setMix (juce::jlimit (0.0f, 0.72f, p.wake * 0.72f));
        chorus.process (context);
    }

    juce::Reverb::Parameters rp;
    rp.roomSize = 0.38f + p.atmosphere * 0.52f;
    rp.damping = 0.48f;
    rp.wetLevel = p.atmosphere * 0.42f;
    rp.dryLevel = 0.0f;
    rp.width = 1.0f;
    reverb.setParameters (rp);
    reverb.processStereo (atmosphereBuffer.getWritePointer (0), atmosphereBuffer.getWritePointer (1), samples);
    for (int ch = 0; ch < 2; ++ch)
    {
        wet.addFrom (ch, 0, atmosphereBuffer, ch, 0, samples, p.wake * 0.45f + p.atmosphere);
        auto* channel = wet.getWritePointer (ch);
        for (int i = 0; i < samples; ++i)
            channel[i] = channel[i] / (1.0f + std::abs (channel[i]) * 0.28f);
    }

    publishSnapshots();
}

void StarpathEngine::publishSnapshots() noexcept
{
    int index = 0;
    for (const auto& v : flybys)
    {
        auto& s = snapshots[static_cast<size_t> (index++)];
        s.active.store (v.active, std::memory_order_relaxed);
        s.progress.store (v.active ? static_cast<float> (v.age) / static_cast<float> (v.duration) : 0.0f, std::memory_order_relaxed);
        s.headingX.store (v.headingX, std::memory_order_relaxed);
        s.headingY.store (v.headingY, std::memory_order_relaxed);
        s.originalHeadingX.store (v.originalHeadingX, std::memory_order_relaxed);
        s.originalHeadingY.store (v.originalHeadingY, std::memory_order_relaxed);
        s.impactPoint.store (v.closest, std::memory_order_relaxed);
        s.size.store (v.size, std::memory_order_relaxed);
        const auto progress = v.active ? static_cast<float> (v.age) / static_cast<float> (v.duration) : 0.0f;
        const auto impactGlow = v.impacted ? 1.0f - smoothstep (v.closest, juce::jmin (1.0f, v.closest + 0.24f), progress) : 0.0f;
        s.brightness.store (v.impacted ? 0.18f + impactGlow * 0.62f : 0.45f + v.proximity * 0.4f, std::memory_order_relaxed);
        s.impactGlow.store (impactGlow, std::memory_order_relaxed);
        s.colourIndex.store (v.colourIndex, std::memory_order_relaxed);
        s.ghost.store (false, std::memory_order_relaxed);
        s.impacted.store (v.impacted, std::memory_order_relaxed);
    }
    for (const auto& g : ghosts)
    {
        auto& s = snapshots[static_cast<size_t> (index++)];
        s.active.store (g.active && g.delay <= 0, std::memory_order_relaxed);
        s.progress.store (g.active ? static_cast<float> (g.age) / static_cast<float> (g.duration) : 0.0f, std::memory_order_relaxed);
        s.headingX.store (g.pan, std::memory_order_relaxed);
        s.headingY.store (g.headingY, std::memory_order_relaxed);
        s.originalHeadingX.store (g.pan, std::memory_order_relaxed);
        s.originalHeadingY.store (g.headingY, std::memory_order_relaxed);
        s.impactPoint.store (0.0f, std::memory_order_relaxed);
        s.size.store (0.25f, std::memory_order_relaxed);
        s.brightness.store (g.gain, std::memory_order_relaxed);
        s.impactGlow.store (0.0f, std::memory_order_relaxed);
        s.colourIndex.store (3, std::memory_order_relaxed);
        s.ghost.store (true, std::memory_order_relaxed);
        s.impacted.store (false, std::memory_order_relaxed);
    }
}

StarpathEngine::Snapshot StarpathEngine::getSnapshot (int index) const noexcept
{
    if (! juce::isPositiveAndBelow (index, static_cast<int> (snapshots.size()))) return {};
    const auto& source = snapshots[static_cast<size_t> (index)];
    return { source.active.load (std::memory_order_relaxed),
             source.progress.load (std::memory_order_relaxed),
             source.headingX.load (std::memory_order_relaxed), source.headingY.load (std::memory_order_relaxed),
             source.originalHeadingX.load (std::memory_order_relaxed), source.originalHeadingY.load (std::memory_order_relaxed),
             source.impactPoint.load (std::memory_order_relaxed), source.size.load (std::memory_order_relaxed),
             source.brightness.load (std::memory_order_relaxed), source.impactGlow.load (std::memory_order_relaxed),
             source.colourIndex.load (std::memory_order_relaxed), source.ghost.load (std::memory_order_relaxed),
             source.impacted.load (std::memory_order_relaxed) };
}

int StarpathEngine::getActiveFlybyCount() const noexcept
{
    return static_cast<int> (std::count_if (flybys.begin(), flybys.end(), [] (const Flyby& v) { return v.active; }));
}

