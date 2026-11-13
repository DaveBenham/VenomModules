Compact polyphonic bipolar VCA (ring modulator) and mixer inspired by Mutable Instruments Blinds.

### General operation - Blinds emulation

By default this module behaves the same as the Audible Instruments Quad VC Polarizer, which in turn emulates the [Mutable Instruments Blinds hardware](https://modulargrid.net/e/mutable-instruments-blinds). All the functionality has been shrunk down to 5hp, very similar to the Southpole Bandana module that was never officially ported to VCV 2.

There are 4 independent module channels, each with an Input, Output, Level attenuverter, and Level CV with Level Amount attenuverter.

The formula for the output is ***V<sub>out</sub> = V<sub>in</sub> x NetLevel%***, where ***NetLevel% = (Level% + V<sub>CV</sub>/V<sub>unity</sub> x CV%)***, clamped to +/- 200%

The input port is normalled to 5V, and the unity voltage is 5V. The way the math works, you can attenuate an input signal and add an offset by patching the input to the Level CV input, and leaving the Input port unpatched. The Level knob becomes the offset, and the CV Level knob the attenuator.

Each output is normalled to the output below so you can mix the outputs.

The Venom Quad VC Polarizer extends the Blinds functionality with support for polyphony, plus a number of options.

### Polyphony

For each output port, the number of polyphony channels is the maximum channel count found across all inputs that contribute to the output. So if all inputs are monophonic except for 5 channel polyphony at the first CV input, and the first output is unpatched, and the second output is patched, then the output will be 5 channel polyphony.

The polyphony acts as would be expected when the input and output channel counts match.

Monophonic input is automatically replicated to match the output channel count.

Polyphonic inputs with fewer channels assume constant 0V for any missing channels.

### O (Oversample) button
Sets the level of oversamping to apply to all inputs and outputs. Oversampling can be useful for controlling aliasing that would otherwise be introduced by ring modulation and/or clipping.
- **Off** (dark gray, default)
- **x2** (yellow)
- **x4** (green)
- **x8** (light blue)
- **x16** (dark blue)
- **x32** (purple)

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### N (Normal input value) button
Sets the input voltage if the port is not patched
- **5V** (yellow, default)
- **10V** (light blue)

### V (VCA CV) button
Determines the type of CV input that is accepted, where unity is 5V or 10V. The unipolar mode effectively creates more typical 2 quadrant VCAs instead of the default 4 quadrant VCAs (ring modulators)
- **Unipolar clamped** (green) = 0V to unity
- **Bipolar clamped** (orange) = -unity to unity
- **Bipolar unlimited** (purple, default)

### U (Unity) button
Determines the CV voltage that represents 100%
- **5V** (yellow, default)
- **10V** (light blue)

### C (Clippping) button
Optional clipping applied to the output
- **Off** (dark gray, default)
- **Hard +/- 10V** (white)
- **Hard +/- 5V** (yellow)
- **Soft +/- 12V** (light blue) = tanh saturation
- **Soft +/- 6V** (dark blue) = tanh saturation

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V when Quad VC Polarizer is bypassed.
