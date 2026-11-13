Adds an additional Rungler CV output to the [Benjolin Oscillator](BenjolinOsc.md). It works as a configurable digital to analog converter.

Any number of Volts expanders may be used. All expanders must be placed to the right of the Benjolin Oscillator in a contiguous chain. The upper left LED glows yellow when the expander is successfully connected to a base module.

Note the output does not participate in oversampling.

### SNAP button
If on (white), then the Bit knobs snap to powers of 2. If off (gray) then the knobs can be freely set to any decimal value betwen 0 and 128.

### Bit knobs 1-8
Each knob represents a Rungler bit. The knob assigns a value to the bit between 0 and 128. Knobs (bits) set to 0 do not participate in the digital to analog conversion. The assigned values for all high Rungler bits are summed, and then scaled and offset to a bipolar +/- 5V range (10 VPP).

### RANGE knob
Scales the peak to peak range of the output to any value between 0 and 10V.

### OFFSET knob
Offsets the output by any value between -10V and 10V. The Offset is applied after the Range.

### OUT port
The final computed voltage is ouput here.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The output is constant monophonic 0V when the Benjolin Volts Expander is bypassed.
