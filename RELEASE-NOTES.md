# Starpath 0.1.1

This patch gives Starpath deliberate gain staging without restraining its dense
flybys, long wakes, or impact ghosts.

## Changes

- Compact **IN** and **OUT** faders now sit in the cockpit window struts.
- IN spans -18 to +6 dB and defaults to -6 dB for usable capture headroom.
- OUT spans -18 to +6 dB before a stereo-linked -0.3 dBFS safety ceiling.
- Gain changes are smoothed, existing automation ordering is preserved, and
  regression tests now verify exact trim gain, positive drive, and peak safety.

## Included downloads

- **Starpath-Windows-x64.zip** — VST3 and standalone application
- **Starpath-macOS-Universal.zip** — Universal VST3, Audio Unit, and standalone application

## Highlights

- Musical 360-degree granular flybys with Doppler motion
- Long modulated wakes and atmospheric space
- Probabilistic Shield interception with spectral impact ghosts
- Animated cockpit interface with audio-reactive traffic
- Built-in Factory Preset

See the included README for installation and control documentation.

