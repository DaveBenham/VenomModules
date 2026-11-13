Provides a level control for each channel of a polyphonic signal. For each polyphonic output channel, the channel's input voltage is scaled (attenuated and/or iniverted and/or amplified) based on the Level knob for that channel, and then sent to the output.

### Level knobs
The Level knobs set the scale factor for each polyphonic input channel. The default range for all Level knobs is unipolar 0-1x.

A "Level range" option in the module context menu lets you specify a different range that is used for all the knobs.
- 0-1x (default)
- 0-2x
- 0-5x
- 0-10x
- +/- 1x
- +/- 2x
- +/- 5x
- +/- 10x

The default (initialize) level for each knob always starts out at 1x, regardless of the range. Of course the default can be overriden by the standard Venom parameter context menu option. 

### Output polyphonic channel count
By default the number of output channels matches the number of input channels. Knobs for channels above the output count are ignored.

There is a "Polyphony channels" option in the module context menu that lets you override the default and select a specific output channel count. Input channels and knobs above the specified channel count are ignored. Monophonic inputs are cloned to match the selected channel count. If the input channel count is poly but less than the selected channel count, then missing channel inputs are assumed to be constant 0 volts, meaning the output will also be 0 volts.

### Channel count display
The number of polyphonic channels at the output is displayed in the LED panel. The display will be yellow if the number of output channels is greater than or equal to the input channel count. The display will be red if the selected channel count is less than the input channel count.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Scale is bypassed then the input is passed unchanged to the output.
