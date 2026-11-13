Provides an offset control for each channel of a polyphonic signal. For each polyphonic output channel, the channel's knob voltage is added to the input voltage to get the final output voltage.

### Offset knobs
There is one offset knob for each of the possible polyphonic channels. The default (initialize) value for all knobs always starts out at 0 volts. Of course the default can be overriden by the standard Venom parameter context menu option.

The module context menu has the following options to configure the behavior of the knobs:

#### Knob Range
Determines the minimum and maximum voltage of the knobs.
- **0-1 V**
- **0-2 V**
- **0-5 V**
- **0-10 V**
- **+/- 1 V**
- **+/- 2 V**
- **+/- 5 V**
- **+/- 10 V** (default)
- **Custom**

If **Custom** is chosen, then two menu options are added to set the custom range: **Custom min** for the full counter-clockwise level, and **Custom max** for the full clockwise level. The min and max values can range from -100 to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom levels. You are free to set the min greater than the max so the knobs work in reverse.

#### Quantize
Determines how values are quantized.
- **Off (continuous)** (default) - Neither the output nor the knob offset values are quantized.
- **Output to Integers (octaves)** - The sum of input voltage plus knob value is quantized to the nearest integral Volt.
- **Output to 1/12 V (semitones)** - The sum of input voltage plus knob value is quantized to the nearest 1/12 Volt.
- **Output to custom interval** - The sum of input voltage plus knob value is quantized to the nearest multiple of an interval that you specify.
- **Offset to Integers (octaves)** - The knob offset value (and display value) is quantized to the nearest integral Volt.
- **Offset to 1/12 V (semitones)** - The knob offset value (and display value) is quantized to the nearest 1/12 Volt.
- **Offset to custom interval** - The knob offset value (and display value) is quantized to the nearest multiple of an interval that you specify.

If either **custom interval** option is chosen, then a **Custom quantize interval** menu option is added to set the custom interval. The interval can be any positive value up to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom interval. If you enter a value less than or equal to zero, then the value will be transformed into 1.


#### Display unit
Determines how knob values are displayed and entered in knob context menu and hover text. Output values are always in Volts.
- **Volts (V)** (default)
- **Cents (¢)**

### Output polyphonic channel count
By default the number of output channels matches the number of input channels. Knobs for channels above the output count are ignored.

There is a "Polyphony channels" option in the module context menu that lets you override the default and select a specific output channel count. Input channels and knobs above the specified channel count are ignored. Monophonic inputs are cloned to match the selected channel count. If the input channel count is poly but less than the selected channel count, then missing channel inputs are assumed to be constant 0 volts, meaning the knob alone specifies the output voltage.

### Channel count display
The number of polyphonic channels at the output is displayed in the LED panel. The display will be yellow if the number of output channels is greater than or equal to the input channel count. The display will be red if the selected channel count is less than the input channel count.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Offset is bypassed then the input is passed unchanged to the output.
