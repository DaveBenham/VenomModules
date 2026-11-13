Replicate each channel of a polyphonic input with a variable detune spread, and merge the results into a single polyphonic output.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](AuxClone.md). Note that auxilliary outputs on the expander are not detuned.

### COUNT (Unison Count) knob
Sets the number of unison channels for each input channel, from 1 to 16.

### COUNT (Unison Count) input
Monophonic bipolar CV modulates the unison count, with each 1/3 volt representing 1 unison voice. The CV is summed with the Count knob value, and the result clamped to the range 1 to 16.

### DETUNE knob
Sets the detune spread for each source channel, measured in semitones. This parameter has no effect if the unison count is 1. The unison voices will be distributed evenly across the spread. The increment between voices = Spread / (Count - 1).

### DETUNE input
Monophonic bipolar input modulates the detune spread. The CV input is scaled so that 10V matches the DETUNE knob range, and then summed with the detune knob value to determine the effective detune spread. A module context menu option is available to use a V/Oct scale for the CV instead.

### DIR (Detune Direction) button
This color coded button specifies how the detune spread is applied to each replication set.
- Off (gray) = Center - The unison voices are divided evenly above and below the input V/oct. The range is (Input - Spread/2) to (Input + Spread/2).
- Green = Up - The unison voices start at the input V/oct and move up. The range is (Input) to (Input + Spread). 
- Red = Down - The unison voices start below the input and end at the input V/oct. The range is (Input - Spread) to (Input).

### RNG (Detune Range) button
This color coded button specifies the range of the detune knob:
- Off (gray) = 1 semitone = 1/12 Volt
- Blue = 1 octave = 12 semitones = 1 Volt
- Green = 5 octaves = 60 semitones = 5 Volts

### POLY input
Each channel from the polyphonic input is replicated based on the unison count as long as the total replicated channel count does not exceed 16. Input channels that cannot be replicated the full amount are ignored.

The absense of input is treated as monophonic constant 0V.

For each channel appearing at the input, the corresponding LED above glows yellow if the channel could be successfully replicated, and red if it could not be replicated. LEDs beyond the input channel count remain off (black).

### GRP (Output grouping) button
The GRP button controls how the replicated channels will be grouped at the output
- **Input channel** ***(yellow, default)*** - All the replicated channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be replicated at the output.

### POLY output
All of the replicated channels are merged into the single polyphonic output. The order of the channels is dependent on the GRP button. Detune spread for each input channel goes from low to high (unless the detune CV creates a negative spread)

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Unison is bypassed then the input is passed unchanged to the output.
