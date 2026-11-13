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
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If the module is bypassed then the Trig input is passed unchanged to the Trig output. All other bypassed outputs are monophonic constant 0 volts.
