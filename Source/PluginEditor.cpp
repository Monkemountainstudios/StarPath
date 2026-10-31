#include "PluginEditor.h"
#include <StarpathAssets.h>

void StarpathAudioProcessorEditor::ConsoleLookAndFeel::drawLinearSlider (
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float, float, juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const auto accent = slider.findColour (juce::Slider::trackColourId);
    if (style == juce::Slider::LinearVertical)
    {
        const auto track = juce::Rectangle<float> (static_cast<float> (x) + width * 0.5f - 7.0f,
                                                    static_cast<float> (y) + 6.0f,
                                                    14.0f, static_cast<float> (height) - 12.0f);
        juce::Path chassis;
        chassis.startNewSubPath (track.getX() + 3.0f, track.getY());
        chassis.lineTo (track.getRight() - 3.0f, track.getY());
        chassis.lineTo (track.getRight(), track.getY() + 3.0f);
        chassis.lineTo (track.getRight(), track.getBottom() - 3.0f);
        chassis.lineTo (track.getRight() - 3.0f, track.getBottom());
        chassis.lineTo (track.getX() + 3.0f, track.getBottom());
        chassis.lineTo (track.getX(), track.getBottom() - 3.0f);
        chassis.lineTo (track.getX(), track.getY() + 3.0f);
        chassis.closeSubPath();
        g.setColour (juce::Colour (0xff182127));
        g.fillPath (chassis);
        g.saveState();
        g.reduceClipRegion (chassis.getBounds().toNearestInt());
        juce::ColourGradient illumination (accent.brighter (0.18f).withAlpha (0.92f), track.getCentreX(), track.getBottom(),
                                           accent.darker (0.46f).withAlpha (0.7f), track.getCentreX(), track.getY(), false);
        g.setGradientFill (illumination);
        g.fillRect (track.withTop (juce::jlimit (track.getY(), track.getBottom(), sliderPos)));
        g.restoreState();
        g.setColour (juce::Colour (0xff7a6a55).withAlpha (0.46f));
        g.strokePath (chassis, juce::PathStrokeType (0.8f));
        g.setColour (juce::Colour (0xff05090c).withAlpha (0.72f));
        for (int segment = 1; segment < 8; ++segment)
        {
            const auto sy = track.getY() + track.getHeight() * static_cast<float> (segment) / 8.0f;
            g.drawHorizontalLine (juce::roundToInt (sy), track.getX() + 2.0f, track.getRight() - 2.0f);
        }
        const auto cursorY = juce::jlimit (track.getY() + 4.0f, track.getBottom() - 4.0f, sliderPos);
        juce::Path cursorPath;
        cursorPath.startNewSubPath (track.getX() + 1.0f, cursorY);
        cursorPath.lineTo (track.getCentreX(), cursorY - 3.5f);
        cursorPath.lineTo (track.getRight() - 1.0f, cursorY);
        cursorPath.lineTo (track.getCentreX(), cursorY + 3.5f);
        cursorPath.closeSubPath();
        g.setColour (accent.brighter (0.34f));
        g.fillPath (cursorPath);
        g.setColour (juce::Colour (0xffffe0a6).withAlpha (0.72f));
        g.strokePath (cursorPath, juce::PathStrokeType (0.65f));
        return;
    }

    const auto track = juce::Rectangle<float> (static_cast<float> (x) + 7.0f,
                                                static_cast<float> (y) + height * 0.5f - 6.0f,
                                                static_cast<float> (width) - 14.0f, 12.0f);
    juce::Path chassis;
    chassis.startNewSubPath (track.getX() + 5.0f, track.getY());
    chassis.lineTo (track.getRight(), track.getY());
    chassis.lineTo (track.getRight() - 5.0f, track.getBottom());
    chassis.lineTo (track.getX(), track.getBottom());
    chassis.closeSubPath();
    g.setColour (juce::Colour (0xff182127));
    g.fillPath (chassis);

    g.saveState();
    g.reduceClipRegion (chassis.getBounds().toNearestInt());
    juce::ColourGradient illumination (accent.withAlpha (0.92f), track.getX(), track.getCentreY(),
                                       accent.darker (0.45f).withAlpha (0.72f), track.getRight(), track.getCentreY(), false);
    g.setGradientFill (illumination);
    g.fillRect (track.withRight (juce::jlimit (track.getX(), track.getRight(), sliderPos)));
    g.restoreState();

    g.setColour (juce::Colour (0xff7a6a55).withAlpha (0.46f));
    g.strokePath (chassis, juce::PathStrokeType (0.8f));
    g.setColour (juce::Colour (0xff05090c).withAlpha (0.7f));
    for (int segment = 1; segment < 8; ++segment)
    {
        const auto sx = track.getX() + track.getWidth() * static_cast<float> (segment) / 8.0f;
        g.drawVerticalLine (juce::roundToInt (sx), track.getY() + 2.0f, track.getBottom() - 2.0f);
    }

    juce::Path cursorPath;
    cursorPath.startNewSubPath (sliderPos, track.getY() - 4.0f);
    cursorPath.lineTo (sliderPos + 5.0f, track.getCentreY());
    cursorPath.lineTo (sliderPos, track.getBottom() + 4.0f);
    cursorPath.lineTo (sliderPos - 5.0f, track.getCentreY());
    cursorPath.closeSubPath();
    g.setColour (accent.brighter (0.34f));
    g.fillPath (cursorPath);
    g.setColour (juce::Colour (0xffffe0a6).withAlpha (0.72f));
    g.strokePath (cursorPath, juce::PathStrokeType (0.65f));
}

