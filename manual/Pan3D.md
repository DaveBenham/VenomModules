A single input is panned across eight outputs in three dimensional space. The output ports are placed at the vertices of a virtual cube. X, Y, and Z panner controls indepently pan the input between outputs on opposite faces of the cube. The panners function linearly in amplitude, ranging from 0% to 100%. The three orthogonal panners are multiplicative. When all three controls are at an extreme, 100% of the input is panned to a single output. When all three controls are at 50%, then each output receives 12.5% of the input. A final Level control can further attenuate the final outputs.

Omri Cohen has produced an introductory video for Pan 3D as well as [Cross Fade 3D](CrossFade3D.md):

https://www.youtube.com/watch?v=-kM_sOxuURI

### Polyphony
Every input and output is fully polyphonic. The output channel count is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the final output channel count. Polyphonic inputs with fewer channels use constant 0V for any missing channels.

### OUTPUTS
The faceplate has a perspective view of a cube with an output at each of the eight vertices. The outputs are not individually labeled on the faceplate, but when you hover over an output a name is displayed that identifies whether the output is left or right, bottom or top, and front or back.

### X, Y, Z panner controls and CV inputs
X controls the left to right ratio, and is measured as percent right.  
Y controls the bottom to top ratio, and is measured as percent top.  
Z controls the front to back ratio, and is measured as percent back.  

Each panner control ranges from 0% to 100%, with the default 50% at noon.

Each dimension has a bipolar CV input and dedicated attenuator. The CV is scaled at 10% per volt by default. The CV is summed with the control value and clamped to a value between 0% and 100%. With a dimension at 50% and the CV attenuator at 100%, a +/- 5V bipolar can modulate a dimension from one extreme to the other.

A module context menu option is available to scale the CV at +/- 200% instead of +/- 100%. This enables a simple bipolar +/- 5V sine or triangle modulator to transition to one extreme and hold, before transitioning in the other direction.

If you prefer to work with spherical coordinates, then the [Sphere To XYZ module](SphereToXYZ.md) is available to convert r, theta, phi spherical coordinates into X, Y, Z cartesian coordinates. 

### MONO OUTPUT button
By default, polyphonic channels are preserved at the outputs. If the Mono Output button is enabled, then polyphonic output channels are summed to a monophonic output signal.

### LEVEL control
The final output levels can be attenuated with the Level control. Each output is attenuated the same amount. This control may be especially useful when working with polyphonic outputs that are summed to a mono output.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V if Pan 3D is bypassed.
