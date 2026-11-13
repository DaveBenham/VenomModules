Adds additional Rungler gate outputs to the [Benjolin Oscillator](BenjolinOsc.md).

Any number of Gates expanders may be used. All expanders must be placed to the right of the Benjolin Oscillator in a contiguous chain. The upper left LED glows yellow when the expander is successfully connected to a base module.

Note the gate outputs do not participate in oversampling.

### MODE (Gate Mode) button
This color coded button controls the timing and length of the gate outputs:
- **Gate** (white, default) - The gate is high as long as the gate logic is true
- **Clock gate** (yellow) - The gate is high when the gate logic is true and the Rungler clock is high
- **Inverse clock gate** (orange) - The gate is high when the gate logic is true and the Rungler clock is low
- **Trigger** (green) - A trigger is sent when the gate logic transitions to true
- **Clock rise trigger** (light blue) - A trigger is sent when the Rungler clock transitions to high while the gate logic is true
- **Clock fall trigger** (dark blue) - A trigger is sent when the Rungler clock transitions to low while the gate logic is true
- **Clock edge trigger** (purple) - A trigger is sent when the Rungler clock tranitions to high or low while the gate logic is true

Triggers are high for 1 msec. But if using a clocked trigger and the clock goes low before the trigger has completed, then the trigger immediately goes low.

### POLAR (Polarity) button
This color coded button controls the polarity of the gates
- **Unipolar** (green, default) - Low = 0V and high = 10V
- **Bipolar** (purple) - Low = -5 V and high = 5 V

### 8 Gate outputs
The LED in the upper right corner of each output port glows yellow whenever the output gate is high.

Each output port has a context menu to specify which Rungler bits are used, and what logic operator to use. By default the gate logic is used for the port name, and displayed as a label above the port.

#### Gate bits
You may select up to 4 bits. Once 4 have been selected you cannot select another without first unselecting one.

#### Gate logic
You must select one of three options:
- **AND** (&) - The gate logic is true if all selected bits are high
- **OR** (|) - The gate logic is true if at least one selected bit is high
- **XOR** (^) - The gate logic is true if exactly one selected bit is high

If only one bit is selected, then all logic operations give the same result - the logic is true if the selected bit is high.

### Factory Presets
There are four factory presets that default to mode = Gate and polarity = Unipolar
- All Bits - There is one gate for each Rungler bit
- AND Gates - AND logic is used with bits 1&2, 2&4, 4&7, 1&2&4&7, 2&3, 3&5, 5&8, 2&3&5&8
- OR Gates - OR logic is used with bits 1|2, 2|4, 4|7, 1|2|4|7, 2|3, 3|5, 5|8, 2|3|5|8
- XOR Gates - XOR logic is used with bits 1^2, 2^4, 4^7, 1^2^4^7, 2^3, 3^5, 5^8, 2^3^5^8

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V when the Benjolin Gates Expander is bypassed.
