# Venom Modules Changelog

## 2.12.0 (????-??-??)
### New Modules
- Cross Fade 3D
- Pan 3D
- Sphere To XYZ

### Enhancements
- VCO Lab and VCO Unit
  - Added a module context menu option to measure LFO frequency in BPM instead of Hz
  - Added pseudo PWM shape modes to sine, triangle, and saw, as well as skew shape modes to sine and triangle.
- VCO Unit
  - Shape mode now always has 8 options, regardless which waveform is selected. The square waveform rotates through the three modes to make 8. This change guarantees that cycling through all the waveform options will not change the shape mode for a particular waveform.
- Poly Fade
  - Added context menu option to disable output channel minimization
- Multi Merge
  - Add context menu options to explicitly set the number of channels on inputs, overriding the actual input channel count.
- Rhythm Explorer
  - Added 96 ppqn clock option
  - Added dotted rate options
- Poly Unison
  - Detune CV now scales 10V to match the detune knob range
    - Old patches default to old behavior of V/Oct for Detune CV
- Bay Modules better preserve links when performing duplication or copy/paste
  - Duplication of input/output pairs establish the link between the new copies (old behavior)
  - Duplication of linked outputs only attempt to establish the link to the original uncopied input (new behavior)

### Bug Fix
- WinComp
  - Absolute value was applied to Tolerance input, but not the tolerance offset parameter
  - Module now correctly sums the tolerance input with the offset before taking the absolute value

### Additional Change
- Mix 4, Mix 4 Stereo, VCA Mix 4, and VCA Mix 4 Stereo
  - In the interest of avoiding memory allocation during the audio process, the Mix modules are now limited to a maximum of 16 expanders. Prior to v2.12 there was no limit.

## 2.11.1 (2024-12-17)
### Enhancements
- All modules with oversampling options
  - New context menu option to select the quality of the oversampling filters
    - New patches default to 10th order (better)
    - Pre-existing patches use 6th order
- VCO Lab and VCO Unit
  - Improved audio quality of square and saw waves with DPW anti-aliasing
    - Added context menu option to disable DPW
    - Pre-existing patches default to DPW disabled
  - Added option to set the sync trigger thresholds
    - Default continues to be 2V high, 0.2V low, which works for both unipolar and bipolar triggers
    - 0V high, -2V low triggers at 0 crossing point, but only works with bipolar triggers
  - Removed max frequency limit

### Bug Fixes
- VCA Mix 4 and VCA Mix 4 Stereo: Band limiting (oversampling) VCA Mode options were improperly implemented

## 2.11.0 (2024-11-11)
### New Modules
- Wave Folder

### Enhancements
- Poly Fade
  - Improved audio quality
    - New phasor input slew rate parameter, defaults to off
    - Removed audio pops when changing internal phasor direction
    - Reduced audio pops when changing channels or start channel
  - New context menu option to disable "Reset if direction off"
  - ***(breaking change)*** Ping pong internal phasor rate is now 1/2 displayed rate to preserve constant rate of channel switching

### Bug Fixes
- Repaired default themes system that was broken in v2.4.1

## 2.10.0 (2024-10-25)
### New Modules
- Mouse Pad
- Poly Fade
- Quad VC Polarizer
- VCO Unit

### Enhancements
- Mix Pan Expander
  - Add polyphony support to CV inputs
- VCO Lab
  - Add waveform morphing shape modes
    - Sine: square <--> sine <--> saw
    - Triangle: sine <--> triangle <--> square
    - Square: triangle <--> square <--> saw
    - Saw: square <--> saw <--> even
  - Sum of Level parameter and Level CV now limited to 100%
    - Module context option to disable Level limit
    - Old patches default to unlimited
  - Level CV can now be configured for unipolar or bipolar (ring mod) input
    - Default is unipolar
    - Old patches default to bipolar
  - Level CV Ring Mod context menu option renamed VCA unity = 5V
  - Square Full PWM shape option color changed from blue to orange to match new VCO Unit module

### Bug Fixes
- Bay Modules
  - Fix crash on some platforms when a linked Bay Input is deleted.
- Benjolin Gates Expander
  - Fix LED gate indicators broken in v2.9.1
- Bypass
  - Trigger button did not work properly via PatchMaster control when in momentary mode. It now works properly as long as Trigger input not patched.
- Mix 4 and VCA Mix 4
  - Bug fix for Pre Offset when using the Mix Offset expander

