Use your mouse as a controller, without having to point at any specific module. Activate the Mouse Pad output by pressing and holding some combination of Shift, Ctrl, and/or Alt (Option on Mac). A 10V gate is generated while active, and there are two indpendent CV outputs that track the mouse movement. The horizontal axis controls the X output, and vertical the Y.

### Activate section

The top four buttons configure how the Mouse Pad is activated. At least one of Shift, Ctrl, and/or Alt must be enabled to use Mouse Pad. Whichever buttons are activated, that specifies what key combination is required to track movement and produce output. The Mouse Pad stops tracking motion when at least one of the keys is released.

The Tog (toggle) button changes behavior. The first combination press activates, and it remains active after release. The next combination press deactivates tracking.

### X / Y sections

Both X and Y have 4 controls to configure the output.

The Scale knobs amplify, attenuate and/or invert the mouse sensitivity. At 1 both the X and Y have a 10V range.

The Origin knobs determine what part of the VCV Rack window corresponds to 0V for X and Y. Values of 0%, 0% correspond to the lower left corner of the window. Values of 100%, 100% correspond to the the upper right corner. So a value of 50%, 50% corresponds to the center of the window.

The Absolute buttons control whether the output is absolute, or relative to the mouse position at the moment of activation. If enabled then the Origin knobs establish the origin. If not enabled then the Origin knobs are ignored.

The Return buttons control whether the outputs return to 0 upon release of the activation keys, or if the last value is held until the next activation.

### Outputs

All outputs are monophonic.

Gate is high (10V) whenever the mouse is tracking, and low (0V) when not tracking.

X output follows the horizontal motion of the mouse, and Y the vertical motion. Both values have a 10V range when Scale is at 100%

### Multiple Mouse Pads

Multiple Mouse Pads can be effectively used if each is given a different combination of activation keys. This allows you to use a single mouse to send user controlled CV to different parts of your patch on demand.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Mouse Pad is bypassed.
