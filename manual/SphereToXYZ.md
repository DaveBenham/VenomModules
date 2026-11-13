Converts spherical coordinates r, theta, phi into cartesian coordinates X, Y, Z using standard physics definitions:

> x = r x sin(theta) x cos(phi)  
> y = r x sin(theta) x sin(phi)  
> z = r x cos(theta)  

When applied to audio rate inputs, it has an effect similar to ring modulation and can lead to some interesting sound design.

https://www.youtube.com/watch?v=EF-rrpFtR00

### Polyphony
All inputs and outputs are fully polyphonic. The number of output channels is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the output channel count. Polyponic inputs with fewer channels use constant 0V for any missing channels.

### OverSample button
This color coded switch establishes the amount of oversampling used to mitigate audio aliasing that may be introduced by the conversion process.

- dark gray (default) = Off - No oversampling
- yellow = x2 oversampling
- green = x4 oversampling

Oversampling is typically not needed for most conversions. But it may be useful with high frequency audio outputs.

Oversampling uses significant CPU resources, so it is best to use the minimum oversampling value that gives the desired output.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### r input
This represents the radial distance r. Negative r values are accepted.

### Theta input
This represents the polar angle theta. It is scaled at 36V/degree, meaning 5V = 180 degrees. Any angle is allowed.

### Phi input
This represents the azimuthul angle phi. It is scaled at 36V/degree, meaning 5V = 180 degrees. Any angle is allowed.

### Scale switch
Specifies the scale factor used for converting spherical radial distances into cartesian distances.
- 1:1 = +/-5V radial distance range specifies a 10V diameter sphere centered about the origin that is inscribed within a 10V x 10V x 10V cube.
- sqrt(3):1 = +/-5V radial distance range specifies a ~17.32V diameter sphere centered about the origin with a 10V x 10V x 10V cube inscribed within it.

Assuming that inputs are bipolar +/-5V, then a 1:1 ratio guarantees that all converted X, Y, and Z outputs are within +/-5V. However, not all possible +/-5V X, Y, Z coordinates are covered.

The sqrt(3):1 ratio guarantees that a bipolar +/-5V radial distance can cover all possible +/-5V X, Y, Z coordinates. However, some converted values may exceed the +/-5V range, depending on the input angles.

### X output
This represents the x cartesian coordinate, after conversion.

### Y output
This represents the y cartesian coordinate, after conversion.

### Z output
This represents the z cartesian coordinate, after conversion.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are constant monophonic 0V when Sphere To XYZ is bypassed.
