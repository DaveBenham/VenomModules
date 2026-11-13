Provides five independently configurable constant voltage knobs. 

### Knob configuration
Each knob has custom menu options to tailor the knob to your needs.

#### Knob Range
Determines the minimum and maximum voltage of the knob.
- **0-1 V**
- **0-2 V**
- **0-5 V**
- **0-10 V**
- **+/- 1 V**
- **+/- 2 V**
- **+/- 5 V**
- **+/- 10 V** (default)
- **Custom**

If **Custom** is chosen, then two menu options are added to set the custom range: **Custom min** for the full counter-clockwise level, and **Custom max** for the full clockwise level. The min and max values can range from -100 to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom levels. You are free to set the min greater than the max so the knob works in reverse.

#### Quantize
Determines how knob values are quantized.
- **Off (continuous)** (default)
- **Integers (octaves)**
- **1/12 V (semitones)**
- **Custom interval**

If **Custom interval** is chosen, then a **Custom quantize interval** menu option is added to set the custom interval. The interval can be any positive value up to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom interval. If you enter a value less than or equal to zero, then the value will be transformed into 1.

#### Display unit
Determines how knob values are displayed and entered in knob context menu and hover text. Output values are always in Volts.
- **Volts (V)** (default)
- **Cents (¢)**

#### Polyphony channels
Determines the number of polyphonic channels to output. All channels will have the same voltage. The default is 1 (mono).

 
### Global Knob configuration
The module context menu includes options to configure all knobs simultaneously. The option values are the same as for individual knobs except custom values are not supported. You must configure each knob individually if you want custom values.

If all knobs currently share the same value, then the current value is displayed in the menu. If at least one knob is different, then the current value is empty.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus. However, the rename function is modified slightly. Renameing a knob will automatically rename the corresponding output port, and vice versa.

### Bypass

All outputs are constant monophonic 0 V when KNOB 5 is bypassed.
