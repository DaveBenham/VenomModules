Eight inputs in three dimensional space are cross faded to a single output. The inputs are placed at the vertices of a virtual cube. X, Y, and Z controls independently cross fade between inputs on opposite faces of the cube. Each fader functions linearly in amplitude, ranging from 0% to 100%. The orthogonal faders are multiplicative, such that when all three controls are at an extreme, then 100% of the output comes from a single input. When all three controls are at 50% then each input contributes 12.5% to the output. A final Level control can further attenuate the final output.

Omri Cohen has produced an introductory video for Cross Fade 3D as well as [Pan 3D](Pan3D.md):

https://www.youtube.com/watch?v=-kM_sOxuURI

### Polyphony
Every input and output is fully polyphonic. The output channel count is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the final output channel count. Polyphonic inputs with fewer channels use constant 0V for any missing channels.

### INPUTS
The faceplate has a perspective view of a cube with a polyphonic input at each of the eight vertices. The inputs are not individually labeled on the faceplate, but when you hover over an input a name is displayed that identifies whether the input is left or right, bottom or top, and front or back.

#### Single polyphonic input alternative
If only the Bottom Left Front input is patched, then Cross Fade 3D treats the input as an 8 channel polyphonic signal where each channel represents a monophonic input for one of the cube vertices. Missing channels are assigned constant 0V. The 8 channels are assigned as follows.
- 1 = Bottom left front
- 2 = Bottom right front
- 3 = Top left front
- 4 = Top right front
- 5 = Bottom left back
- 6 = Bottom right back
- 7 = Top left back
- 8 = Top right back

### X, Y, Z fader controls and CV inputs
X controls the left to right ratio, and is measured as percent right.  
Y controls the bottom to top ratio, and is measured as percent top.  
Z controls the front to back ratio, and is measured as percent back.  

Each fader control ranges from 0% to 100%, with the default 50% at noon.

Each dimension has a bipolar CV input and dedicated attenuator. The CV is scaled at 10% per volt by default. The CV is summed with the control value and clamped to a value between 0% and 100%. With a dimension at 50% and the CV attenuator at 100%, a +/- 5V bipolar can modulate a dimension from one extreme to the other.

A module context menu option is available to scale the CV at +/- 200% instead of +/- 100%. This enables a simple bipolar +/- 5V sine or triangle modulator to transition to one extreme and hold, before transitioning in the other direction.

If you prefer to work with spherical coordinates, then the [Sphere To XYZ module](SphereToXYZ.md) is available to convert r, theta, phi spherical coordinates into X, Y, Z cartesian coordinates. 

### MONO OUTPUT button
By default, polyphonic channels are preserved at the output. If the Mono Output button is enabled, then polyphonic output channels are summed to a monophonic output signal.

### LEVEL control
The final output level can be attenuated with the Level control. This may be especially useful when working with polyphonic inputs that are summed to a mono output.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The output is constant monophonic 0V if the module is bypassed.
