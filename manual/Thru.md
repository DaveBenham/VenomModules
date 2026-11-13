A simple utility module with 5 polyphonic input/output pairs suitable for unity mixing of stacked inputs, and/or introduction of sample delays.

### Unity Mixing
Nothing special here - just taking advantage of VCV Rack's built in capability to stack input cables. But it can be convenient if you need to distribute the unity mix to multiple destinations.

### Add Sample Delays
Each input starting with the 2nd is normalled to the output above, with a delay of 1 sample added. So an input at port 1 with an output at port 5 will yield 4 sample delays, plus the delay introduced by the module itself, for a total of 5.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
Each input is passed to the output below when the module is bypassed. However, the inputs are ***not*** normalled to the output above when bypassed.
