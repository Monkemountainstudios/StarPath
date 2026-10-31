# Starpath design notes

## Identity

Starpath is a sister effect to Event Horizon. Event Horizon describes orbital motion; Starpath describes transit. Its core metaphor is a continuous cockpit view in which captured sound becomes traffic moving from a central vanishing point, past the listener, and into a receding wake.

The short description is: **a granular cloud with a vector attached to every grain**.

## Event life cycle

1. Capture material continuously into rolling stereo memory.
2. Spawn a grain near the spatial centre and assign a 0–360° viewport heading.
3. Approach: amplitude, apparent size, brightness, and positive Doppler increase.
4. Closest point: the trajectory commits left or right and reaches maximum spatial width.
5. Departure: Doppler reverses and the grain dissolves toward high-pass Dust or low-pass Void.
6. Wake: chorus and atmosphere preserve a short diffuse trace.

## Population controls

Controls describe tendencies, never absolute values. Each event draws from a biased distribution. A low-probability anomaly path deliberately creates objects outside the normal range.

- **Warp:** typical transit time. Rare drifters and extreme streaks remain possible.
- **Traffic:** mean capture/spawn density, with clusters and gaps.
- **Mass:** bias toward small or large events without excluding either extreme.
- **Trajectory:** bias toward distant or close passes.
- **Scatter:** variance across timing, speed, trajectory, pitch, and filtering.
- **Doppler:** perceptual pitch-motion depth rather than literal physical simulation.
- **Dust / Void:** bipolar departure filter; high-pass fragmentation versus low-pass disappearance.
- **Wake:** short modulated diffusion attached to moving bodies.
- **Atmosphere:** persistent world reverb.

Future population controls worth testing are **Memory** (property inheritance between adjacent grains) and **Streams** (independent traffic versus coherent showers).

## Shield

Shield is a collision system, not a conventional insert effect. On interception:

- the recognizable body is pushed outward and slightly attenuated;
- a delayed spectral ghost is emitted from the impact point;
- ghost repeats alternate sides, lose energy, drift in pitch, and become less recognizable.

Future Shield parameters may include Material, Deflect, Ghost Delay, and Surface Travel. Large slow events should excite deep resonances; small fast events should create bright scattered ticks.

## Visual language

The reference mood board establishes hierarchy rather than a literal production asset:

- wide forward viewport and central vanishing point;
- dark cockpit framing with restrained warm instrument light;
- audio-driven star streaks in the foreground;
- a deterministic star field, nebula band, and shaded planet as deep-field parallax anchors;
- a visible shield ellipse whose energy follows Shield amount.

Deep-field objects should eventually drift at separate sub-pixel rates. Their slow parallax is important because it makes the foreground traffic appear much faster.

## Musical safety model

- Base-rate variance stays within fractions of a semitone in the normal population.
- Doppler uses a broad musical range followed by a nonlinear extreme zone above roughly 72%.
- Flybys use overlapping Hann-windowed read heads to loop captured material without grain-boundary clicks.
- The current texture population mixes sustained bodies, echo-pulsed bodies, and gently fluttering bodies.
- Grains begin almost mono and gain stereo width near closest approach.
- A soft safety curve catches dense traffic after the atmosphere return without imposing a hard digital clip.
- Shield ghosts favour unison, with rarer stable octaves and fifths; repeats do not accumulate pitch drift.
- Sound-grain spawning is gated by protected input activity. Silence creates no new Shield collisions.
- A separate visual-only population supplies a few dim edge stars; these never enter or strike the Shield.
- Audio grains use a restrained cyan, amber, green, and violet palette so captured sound reads separately from background motion.
- Moving geometry is hard-clipped to the viewport glass and can never paint into the control panel.
- Wake is a cross-coupled receding feedback trail. Its upper range is intentionally capable of very long decays.
- Up to two rare 0.85–1.4 second “comet” grains may appear outside the normal Mass-biased population.
- Roughly 14% of grains read their captured material in reverse.
- Above 30%, Shield fades in a separate conventional delay: 700–1200 ms, 20–40% feedback, and up to roughly 46% return level.
- Shield size grows substantially with its control value, visually matching the larger protected field.
- The cockpit frame overlays the viewport; planet, nebula, and three families of asteroid visitors drift on independent long cycles.
- The supplied starfield JPEG is embedded into the binaries as the viewport backdrop.
- Rocks, volumetric colour fields, and successive planet variants all expand outward from the vanishing point; lateral deep-field travel is avoided.
- Shield uses a gently pulsing radial surface and inner contour rather than only an outline.
- Controls use custom slanted linear console tracks in red, amber, and ochre families.
- Console controls are vertical segmented faders arranged as two rows of six.
- A single outer glass clip contains the complete deep-field render, preventing backdrops and large planets from entering the control panel.
- Vertical fader chassis are straight and symmetric; cursors remain clamped inside the track at minimum and maximum.
- Planets fully clear the viewport during 64–78 second transits, followed by deterministic 40–54 second empty intervals.
- Windows standalone builds explicitly enable JUCE's ASIO backend; a compatible manufacturer driver must still be installed.
- Shield amount is also interception probability: an eligible grain makes one collision roll at the field. Passing grains remain completely unaltered; intercepted grains alone deflect and emit ghosts. At 100%, all eligible grains collide.
- Doppler and Shield-heading transitions are continuous at closest approach to avoid isolated click discontinuities.
- Control columns are recessed gradient modules with metal name plaques; window struts include plating seams and fasteners.
- Planet generations rotate through banded, ringed, cratered, and clouded surface families.
- `Assets/StarpathIcon.png` supplies the standalone and plugin bundle icons.

## Factory Preset

The sole built-in program is named **Factory Preset** and opens new instances at the approved reference sound: Warp 0.2884615, Traffic 0.6484375, Mass 1.0, Trajectory 1.0, Scatter 0.3164062, Doppler 0.0703125, Dust/Void 0.3604375, Wake 0.7890625, Atmosphere 0.988, Shield 0.3867188, Impact 1.0, and Mix 0.636. Numeric text boxes are hidden; the console faders communicate values visually.

## Chaining

- **Starpath → Reverbulator:** discrete trajectories seed large spatial smears.
- **Reverbulator → Starpath:** a cloud is sampled into moving spectral bodies.
- Preserve meaningful gaps in Starpath's own atmosphere so downstream effects have articulated events to transform.