## 2.9.1 (2024-09-08)
### Enhancements
- Benjolin Oscillator
  - Added menu option to configure Rungler DAC to use bits 6,7,8 as Rob Hordijk's original design instead of bits 2,4,7
  - Significantly improved CPU usage of Volts and Gates expanders

## 2.9.0 (2024-08-22)
### New Modules
- Benjolin Gates Expander
- Benjolin Volts Expander
- Thru

### Enhancements
- All Venom modules with expanders
  - Added context menu options to add expanders
- Mixers
  - Added an extra clipping mode for saturation limiting at +/- 6V
- Benjolin Oscillator
  - Detune VCO 1 a tiny bit so PWM is not flat at initiation
  - Add context menu option for unipolar clock input
- VCO Lab
  - Added shape mode options to Sin, Tri, and Saw waveforms
  - Rearranged faceplate to be more logical
  - Changed Sin, Tri, and Saw shape CV so 10 VPP covers full range, with option for old behavior
  
### Bug Fixes
- VCO Lab
  - Fix Polyphonic CV level when level input is monophonic
- Logic
  - Fix locking for reuse and operator parameters

## 2.8.0 (2024-06-26)
### New Modules
- Bay Input
- Bay Normal
- Bay Output
- Blocker
- Bypass
- VCO Lab

### Enhancements
- Benjolin Oscillator
  - Minor reduction in CPU usage
- Knob 5
  - New quantize options: Off, Integers, Semitones
  - New display unit options: Volts, Cents
  - Reduced CPU usage
- Linear Beats
  - Save toggle CV context menu option with patch
- Mix Aux Send Expander
  - Add a chain option for use when VCA mixers are chained
- Poly Offset
  - New quantize options: Off, Integers, Semitones
  - New display unit options: Volts, Cents
  - BREAKING CHANGE: mono input now cloned to match selected channel count
- Poly Scale
  - New output channel count option
  - Mono input cloned to match selected channel count
- Shaped VCA
  - Allow exponential shape output to be bipolar in bipolar VCA mode
  - Added additional VCA modes to better work with +/- 5V inputs with shaping
  - Changed Bias knob to bipolar +/- 5V instead of unipolar 0-5V
  - Modified context menu options for backward compatibility

### Bug Fixes
- Benjolin
  - Clamped the oscillator frequency to between ~0.03 Hz and ~12.5 kHz to prevent loss of output.
- Mix Fade and Mix Fade 2
  - Fixed range of level output to 0-10 V. Was erroneously 0-1 V.
- VCA Mix 4 and VCA Mix 4 Stereo
  - Fixed output polyphony for excluded inputs
  - Fixed right chain input normalization to left chain input in the stereo version
  - Fixed summing of polyphonic chain inputs to a single channel if in Unipolar Poly Sum mode
- All modules that have an option for DC offset removal
  - The old DC removal algorithm was failing on some machines, so it has been replaced by an entirely new algorithm
  - This has the potential to alter the sound of older patches that use Venom DC offset removal

## 2.7.0 (2024-03-11)
### New Modules
- Auxilliary Clone Expander
- Multi Merge
- Multi Split
- Poly Offset
- Poly Scale

### Enhancements
- Modified Clone Merge, Poly Clone, and Poly Unison to work with the Auxilliary Clone Expander

### Bug Fixes
- Fixed a bug with Venom expander modules. Prior to version 2.7, Venom expander modules could misbehave and possibly crash VCV Rack if an expander or base module was deleted.

## 2.6.1 (2024-02-28)
### Enhancements
- Harmonic Quantizer: Add detune parameters
- Parameter tooltips: Moved "locked" indicator from name to description
- Port tooltips: Added normalled values to description
- Light tooltips: Moved color key from name to description

### Bug Fixes
- Benjolin Oscillator: Fix CV1, CV2, Clock normalled values (were 20% of intended value), with option to use old behavior

## 2.6.0 (2024-02-14)
### New Modules
- Benjolin Oscillator
- Knob 5
- Logic
- NORSIQ Chord To Scale
- Poly Sample & Hold Analog Shift Register
- Push 5
- Widget Menu Extender

### Enhancements
- Improved discrimination of DC offset removal by lowering cutoff frequency to 2 Hz
- All Venom parameters and input/output ports have user configurable names (except Rhythm Explorer)

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
