Adds three channels of stepped CV output for each division of the parent [Rhythm Explorer](RhythmExplorer.md). All three channels for a division are updated each time the division fires, and the randomly generated CV repeats in a pattern the same way as the Rhythm Explorer division gates. The Rhythm Explorer module supports one expander on either side, so each division can have as many as 6 channels of CV.

### Unlabled direction button
Determines which direction the expander looks for the parent Rhythm Explorer
- **Left** ***(yellow, default)***
- **Right** ***(orange)***

The upper corner LED on the chosen side will glow yellow if the connection is successful, or red if not successful.

### RNGn (Range for channel n) knob
Specifies the voltage range that the channel output can span. The value ranges from 1 to 10 volts, with 1 being the default.

### OFFn (Offset for channel n) knob
Specifies the minimum voltage for the channel output. The value ranges from -10 to 10 volts. The maximum output value will be the offset plus the range.

### RNDn (Random for channel n) input
Normally each channel will sample output voltages from the Rhythm Explorer internal random number generator. This is true even if the Rhythm Explorer RAND input has been patched.

The RNDn input lets you override the random number generator with your own voltage values. The input should have 8 polyphonic varying voltage channels so each division has its own voltage to choose from. If the input is monophonic than all divisions will be sampling from the same value. If the input is polyphonic with fewer than 8 channels than constant 0V will be used for all missing channels.

Typically the source is a polyphonic oscillator with its reset or hard sync input patched to the Rhythm Explorer Phrase Start output so that the oscillator provides repeatable patterns. Ideally each channel of the oscillator should be running at a different rate so each division gets a different voltage.

The RNG and OFF knobs are ignored if the RNDn input is patched, meaning the input value is used directly, without any scaling or offset.

### CVn (CV for channel n) outputs
Each channel has eight CV outputs, one for each division in the Rhythm Explorer. The outputs going top to bottom correspond with the Rhythm Explorer divisions going from left to right.

Every time a division fires, each channel output for that division is updated with a new voltage from the RNDn input if available, or else the Rhythm Explorer internal random number generator.

Note that division CV outputs are not updated if the division is muted on the Rhythm Explorer.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are monophonic 0V when the module is bypassed.
