# Venom Modules Changelog

## 2.6.0
### New Modules
- Benjolin Oscillator
- Logic
- NORSIQ Chord To Scale

### Enhancements
- Improved discrimination of DC offset removal by lowering cutoff frequency to 2 Hz

### Bug Fixes
- Fixed locked switch context menu
  - Linear Beats Expander
  - Reformation
  - Shaped VCA
- Rhythm Explorer - Fixed some polyphonic port displays

## 2.5.0 (2023-11-22)
### New Modules
- Linear Beats
- Linear Beats Expander
- Mix Offset Expander
- Mix Mute Expander
- Mix Solo Expander
- Mix Fade Expander
- Mix Fade 2 Expander
- Mix Pan Expander
- Mix Aux Send Expander
- Non-Octave Repeating Scale Intervallic Quantizer

### Enhancements
- Add hover help text to all LED lights.
- All modules: Polyphonic ports now use brass cores, while monophonic ports use steel.
- Shaped VCA:
  - The response curve displayed range is now -100% exp to 100% log instead of -1 to 1
  - Improved 4 quadrant logarithmic response with option for old behavior
- Bernoulli Switch:
  - Added "Normal Value" parameter that specifies what value is normaled to the A input, either the raw trig input, or the Schmitt trigger result
  - Bernoulli Gate and Toggled Bernoulli Gate presets now send normaled Schmitt trigger gates instead of the the raw trigger input, so those modes now truly behave like the Mutable Instruments Branches.
  - Increased anti-pop crossfade time from 2.5 msec to 10 msec.

### Bug Fixes
- Bernoulli Switch: Fixed a number of bugs dealing with polyphony
- WinComp: Force minimum tolerance of 1e-6 (behind the scene) to account for limitations of float.
  
## 2.4.1 (2023-09-01)
### Enhancements
- Venom Default Dark Theme option has been added to all module context menus
- Modules using default theme use normal default if VCV dark panel option is off, and dark default if VCV dark panel option is on
- Added option to set custom default value for most parameters via parameter context menus
- Presets now save and restore parameter lock and default value settings

## 2.4.0 (2023-08-04)
### New Module
- Venom Blank

### Enhancements
- Rhythm Explorer: There are now options to select 24 or 48 PPQN input clocks, as well as options to set output gate widths to input clock width, 50% of division width, or 100% of division width.

### Bug Fix
- Rhythm Explorer: Fix initial load switch display when lock is active

## 2.3.2 (2023-07-23)
### New Module
- Bernoulli Switch Expander

## 2.3.0 (2023-07-03)
### New Modules
- Poly Unison
- Reformation
- Shaped VCA

### Behavior Change
- Bernoulli Switch: The B input now remains normalled to the A input when bypassed, which in turn can affect the bypassed B output.

### Bug Fixes
- HQ
  - The selected channel for monitoring is now saved with the patch
- Recurse Stereo
  - Fixed Right Signal output label  
- VCA Mix 4
  - Excluded patched channel outputs no longer contribute to the polyphony count in the Mix output
- VCA Mix 4 Stereo
  - Excluded patched channel outputs no longer contribute to the polyphony count in the Mix output
  - Fixed Right Mix output label

## 2.2.0 (2023-04-24)
### New Modules
- Mix 4
- Mix 4 Stereo
- VCA Mix 4
- VCA Mix 4 Stereo

### Behavior Change
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
