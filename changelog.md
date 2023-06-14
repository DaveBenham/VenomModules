# Venom Modules Changelog

## 2.?.? (2023-??-??)
### New Modules
- Poly Unison

### Bug Fixes
- HQ
  - The selected channel for monitoring is now saved with the patch
- VCA Mix 4, VCA Mix 4 Stereo
  - Excluded patched channel outputs no longer contribute to the polyphony count in the Mix output

## 2.2.0 (2023-04-24)
### New Modules
- Mix 4
- Mix 4 Stereo
- VCA Mix 4
- VCA Mix 4 Stereo

### Behavior Changes
- Recurse Stereo: The right input now remains normalled to the left input when bypassed, which in turn can affect the bypassed right output.

### Enhancements
- Each Venom plugin parameter (knob, switch, button, etc.) has been given context menu options to lock or unlock the parameter.
- Each Venom plugin module has been given module context menu options to lock or unlock all parameters of that module instance.
- The new parameter lock/unlock options do not apply to Rhythm Explorer as it already had its own parameter locking implementation.

### Discontinued
- Removed the hidden VCO that will never be released. When/if I do release a VCO, it will be entirely different code.

## 2.1.4 (2023-03-20)
### Enhancements
- Bernoulli Switch
  - Add anti-pop switching and oversampling options to the module context menu to better support audio output
  - The A and B outputs now produce the same number of channels in all cases
  - Add presets to emulate the four possible Mutable Instruments Branches configurations
- WinComp
  - Add oversampling module context menu options to better support audio output
  - Move Absolute Value and Invert module context menu options to individual port context menus
- Recurse and Recurse Stereo
  - Show all normalled connections on faceplate
- Rhythm Explorer
  - Add "All - Reset" and "All - New" division modes. The "All - New" option enables creation of multiple rhythm generators that share a common clock, as well as shared  reset and dice triggers.
  - Add Reset Timing context menu option
  - Dice and Reset buttons now remain lit until they take effect
  - Preserve seed between sessions
  - User presets now preserve seed so entire pattern can be saved
  - Cleanup run start behavior
  - Update Vermona Random Rhythm factory preset to support two rhythm generators, like the Vermona.
  - Round the square button corners

## 2.0.1 (2023-02-12)
### Initial release
- Bernoulli Switch
- Clone Merge
- Harmonic Quantizer (HQ)
- Poly Clone
- Recurse
- Recurse Stereo
- Rhythm Explorer
- WinComp
