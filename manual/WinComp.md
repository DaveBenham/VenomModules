A windowed polyphonic comparator inspired by the VCV Free COMPARE module, including the following enhancements:
- A tolerance factor to determine equivalency (the window)
- Options to rectify and/or invert signal outputs
- Gate output voltage options
- Additional gate outputs for A>=B and A<=B
- Oversampling options for audio applications
- An option to normal the B input to the previous A input sample so WinComp can function as a crude slope detector.
                                                                                                                   
### Polyphony
WINCOMP is fully polyphonic - the number of output channels is the maximum number of channels found across all three inputs.
Monophonic inputs are replicated to match the number of output channels. Polyphonic inputs that have fewer channels use 0V for missing channels.

### Inputs with Offsets
- **A** = A input, with OFFSET knob
- **B** = B input, with OFFSET knob
- **TOL** = Tolerance, with OFFSET knob. The tolerance specifies how close A must be to B in order to be considered equal.

Each input is summed with the corresponding OFFSET value. The OFFSETS are bipolar +/-10V. The resultant values are unconstrained.

The absolute value of TOL is used in all computations.
The tolerance affects all outputs except MIN and MAX.

### Signal Outputs

- **MIN** = Minimum output - the instantaneous minimum of the A and B inputs. TOL does not affect MIN.
- **MAX** = Maximum output - the instantaneous maximum of the A and B inputs. TOL does not affect MAX.
- **CLAMP** = A clamped to within B +/- TOL
- **OVER** = The overflow (positive or negative) from the CLAMP operation, computed as A - CLAMP.

Each of the signal output ports have context menu options to take the absolute value and or invert the output. The absolute value operation is performed prior to the inversion, so the output is guaranteed to be <=0V if both absolute value and invert are enabled.

A glowing green light in the lower left corner of the port indicates the output has the absolute value option enabled.
A glowing red light in the lower right corner indicates the output has the invert option enabled.

### Gate Outputs

- **A=B** = This gate is high if A is within B +/- TOL. OVER will be 0V when A=B is high. Computed as |A-B| <= |TOL|
- **A<>B** = This gate is high if A is not within B +/- TOL. OVER will be non-zero when A<>B is high. Computed as |A-B| > |TOL|
- **A<=B** = This gate is high if A is less than or equal to B +/- TOL. Computed as A <= B + |TOL|
- **A>=B** = This gate is high if A is greater than or equal to B +/- TOL. Computed as A >= B - |TOL|
- **A<B** = This gate is high if A is less than B +/- TOL. Computed as A < B - |TOL|
- **A>B** = This gate is high if A is greater than B +/- TOL. Computed as A > B + |TOL|

Each gate output has a small light in the lower right corner that glows yellow when the gate is high and the ouput is monophonic.
The light glows blue if the output is polyphonic and at least one channel is high.

The gate high and low values are 0V and 10V by default. The module context menu includes an option to specify any of the following alternate values
- 0,1
- +/-1
- 0,5
- +/-5
- 0,10
- +/-10

### Slope Detector mode
The context menu has a "B normalled to A -1 sample" option so WinComp functions as a crude slope detector. When enabled, the small LED between the A and B inputs glows blue.

If enabled and the B input is unpatched, then B will receive the previous sample from the A input. In this mode the following gates indicate the current slope of the A input.
- **A>B** - Positive (rising) slope
- **A<B** - Negative (falling) slope
- **A=B** - Zero (flat) slope

The slope detection works well with LFO inputs. But with audio inputs anti-aliasing can lead to noisy gates. Careful adjustment of the Tolerance may improve the quality of the slope detection.

### Oversampling
By default WINCOMP is configured to output CV values, without any anti-aliasing. But if producing audio output, then the output may have unacceptable aliasing artifacts. The context menu has an option to enable oversampling to greatly reduce aliasing in audio outputs. The oversampling applies to all the outputs, including gate outputs.

Oversampling uses significant CPU, so there are multiple options to choose from: x2, x4, x8, and x16. The higher the oversample rate, the better the result, but more CPU is used.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

An LED glows blue above the output ports if oversampling is enabled. The LED is black when oversampling is off.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are monophonic 0V if the module is bypassed.
