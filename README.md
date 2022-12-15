# VenomModules
Venom VCV Rack Modules

## WINCOMP
A windowed comparator inspired by the VCV Fundamental COMPARE module, based on specs originally proposed at 
https://community.vcvrack.com/t/vcv-compare-gates-logic-and-process/17828/17?u=davevenom.

#### Polyphony
WINCOMP is fully polyphonic - the number of output channels is the maximum number of channels found across all three inputs.
Monophonic inputs are replicated to match the number of output channels. Polyphonic inputs that have fewer channels use 0V for missing channels.

#### Inputs with Offsets
- **A** = A input, with OFFSET knob
- **B** = B input, with OFFSET knob
- **TOL** = Tolerance, with OFFSET knob. The tolerance specifies how close A must be to B in order to be considered equal.
The absolute value of TOL is used in all computations.
The tolerance affects all outputs except MIN and MAX.

Each input is summed with the corresponding OFFSET value. The OFFSETS are bipolar +/-10V. The resultant values are unconstrained.

#### Outputs

- **MIN** = Minimum output - the instantaneous minimum of the A and B inputs. TOL does not affect MIN.
- **MAX** = Maximum output - the instantaneous maximum of the A and B inputs. TOL does not affect MAX.
- **CLAMP** = A clamped to within B +/- TOL
- **OVER** = The overflow (positive or negative) from the CLAMP operation, computed as A - CLAMP.
- **A==B** = This gate is high if A is within B +/- TOL. OVER will be 0V when A==B is high. Computed as |A-B| <= |TOL|
- **A<>B** = This gate is high if A is not within B +/- TOL. OVER will be non-zero when A<>B is high. Computed as |A-B| > |TOL|
- **A<=B** = This gate is high if A is less than or equal to B +/- TOL. Computed as A < B + |TOL|
- **A>=B** = This gate is high if A is greater than or equal to B +/- TOL. Computed as A > B - |TOL|
- **A<B** = This gate is high if A is less than B +/- TOL. Computed as A < B - |TOL|
- **A>B** = This gate is high if A is greater than B +/- TOL. Computed as A > B + |TOL|

Each gate output has a small light in the upper right corner that glows yellow when the gate is high and the ouput is monophonic.
The light glows blue if the output is polyphonic and at least one channel is high.

#### Context Menu
The context menu includes an option to specify the low and high gate values. The following options are available
- 0,1
- +/-1
- 0,5
- +/-5
- 0,10
- +/-10

The MIN, MAX, CLAMP, and OVER outputs each have independent options to take the absolute value and or invert the output.
The absolute value operation is performed prior to the inversion, so the output is guaranteed to be <=0V if both absolute value and invert are enabled.

A glowing green light in the upper left corner indicates the output has the absolute value option enabled.
A glowing red light in the upper right corner indicates the output has the invert option enabled.

#### Bypass
All outputs are 0V if the module is bypassed.