StarpathAudioProcessorEditor::StarpathAudioProcessorEditor (StarpathAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    starfieldBackdrop = juce::ImageCache::getFromMemory (StarpathAssets::starfield_jpg,
                                                         StarpathAssets::starfield_jpgSize);
    for (auto [id, name] : { std::pair { "warp", "WARP" }, { "traffic", "TRAFFIC" },
                             { "mass", "MASS" }, { "trajectory", "TRAJECTORY" },
                             { "scatter", "SCATTER" }, { "doppler", "DOPPLER" },
                             { "dissolve", "DUST  /  VOID" }, { "wake", "WAKE" },
                             { "atmosphere", "ATMOSPHERE" }, { "shield", "SHIELD" },
                             { "impact", "IMPACT" }, { "mix", "MIX" } })
        addControl (id, name);
    setSize (980, 620);
    startTimerHz (30);
}

StarpathAudioProcessorEditor::~StarpathAudioProcessorEditor()
{
    for (auto& control : controls)
        control->slider.setLookAndFeel (nullptr);
}

void StarpathAudioProcessorEditor::timerCallback()
{
    ambienceTime += 1.0f / 30.0f;
    repaint();
}

void StarpathAudioProcessorEditor::addControl (const char* id, const char* name)
{
    auto c = std::make_unique<Control>();
    c->slider.setSliderStyle (juce::Slider::LinearVertical);
    c->slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    c->slider.setLookAndFeel (&consoleLookAndFeel);
    static constexpr std::array<juce::uint32, 4> consoleColours {
        0xffd5583f, 0xffe9913f, 0xffc59a45, 0xffb74838
    };
    const auto colour = juce::Colour (consoleColours[controls.size() % consoleColours.size()]);
    c->slider.setColour (juce::Slider::trackColourId, colour);
    c->slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffcceeff));
    c->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    c->label.setText (name, juce::dontSendNotification);
    c->label.setJustificationType (juce::Justification::centred);
    c->label.setColour (juce::Label::textColourId, juce::Colour (0xff8ebad0));
    addAndMakeVisible (c->slider);
    addAndMakeVisible (c->label);
    c->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.parameters, id, c->slider);
    controls.push_back (std::move (c));
}

void StarpathAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff030811));
    const auto visual = getLocalBounds().removeFromTop (360).toFloat();
    const auto consoleArea = getLocalBounds().withTrimmedTop (360).toFloat();
    juce::ColourGradient consoleShade (juce::Colour (0xff172127), consoleArea.getCentreX(), consoleArea.getY(),
                                       juce::Colour (0xff060a0d), consoleArea.getCentreX(), consoleArea.getBottom(), false);
    g.setGradientFill (consoleShade);
    g.fillRect (consoleArea);
    g.saveState();
    g.reduceClipRegion (visual.withTrimmedBottom (15.0f).toNearestInt());
    if (starfieldBackdrop.isValid())
    {
        const auto viewportBounds = visual.toNearestInt();
        g.drawImageWithin (starfieldBackdrop, viewportBounds.getX(), viewportBounds.getY(),
                           viewportBounds.getWidth(), viewportBounds.getHeight(),
                           juce::RectanglePlacement::fillDestination, false);
    }
    else
        g.fillRect (visual);
    g.setColour (juce::Colour (0xff02070d).withAlpha (0.22f));
    g.fillRect (visual);

    const auto vanishing = juce::Point<float> (visual.getCentreX(), visual.getCentreY() + 8.0f);

    // Slow volumetric colour maps expand from the vanishing point with independent cycles.
    static constexpr std::array<juce::uint32, 4> fieldColours {
        0xff3b7fa0, 0xff8f4e73, 0xff96723c, 0xff4b806e
    };
    for (int field = 0; field < 4; ++field)
    {
        const auto phase = std::fmod (ambienceTime / (46.0f + field * 13.0f) + field * 0.23f, 1.0f);
        const auto expansion = std::pow (phase, 1.7f);
        const auto angle = 0.8f + field * 1.47f;
        const auto centre = vanishing + juce::Point<float> (std::cos (angle), std::sin (angle))
                                    * expansion * visual.getHeight() * 0.48f;
        const auto radius = 18.0f + expansion * 230.0f;
        const auto alpha = std::sin (juce::MathConstants<float>::pi * phase) * 0.075f;
        const auto colour = juce::Colour (fieldColours[static_cast<size_t> (field)]);
        juce::ColourGradient volume (colour.withAlpha (alpha), centre.x, centre.y,
                                     colour.withAlpha (0.0f), centre.x + radius, centre.y, true);
        g.setGradientFill (volume);
        g.fillEllipse (centre.x - radius, centre.y - radius * 0.62f, radius * 2.0f, radius * 1.24f);
    }

    // Planets occupy only part of a long slot. They travel fully beyond the glass,
    // followed by a varying empty interval before the next palette and heading appears.
    constexpr float planetSlotSeconds = 118.0f;
    static constexpr std::array<float, 4> planetActiveSeconds { 72.0f, 64.0f, 78.0f, 68.0f };
    const auto planetGeneration = static_cast<int> (std::floor (ambienceTime / planetSlotSeconds));
    const auto planetLocalTime = std::fmod (ambienceTime, planetSlotSeconds);
    const auto planetActiveTime = planetActiveSeconds[static_cast<size_t> (planetGeneration & 3)];
    if (planetLocalTime < planetActiveTime)
    {
        const auto planetPhase = planetLocalTime / planetActiveTime;
        const auto planetExpansion = std::pow (planetPhase, 1.72f);
        const auto planetAngle = 0.35f + std::fmod (planetGeneration * 1.73f, 5.4f);
        const auto planetDirection = juce::Point<float> (std::cos (planetAngle), std::sin (planetAngle));
        const auto planetRadius = 7.0f + planetExpansion * 106.0f;
        const auto planetTravel = visual.getWidth() * 0.88f;
        const auto planetCentre = vanishing + planetDirection * planetExpansion * planetTravel;
        static constexpr std::array<juce::uint32, 4> planetColours {
            0xff8eb7c8, 0xffbc7d58, 0xff8c9b68, 0xff9a78ae
        };
        const auto planetVariant = planetGeneration & 3;
        const auto planetBase = juce::Colour (planetColours[static_cast<size_t> (planetVariant)]);
        const auto planetBounds = juce::Rectangle<float> (planetCentre.x - planetRadius,
                                                           planetCentre.y - planetRadius,
                                                           planetRadius * 2.0f, planetRadius * 2.0f);
        if (planetVariant == 1)
        {
            auto ringBounds = juce::Rectangle<float> (planetCentre.x - planetRadius * 1.55f,
                                                       planetCentre.y - planetRadius * 0.28f,
                                                       planetRadius * 3.1f, planetRadius * 0.56f);
            juce::Path rearRing;
            rearRing.addEllipse (ringBounds);
            rearRing.applyTransform (juce::AffineTransform::rotation (-0.17f, planetCentre.x, planetCentre.y));
            g.setColour (juce::Colour (0xffd7b77e).withAlpha (0.24f));
            g.strokePath (rearRing, juce::PathStrokeType (juce::jmax (0.55f, planetRadius * 0.045f)));
            g.setColour (juce::Colour (0xff8b6e47).withAlpha (0.19f));
            g.strokePath (rearRing, juce::PathStrokeType (juce::jmax (0.35f, planetRadius * 0.018f)));
        }
        juce::ColourGradient planetShade (planetBase.brighter (0.42f), planetCentre.x - planetRadius * 0.45f,
                                          planetCentre.y - planetRadius * 0.38f, juce::Colour (0xff03070c),
                                          planetCentre.x + planetRadius * 0.58f,
                                          planetCentre.y + planetRadius * 0.48f, false);
        planetShade.addColour (0.42, planetBase.darker (0.25f));
        g.setGradientFill (planetShade);
        g.fillEllipse (planetBounds);

        juce::Path planetDisc;
        planetDisc.addEllipse (planetBounds);
        g.saveState();
        g.reduceClipRegion (planetDisc);
        if (planetVariant == 0 || planetVariant == 1)
        {
            for (int band = -4; band <= 4; ++band)
            {
                const auto normalY = static_cast<float> (band) / 5.0f;
                const auto halfWidth = planetRadius * std::sqrt (juce::jmax (0.0f, 1.0f - normalY * normalY));
                const auto bandY = planetCentre.y + normalY * planetRadius;
                const auto bandColour = band % 2 == 0 ? planetBase.brighter (0.18f) : planetBase.darker (0.28f);
                g.setColour (bandColour.withAlpha (0.10f + planetExpansion * 0.12f));
                g.drawLine (planetCentre.x - halfWidth, bandY,
                            planetCentre.x + halfWidth, bandY + std::sin (band * 1.7f) * planetRadius * 0.035f,
                            juce::jmax (0.45f, planetRadius * 0.026f));
            }
        }
        else if (planetVariant == 2)
        {
            for (int crater = 0; crater < 9; ++crater)
            {
                const auto angle = crater * 2.19f + 0.6f;
                const auto distance = planetRadius * (0.18f + (crater % 4) * 0.16f);
                const auto centre = planetCentre + juce::Point<float> (std::cos (angle), std::sin (angle)) * distance;
                const auto size = planetRadius * (0.055f + (crater % 3) * 0.025f);
                g.setColour (planetBase.darker (0.55f).withAlpha (0.2f));
                g.fillEllipse (centre.x - size, centre.y - size * 0.7f, size * 2.0f, size * 1.4f);
                g.setColour (planetBase.brighter (0.32f).withAlpha (0.12f));
                g.drawEllipse (centre.x - size, centre.y - size * 0.7f, size * 2.0f, size * 1.4f, 0.55f);
            }
        }
        else
        {
            for (int cloud = -2; cloud <= 2; ++cloud)
            {
                const auto cy = planetCentre.y + cloud * planetRadius * 0.27f;
                g.setColour (planetBase.brighter (0.46f).withAlpha (0.075f));
                g.drawEllipse (planetCentre.x - planetRadius * 0.82f, cy - planetRadius * 0.15f,
                               planetRadius * 1.64f, planetRadius * 0.3f, juce::jmax (0.5f, planetRadius * 0.035f));
            }
        }
        g.restoreState();

        if (planetVariant == 1)
        {
            auto ringBounds = juce::Rectangle<float> (planetCentre.x - planetRadius * 1.55f,
                                                       planetCentre.y - planetRadius * 0.28f,
                                                       planetRadius * 3.1f, planetRadius * 0.56f);
            juce::Path frontRing;
            frontRing.addArc (ringBounds.getX(), ringBounds.getY(), ringBounds.getWidth(), ringBounds.getHeight(),
                              juce::MathConstants<float>::halfPi,
                              juce::MathConstants<float>::pi + juce::MathConstants<float>::halfPi, true);
            frontRing.applyTransform (juce::AffineTransform::rotation (-0.17f, planetCentre.x, planetCentre.y));
            g.setColour (juce::Colour (0xffedce93).withAlpha (0.34f));
            g.strokePath (frontRing, juce::PathStrokeType (juce::jmax (0.55f, planetRadius * 0.04f)));
        }
        g.setColour (planetBase.brighter (0.5f).withAlpha (0.18f));
        g.drawEllipse (planetBounds, 0.8f);
    }

    // Rocks share the same vanishing-point expansion as sound traffic, but move slowly.
    for (int rock = 0; rock < 3; ++rock)
    {
        const auto cycleSeconds = 24.0f + static_cast<float> (rock) * 13.0f;
        const auto phase = std::fmod (ambienceTime / cycleSeconds + static_cast<float> (rock) * 0.37f, 1.0f);
        const auto visibility = std::pow (std::sin (juce::MathConstants<float>::pi * phase), 2.0f);
        if (visibility < 0.025f) continue;
        const auto expansion = std::pow (phase, 2.0f);
        const auto heading = 0.52f + rock * 2.03f;
        const auto direction = juce::Point<float> (std::cos (heading), std::sin (heading));
        const auto centre = vanishing + direction * expansion * visual.getWidth() * 0.56f;
        const auto rockSize = 2.5f + expansion * (14.0f + static_cast<float> (rock) * 6.0f);
        juce::Path asteroid;
        for (int vertex = 0; vertex < 8; ++vertex)
        {
            const auto angle = static_cast<float> (vertex) / 8.0f * juce::MathConstants<float>::twoPi
                             + ambienceTime * (0.018f + rock * 0.007f);
            const auto irregularity = 0.72f + 0.23f * std::sin (vertex * 2.41f + rock * 1.7f);
            const auto point = centre + juce::Point<float> (std::cos (angle), std::sin (angle))
                                      * rockSize * irregularity;
            if (vertex == 0) asteroid.startNewSubPath (point); else asteroid.lineTo (point);
        }
        asteroid.closeSubPath();
        const auto rockColour = rock == 1 ? juce::Colour (0xff7f94a0) : juce::Colour (0xff665f59);
        g.setColour (rockColour.withAlpha (0.12f + visibility * 0.16f));
        g.fillPath (asteroid);
        g.setColour (juce::Colour (0xffd4c2a5).withAlpha (visibility * 0.13f));
        g.strokePath (asteroid, juce::PathStrokeType (0.55f));
    }

    g.saveState();
    g.reduceClipRegion (visual.withTrimmedBottom (15.0f).toNearestInt());
    g.setColour (juce::Colour (0xff36556a).withAlpha (0.22f));
    for (int i = -6; i <= 6; ++i)
        g.drawLine (vanishing.x, vanishing.y, visual.getCentreX() + i * 120.0f, visual.getBottom(), 0.7f);

    // Visual-only edge traffic. These dim stars never enter the Shield field and
    // remain clearly subordinate to grains that actually represent captured sound.
    for (int i = 0; i < 7; ++i)
    {
        const auto angle = 0.42f + static_cast<float> (i) * 0.91f;
        const auto phase = std::fmod (ambienceTime * (0.055f + static_cast<float> (i) * 0.006f)
                                      + static_cast<float> (i) * 0.173f, 1.0f);
        const auto radius = 0.74f + phase * 0.28f;
        const auto direction = juce::Point<float> (std::cos (angle), std::sin (angle));
        const auto point = juce::Point<float> (vanishing.x + direction.x * radius * visual.getWidth() * 0.52f,
                                               vanishing.y + direction.y * radius * visual.getHeight() * 0.53f);
        const auto tail = point - direction * (4.0f + phase * 10.0f);
        const auto starColour = (i % 4 == 0) ? juce::Colour (0xffd8b779) : juce::Colour (0xff94b9ca);
        g.setColour (starColour.withAlpha (0.10f + phase * 0.07f));
        g.drawLine ({ tail, point }, 0.38f);
    }

    for (int i = 0; i < StarpathEngine::maxFlybys + StarpathEngine::maxGhosts; ++i)
    {
        const auto s = processor.getSnapshot (i);
        if (! s.active) continue;
        const auto depth = juce::jlimit (0.0f, 1.0f, s.progress);
        const auto expansion = std::pow (depth, 2.3f);
        const auto x = vanishing.x + s.headingX * expansion * visual.getWidth() * 0.51f;
        const auto y = vanishing.y + s.headingY * expansion * visual.getHeight() * 0.52f;
        const auto length = 2.0f + expansion * (20.0f + s.size * 62.0f);
        static constexpr std::array<juce::uint32, 4> soundColours {
            0xffbdeeff, 0xffffd18a, 0xff8fe6cf, 0xffc6a4ff
        };
        const auto colour = s.ghost ? juce::Colour (0xffbd77ff)
                                    : juce::Colour (soundColours[static_cast<size_t> (juce::jlimit (0, 3, s.colourIndex))]);
        g.setColour (colour.withAlpha (juce::jlimit (0.12f, 0.92f, s.brightness)));
        if (s.impacted && ! s.ghost)
        {
            const auto impactExpansion = std::pow (s.impactPoint, 2.3f);
            const auto impact = juce::Point<float> (
                vanishing.x + s.originalHeadingX * impactExpansion * visual.getWidth() * 0.51f,
                vanishing.y + s.originalHeadingY * impactExpansion * visual.getHeight() * 0.52f);
            if (s.impactGlow > 0.015f)
            {
                const auto approachTail = vanishing + (impact - vanishing) * 0.58f;
                juce::Path deflectedPath;
                deflectedPath.startNewSubPath (approachTail);
                deflectedPath.lineTo (impact);
                deflectedPath.quadraticTo (impact.x + s.headingX * 14.0f,
                                           impact.y + s.headingY * 14.0f, x, y);

                const auto phosphor = juce::Colour (0xffb8ff72);
                g.setColour (phosphor.withAlpha (0.10f * s.impactGlow));
                g.strokePath (deflectedPath, juce::PathStrokeType (2.2f,
                                                                   juce::PathStrokeType::curved,
                                                                   juce::PathStrokeType::rounded));
                g.setColour (juce::Colour (0xffddffad).withAlpha (0.82f * s.impactGlow));
                g.strokePath (deflectedPath, juce::PathStrokeType (0.48f,
                                                                   juce::PathStrokeType::curved,
                                                                   juce::PathStrokeType::rounded));
                g.setColour (phosphor.withAlpha (0.18f * s.impactGlow));
                g.fillEllipse (impact.x - 5.0f, impact.y - 5.0f, 10.0f, 10.0f);
                g.setColour (juce::Colour (0xffefffd6).withAlpha (0.9f * s.impactGlow));
                g.fillEllipse (impact.x - 1.15f, impact.y - 1.15f, 2.3f, 2.3f);
            }
        }
        else
        {
            const auto tailScale = juce::jmax (0.0f, 1.0f - length / 180.0f);
            g.drawLine (vanishing.x + (x - vanishing.x) * tailScale,
                        vanishing.y + (y - vanishing.y) * tailScale, x, y,
                        0.8f + s.size * 3.0f);
        }
    }
    g.restoreState();

    const auto shield = processor.parameters.getRawParameterValue ("shield")->load();
    if (shield > 0.01f)
    {
        const auto pulse = 0.5f + 0.5f * std::sin (ambienceTime * 1.15f);
        const auto radius = (52.0f + shield * 112.0f) * (0.988f + pulse * 0.024f);
        juce::ColourGradient field (juce::Colour (0xff73ecff).withAlpha ((0.018f + pulse * 0.018f) * shield),
                                    vanishing.x, vanishing.y,
                                    juce::Colour (0xff4fd8ed).withAlpha ((0.006f + pulse * 0.014f) * shield),
                                    vanishing.x + radius, vanishing.y, true);
        g.setGradientFill (field);
        g.fillEllipse (vanishing.x - radius, vanishing.y - radius * 0.58f,
                       radius * 2.0f, radius * 1.16f);
        g.setColour (juce::Colour (0xff67e8ff).withAlpha (0.10f + shield * (0.18f + pulse * 0.11f)));
        g.drawEllipse (vanishing.x - radius, vanishing.y - radius * 0.58f,
                       radius * 2.0f, radius * 1.16f, 1.0f + shield * 1.45f);
        g.setColour (juce::Colour (0xffb8f7ff).withAlpha (0.035f + pulse * 0.045f));
        g.drawEllipse (vanishing.x - radius * 0.72f, vanishing.y - radius * 0.42f,
                       radius * 1.44f, radius * 0.84f, 0.55f);
    }
    g.restoreState();

    g.setColour (juce::Colour (0xffe5f7ff));
    g.setFont (juce::FontOptions (30.0f).withStyle ("Bold"));
    g.drawText ("S T A R P A T H", visual.reduced (26.0f), juce::Justification::topLeft);
    g.setFont (juce::FontOptions (11.0f));
    g.setColour (juce::Colour (0xff6e9bb2));
    g.drawText ("VECTOR GRANULAR TRANSIT ENGINE", visual.reduced (28.0f).withTrimmedTop (38.0f), juce::Justification::topLeft);

    // Cockpit sill: enough framing to suggest an instrument panel without stealing
    // control space or relying on a fixed-resolution background asset.
    g.setColour (juce::Colour (0xff101820));
    g.fillRect (0.0f, visual.getBottom() - 15.0f, visual.getWidth(), 15.0f);
    g.setColour (juce::Colour (0xffbb8550).withAlpha (0.65f));
    g.drawHorizontalLine (static_cast<int> (visual.getBottom() - 15.0f), 42.0f, visual.getRight() - 42.0f);

    // Angular cockpit frame laid over the glass.
    juce::Path leftFrame;
    leftFrame.startNewSubPath (0.0f, 0.0f);
    leftFrame.lineTo (20.0f, 0.0f);
    leftFrame.lineTo (49.0f, visual.getBottom() - 15.0f);
    leftFrame.lineTo (0.0f, visual.getBottom());
    leftFrame.closeSubPath();
    juce::Path rightFrame;
    rightFrame.startNewSubPath (visual.getRight(), 0.0f);
    rightFrame.lineTo (visual.getRight() - 20.0f, 0.0f);
    rightFrame.lineTo (visual.getRight() - 49.0f, visual.getBottom() - 15.0f);
    rightFrame.lineTo (visual.getRight(), visual.getBottom());
    rightFrame.closeSubPath();
    g.setColour (juce::Colour (0xff121a21));
    g.fillPath (leftFrame);
    g.fillPath (rightFrame);
    g.fillRect (0.0f, 0.0f, visual.getWidth(), 9.0f);
    g.setColour (juce::Colour (0xff31414b));
    g.strokePath (leftFrame, juce::PathStrokeType (1.1f));
    g.strokePath (rightFrame, juce::PathStrokeType (1.1f));
    g.setColour (juce::Colour (0xffffc67a).withAlpha (0.42f));
    g.drawLine (18.0f, 28.0f, 41.0f, visual.getBottom() - 42.0f, 0.75f);
    g.drawLine (visual.getRight() - 18.0f, 28.0f, visual.getRight() - 41.0f,
                visual.getBottom() - 42.0f, 0.75f);

    // Plating seams and recessed fasteners give the window struts a constructed surface.
    for (int seam = 0; seam < 4; ++seam)
    {
        const auto sy = 62.0f + seam * 76.0f;
        const auto inset = 20.0f + (sy / juce::jmax (1.0f, visual.getBottom() - 15.0f)) * 29.0f;
        g.setColour (juce::Colour (0xff52616a).withAlpha (0.46f));
        g.drawLine (3.0f, sy, inset - 3.0f, sy + 3.0f, 0.65f);
        g.drawLine (visual.getRight() - 3.0f, sy, visual.getRight() - inset + 3.0f, sy + 3.0f, 0.65f);
        g.setColour (juce::Colour (0xff030609));
        g.fillEllipse (inset - 7.0f, sy - 2.0f, 4.8f, 4.8f);
        g.fillEllipse (visual.getRight() - inset + 2.2f, sy - 2.0f, 4.8f, 4.8f);
        g.setColour (juce::Colour (0xffa98a63).withAlpha (0.62f));
        g.drawEllipse (inset - 7.0f, sy - 2.0f, 4.8f, 4.8f, 0.55f);
        g.drawEllipse (visual.getRight() - inset + 2.2f, sy - 2.0f, 4.8f, 4.8f, 0.55f);
    }

    // Each control is an individual inlaid console module. Child sliders and labels
    // are painted after this component, so these surfaces sit naturally behind them.
    for (size_t index = 0; index < controls.size(); ++index)
    {
        const auto labelBounds = controls[index]->label.getBounds();
        const auto sliderBounds = controls[index]->slider.getBounds();
        const auto module = labelBounds.getUnion (sliderBounds).expanded (3, 2).toFloat();
        const auto accent = controls[index]->slider.findColour (juce::Slider::trackColourId);
        juce::ColourGradient moduleDepth (juce::Colour (0xff070b0e), module.getX(), module.getCentreY(),
                                          juce::Colour (0xff070b0e), module.getRight(), module.getCentreY(), false);
        moduleDepth.addColour (0.5, juce::Colour (0xff202a2f).interpolatedWith (accent, 0.08f));
        g.setGradientFill (moduleDepth);
        g.fillRoundedRectangle (module, 5.0f);
        g.setColour (juce::Colour (0xff020405).withAlpha (0.92f));
        g.drawRoundedRectangle (module.reduced (1.0f), 4.2f, 1.2f);
        g.setColour (accent.withAlpha (0.24f));
        g.drawRoundedRectangle (module.reduced (2.2f), 3.4f, 0.65f);

        const auto plaque = labelBounds.toFloat().reduced (13.0f, 0.5f);
        juce::ColourGradient plaqueMetal (juce::Colour (0xff4a4e4b), plaque.getX(), plaque.getCentreY(),
                                          juce::Colour (0xff171c1e), plaque.getRight(), plaque.getCentreY(), false);
        plaqueMetal.addColour (0.46, juce::Colour (0xff30383a));
        g.setGradientFill (plaqueMetal);
        g.fillRoundedRectangle (plaque, 2.8f);
        g.setColour (accent.withAlpha (0.38f));
        g.drawRoundedRectangle (plaque, 2.8f, 0.6f);
    }
}

void StarpathAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (370).reduced (18, 5);
    const auto columns = 6;
    const auto cellWidth = area.getWidth() / columns;
    const auto cellHeight = area.getHeight() / 2;
    for (size_t i = 0; i < controls.size(); ++i)
    {
        auto cell = juce::Rectangle<int> (area.getX() + static_cast<int> (i % columns) * cellWidth,
                                          area.getY() + static_cast<int> (i / columns) * cellHeight,
                                          cellWidth, cellHeight).reduced (4);
        controls[i]->label.setBounds (cell.removeFromTop (18));
        controls[i]->slider.setBounds (cell);
    }
}

