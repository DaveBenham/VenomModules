A dual windowed comparator combined with logic operations inspired by the [Joranalogue Compare 2 Eurorack module](https://joranalogue.com/collections/eurorack/products/compare-2). The Venom module implements all the features of the Joranalogue hardware, and then adds:
- polyphony, all inputs and outputs are fully polyphonic
- options for gate output voltage levels
- tripled the number of outputs
  - Joranalogue derives all outputs from whether an input is within the window
  - Venom adds outputs for when the input is greater than the window, and less than the window.
- oversampling options to mitigate aliasing introduced by the digital implementation (obviously not needed for the analog Joranalogue module)

### General Operation
There are two identical comparators, each with an input plus controls and CV inputs to define a voltage window based on window center (shift) and window size. Each comparator produces gates for when the input voltage is either within the window (=), above the window (>), or below the window (<), as well as their negated values. Logic is then applied to the paired =, >, and < gate outputs using AND, OR, and XOR operations. The XOR outputs are then used to drive three Flip Flop outputs.

### Use Cases
There is a [Joranalogue Compare 2 Practical User Guide](https://cdn.shopify.com/s/files/1/1594/2421/files/Compare_2_Practical_User_Guide_v1.5_300_dpi.pdf?v=1663597287) that shows a number of use cases for the Joranalogue hardware that also apply to Venom WinComp 2 + Logic. Of course the Venom module can do things that the Joranalogue module can't, but the practical guide is a good starting point.

### Inputs and Controls

#### SHIFT 1 and 2 knobs and CV inputs
Specifies the center of the comparator window. The knob value is summed with the CV input to establish the effective window center.

The Shift 2 CV input is normalled to the Shift 1 CV value.

#### SIZE 1 and 2 knobs and CV inputs
Specifies the size of the comparator window. The knob value is summed with the CV input to establish the effective window size.

The Size 2 CV input is normalled to the Size 1 CV value.

The window maximum is simply the Shift value plus 1/2 the Size value, and the minimum is the Shift value minus 1/2 the Size value.

#### IN 1 and 2 inputs

The IN inputs are the values that are compared against the comparator windows.

The IN 2 input is normalled to the IN 1 value.

#### RANGE (Output Range) square button

Specifies the low and high values for all gate outputs. The following unipolar and bipolar values are available:
- 0-1V
- 0-5V
- 0-10V (default)
- +/- 1V
- +/- 5V
- +/- 10V

#### OVER (Oversample) square button

Specifies the amount of oversampling used to mitigate aliasing that can be introduced by the digital processing. This is generally only useful if working with relatively high frequency audio inputs.
- Off (default)
- x2
- x4
- x8
- x16
- x32

There is also a module context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### Comparator Outputs

#### OUT 1 and 2 outputs
Each comparator has three OUT outputs
- \> - High when the input is above the window (> window max)
- = - High when the input is within the window (>= window min and <= window max)
- < - High when the input is below the window (< window min)

Note that if the effective window size is < 0, then the window max becomes lower than the window min, and = OUT can never be in a high state.

#### NOT 1 and 2 outputs
Each comparator has three NOT outputs
- \> - High when the input is not above the window (<= window max)
- = - High when the input is not within the window (> window max or < window min)
- < - High when the input is not below the window (>= window min)

### Logic Outputs

Each logic operation has three outputs

#### AND outputs
- \> - High when both > OUT1 and > OUT2 are high, low when either is low
- = - High when both = OUT1 and = OUT2 are high, low when either is low
- < - High when both < OUT1 and < OUT1 are high, low when either is low

#### OR outputs
- \> - High when either > OUT1 or > OUT2 is high, low when both are low
- = - High when either = OUT1 or = OUT2 is high, low when both are low
- < - High when either < OUT1 or < OUT2 is high, low when both are low

#### XOR outputs
- \> - High when > OUT1 and > OUT2 have different states, low when they are the same
- = - High when = OUT1 and = OUT2 have different states, low when they are the same
- < - High when < OUT1 and < OUT2 have different states, low when they are the same

#### FF (flip flop) outputs
- \> - Changes state upon the leading edge of each high > XOR gate
- = - Changes state upon the leading edge of each high = XOR gate
- < - Changes state upon the leading edge of each high < XOR gate

### Polyphony

All inputs and outputs are fully polyphonic. The number of output channels is the maximum number of input channels found across all inputs.

If an input is monophonic, then the single input channel is replicated to match the output channel count.

If an input has fewer channels then the outputs, then missing channels are assigned constant 0V.

### LED lights

Every gate output has an associated LED in the upper right corner that is off (dark gray) when the gate is low, and on (yellow) when the gate is high.

For polyphonic outputs, the default behavior is to set the LED brightness proportional to the percentage of channels in a high state.

There is a module context menu option to change which polyphonic channels are monitored.
- **Off** - all LEDS are permanently off (dark gray)
- **All** (default) - Each LED brightness is proportional to the percentage of channels in a high state.
- **Single channel 1 through 16** - Only the specified channel is monitored

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If WinComp 2 + Logic is bypassed then all outputs are constant monophonic 0V.
