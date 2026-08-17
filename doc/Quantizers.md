# Quantizers
- [HARMONIC QUANTIZER](#harmonic-quantizer)
- [NON-OCTAVE REPEATING SCALE INTERVALLIC QUANTIZER](#non-octave-repeating-scale-intervallic-quantizer)
- [NORSIQ CHORD TO SCALE](#norsiq-chord-to-scale)
- [RATIO](#ratio)

[Quantizers top](#quantizers) | [Venom top](/README.md#venom)

## HARMONIC QUANTIZER
![Harmonic Quantizer module image](HQ.PNG)  
Computes a selected harmonic or subharmonic partial relative to a fundamental root V/Oct, or quantizes an input V/Oct to the nearest partial relative to the root.

This module is fully polyphonic. The number of output channels is the maximum channel count found across all three inputs. Any monophonic input is replicated to match the output channel count. A polyphonic input with fewer channels uses 0V for any missing channels.

### Digital Display (unlabled)
Displays the integral partial number that is currently being output for a single channel. A yellow value indicates a positive value, representing a true partial from the natural harmonic series. A red value indicates a negative value, representing a subharmonic.

By default the display monitors channel 1. A different channel may be selected via the module context menu. The display will be blank if the selected channel is Off, or greater than the number of output channels.

### Harmonic Series switch (unlabled)
Selects one of three possible series
- **A** = All partials -  (default)
- **O** = Odd partials
- **E** = Even partials (includes the fundamental partial, even though it is odd)

The output will be constrained to the selected series.

### Partial knob
Selects a single integral partial number. By default the range is from 1 (fundamental) to 16 (4 octaves above the fundamental). The context menu allows selection of any one of the following ranges:
- 1 to 16 (default)
- 1 to 32
- 1 to 64
- 1 to 128
- -16 to 1
- -32 to 1
- -64 to 1
- -128 to 1
- -16 to 16
- -32 to 32
- -64 to 64
- -128 to 128

Negative values refer to subharmonics. There is no 0 partial, and both -1 and 1 refer to the same fundamental root. So both 0 and -1 are skipped within the sequence of allowable values. 

### CV knob and input
The bipolar CV input modulates the selected partial, adding or subtracting 1 partial for every 0.1 volt. The CV input is attenuated and/or inverted by the CV knob. The resultant partial is clamped to within the currently active Partial range. Keep in mind that 0 and -1 are skipped when computing the end result, so a knob value of -2 + 0.1 volt modulation yields 1, not -1.

The Partial knob, CV knob, and CV input are all ignored if the IN input is patched (although the CV input can still modify the number of output channels).

### DETUNE
Harmonics (integral ratios) are often used to create harmonious results with things like frequency modulation, phase modulation, and ring modulation. But if the harmonic is detuned slightly, it can give a richer sound, much like detuned unison voises. The result often has a beat tone. If the amount of detune (measured in cents) remains constant, the beat tone frequency increases as the pitch rises. If the degree of detuning is halved for every rise of one octave, then the beat tone rate remains constant across all frequencies.

#### AMT (Detune Amount) knob
This bipolar knob sets the base amount of detuning. The default at noon gives no detuning. Turning to the left (counterclockwise) detunes to lower frequencies, and to the right (clockwise) detunes to higher frequencies.

#### COMP (Detune Frequency Compensation) knob
This unipolar knob controls how much compensation is applied to keep the detuned beat tone rate more constant as frequency changes. The fully counterclockwise value of 0 applies no compensation - the detune amount remains constant at all frequencies, and the beat tone rates vary the most. The fully clockwise value of 1 decreases the detune amount by 1/2 for every octave rise, and the beat tone rate remains constant at all frequencies.

### ROOT input
Establishes the fundamental V/Oct value (1st partial)

### IN input
If this port is patched, then the V/Oct input is quantized to the nearest harmonic or subharmonic partial relative to the ROOT input. The computation is clamped to within +/- 128 partials (+/- 7 octaves). The Partial knob, CV knob, and CV input are all ignored when quantizing IN input.

### OUT output
The final computed partial is converted into a delta V/Oct and added to the ROOT to establish the final output V/Oct value.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The IN input is passed unchanged to the OUT output when the Harmonic Quantizer is bypassed.

[Quantizers top](#quantizers) | [Venom top](/README.md#venom)


## NON-OCTAVE REPEATING SCALE INTERVALLIC QUANTIZER
![Non-Octave Repeating Scale Intervallic Quantizer image](NORS_IQ.png)  
Quantizer for any scale with up to 13 intervals between notes. The scale is defined by a root note for the scale, followed by a series of intervals. The first interval is added to the root to get the 2nd note in the scale. The second interval is added to the 2nd note to define the 3rd note, etc. The final interval defines the step from the Nth note of the scale to the root of the next pseudo-octave in the series. The total interval from one root to the next need not be an octave.

The module is also convenient for defining extended chords based on intervals.

### Interval Unit switch
This switch controls what unit is used to display and or manually enter intervals.
- **V/Oct**: 1 V = 1 octave.
- **Cents** (default): 100 cents = one 12 Equal Tempered half step, and 1200 cents = one octave. The scale and/or interval need not have any relationship to 12 ET - the cents unit is just a convenient way to measure an interval.
- **Ratio**: Interval frequency ratios are unitless, and are defined as the frequency of the high note / frequency of the low note. For example, a perfect fifth may be entered as 3/2, but the ratio will always be displayed relative to 1. So a perfect fifth would be displayed as 1.5:1

### Equal Divs button
Controls how intra-note intervals are defined.

If On (default), then intra-note intervals are specified as an integral number of chromatic steps that are defined by an equal division (EDPO) of some larger interval (Pseud-Oct Interval).

If the Equal Divs button is off, then the Pseud-Oct Interval and EDPO values are ignored, and intra-note intervals are specified directly as an interval.

### Pseud-Oct Interval knob and input
Defines the pseudo-octave interval that will be divided into equal divisions to define the chromatic interval for the scale. The default knob value is one octave. The interval unit is specified by the Interval Unit switch.

The monophonic input always uses the V/Oct standard, regardless how the Interval Unit switch is configured.

The effective pseudo-octave interval is the sum of the knob and input values, clamped to a value between 0 and 4 octaves. The effective value is displayed below the knob, using the units defined by the switch.

The PseudoOctave Interval is ignored and nothing is displayed if the Equal Divs button is off.

### EDPO (Equal Divisions per Pseudo Octave) knob and input
Defines the number of equal divisions of the pseudo-octave interval, which in turn defines the scale's chromatic step interval. The default value is 12.

The monophonic input is scaled at 10 divisions per volt.

The effective number of divisions is the sum of the knob and input values, rounded and clamped to an integral value between 1 and 100. The effective value is displayed below the knob.

The EDPO is ignored if the Equal Divs button is off.

### Scale Length and input
Defines the number of intervals or steps in the repeating scale (notes - 1). The final interval defines the jump from the Nth note in the scale to the root of the next pseudo-octave. The default value is 12.

The monophonic input is scaled at 0.5 Volt per step.

The effective scale length is the sum of the knob and input values, rounded and clamped to an integral value between 1 and 13. The effective value is displayed below the knob.

### Scale Root knob, input, and Root Unit switch
Defines the root note of the repeating scale.

The switch beside the knob defines what unit is used for display and manual entry.
- **V/Oct**: 1 V = 1 octave, where 0 V represents the note C4
- **Cents**: 100 cents = one 12 ET half step, where 0 cents represents the note C4
- **Freq** (default): Measures the root note frequency in Hz.

The knob default is always C4, regardless which unit is used.

The monophonic input always uses the V/Oct standard.

The effective Scale Root is the sum of the knob and input values, clamped to a value between -4V (C0) and 4V (C8). The effective value is displayed below the knob, using the units defined by the switch.

### Round switch
Determines how input voltages are quantized
- **Up**: Input values between two notes are always rounded up to the higher note
- **Nearest** (default): Input values between two notes are always rounded to the closest note, with values at the halfway point rounded up
- **Down**: Input values between two notes are always rounded down to the lower note

### Equi-likely button
If enabled, then the notes of the scale are mapped to even divisions of the total scale interval, such that each note of the scale has equal probability when the input is fed random values.

By default this mode is off.

### Scale interval knobs and inputs 1 through 13, and Poly Interval input
The 13 small knobs and monophonic inputs below the display area define the intervals used in the scale. Up to 13 channels in the Poly Interval input can also be used to modulate the 13 intervals.

The method used to define the interval depends on the Equal Divs button.

#### Equal Divs ON
Each interval is defined as an integral number of chromatic steps as defined by the Pseudo-Octave Interval and the EDPO. The left most knob defines the interval between the root note and the 2nd note of the scale. The right-most active knob defines the interval between the Nth note of the scale and the root of the next pseudo-octave.

Each knob defaults to 1, with a range of 1 to 100.

The monophonic inputs are scaled at 10 chromatic steps per volt. 

The effective count is the sum of the knob and input values, rounded and clamped to an integral value between 1 and 100. The effective count is displayed above the respective knob. Inactive interval knobs, as defined by the Scale Length, do not have any value displayed above them.

#### Equal Divs OFF
Each interval is specified directly.

The knob value defaults to 0 (no change in pitch), with a range from 0 to 2 octaves. The unit used for manual entry and display is controled by the Interval Unit switch.

The monophonic input is scaled at 1 volt per octave.

The effective interval is the sum of the knob and the input values, clamped to a value between 0 and 2 octaves. The effective interval value is displayed above each respective knob. Inactive interval knobs, as defined by the Scale Length, do not have any value displayed above them.

### Display Panel
The display panel is divided into 3 sections. As already described above, the top section displays the effective values for the upper controls, and the bottom section displays the effective values for the 13 interval controls.

The middle section indicates which note within the scale is selected. Each polyphonic channel has its own position within a 4 x 4 grid above each interval.

The left most grid above the left most interval knob represents the root of the scale. The next grid represents the 2nd note, etc.

The indicator is yellow when the selected note is >= the effective Scale Root, and red if < the Scale Root. The character within each indicator represents the distance away from the root (pseudo)octave. A value of 0 represents the scale containing the Scale Root. The (pseudo)octave indicators are coded as follows:
 - 0-9 = 0-9
 - A-Z = 10 - 35
 - a-z = 36 - 61
 - \<space\> 62 or higher

### IN input
The polyphonic IN input provides the V/Oct frequency control voltage to be quantized.

### Trig input
If the polyphonic Trig input is patched, then input values are only quantized when a trigger is received. Each quantized value is held until the next trigger is received.

If the Trig input is not patched, then input V/Oct values are continuously quantized.

### Chromatic Scales
Equal Division Chromatic scales with more than 13 intervals may be accomplished by setting the Scale Length to 1 and the first (and only) interval to 1.

### OUT output
The polyphonic OUT output produces the quantized V/Oct values.

### P-OCT (Pseudo-Octave) output
This polyphonic output produces integral voltages that represent the pseudo-octave of each quantized note. A value of 0 represents the scale pseudo-octave containing the effective Scale Root. A value of 1 represents the next scale pseudo-octave up. A value of -1 represents the first pseudo-octave below the effective Scale Root. And so on...

This value will not have much meaning if using a chromatic scale with Scale Length = 1.

### Trig output
If the Trig input is not patched, then a 1ms 10V trigger is issued every time a new note is quantized.

If the Trig input is patched, then the Trig input is passed through directly to the Trig output.

### Polyphony channel count
The polyphony channel count for the three outputs above are computed as the maximum number of channels found between the IN and Trig inputs.

If one input is monophonic, and the other polyphonic, then the monophonic values are replicated to match the channel count of the polyphonic input.

If both inputs are polyphonic, but one input has fewer channels than the other, then missing channels are treated as constant 0V.

### Scale output
This polyphonic output produces all the V/Oct values for the scale starting at the Scale Root, through the first note of the next scale one scale pseudo-octave up. The number of channels will be the Scale Length + 1.

This output can also be used as an extended chord rooted at the Scale Root. Simply define the intervals between the notes in your chord, patch the root of the chord to the Scale Root input, and leave the IN input unpatched.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If the module is bypassed then the Trig input is passed unchanged to the Trig output. All other bypassed outputs are monophonic constant 0 volts.

[Quantizers top](#quantizers) | [Venom top](/README.md#venom)


## NORSIQ Chord To Scale
![NORSIQ Chord To Scale module image](NORSIQChord2Scale.png)  
Converts up to 14 channels of a polyphonic "chord" input into a set of CV outputs that define a scale for the Non-Octave-Repeating Scale Intervallic Quantizer (NORSIQ).

### General Operation
Polyphonic V/Oct channels at the CHORD input are sorted, and the lowest note defines the scale root, the number of channels determines the scale length, and the intervals between the sorted notes define the intervals for the NORSIQ. The computation of the defining scale CV may be continuous, or it may be held constant until a trigger is received.

#### NORSIQ configuration
The NORSIQ module must be configured properly for the CV from the NORSIQ Chord To Scale module to define the correct scale:
- The EQUAL DIVS button must be off so that intervals can be defined directly.
- All 14 Interval knobs must be fully counter-clockwise (0V, 0 cents, or 1:1 ratio)
- The SCALE LENGTH knob must be fully counter-clockwise (1)
- The SCALE ROOT should be at the default noon position (C4), unless you want to transpose the scale to a different key.
  
The PSEUD-OCT INTERVAL and EDPO knobs and inputs are ignored. The INTERVAL UNIT, ROOT UNIT, ROUND, and EQUI LIKELY contols may configured as you see fit.

The NORSIQ has a "NORSIQ Chord To Scale configuration" factory preset that quickly sets the appropriate configuration with some parameters locked to make sure it works properly.

### OCTAVE FOLD button
Controls where the scale repeats
- **Off (gray - default)**: The last (highest) note of the sorted chord input defines the last note of the scale, which becomes the root of the next scale.
- **On (white)**: An extra note that is an octave multiple above the scale root is added to define the last note of the scale. The lowest root octave that is above the highest chord note is used. If the highest chord note is already an octave of the root, then it becomes the last note of the scale, and no note is added.

### TRIG input
If patched, then the outputs will not change until a monophonic trigger is received. The trigger is detected by a Schmitt trigger with a high threshold of 2V and low threshold of 0.1V.

If left unpatched, then the outputs are continuosly updated according to the current inputs.

### CHORD input
The polyphonic V/Oct input used to define the scale. If the OCTAVE FOLD is off, then only the first 14 channels are used. If the OCTAVE FOLD is on, then only the first 13 channels are used.

### TRIG output
The result of the TRIG input Schmitt trigger is forwarded to the TRIG output so that the trigger remains in sync with any scale changes.

### ROOT output
This monophonic output is typically patched to the NORSIQ SCALE ROOT input to define the root of the scale according to the lowest sorted note from the CHORD input. The ROOT output can be ignored if you want to transpose the scale to a different key.

### LENGTH output
This monophonic output must be patched to the NORSIQ SCALE LENGTH input to define the length of the scale.

### INTVLS output
This polyphonic output must be patched to the NORSIQ POLY INTERVALS input to define the interval between each of the notes in the scale.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V if NORSIQ Chord To Scale is bypassed.

[Quantizers top](#quantizers) | [Venom top](/README.md#venom)


## RATIO
![Ratio module image](Ratio.png)  
Computes V/Oct CV for musical ratios using integral numerators and denominators.

[Quantizers top](#quantizers) | [Venom top](/README.md#venom)


