Merge one or more sets of mono and/or poly inputs into polyphonic outputs.

This merge utility is extremely flexible, with many configurations possible. It can merge a set of monophonic inputs into one polyphonic output. It can also merge a set of polyphonic inputs into one polyphonic ouput. Or it can merge a mixture of mono and poly inputs into one polyphonic output. Finally, there can be multiple sets of merges, each with its own poly output.

The thick red lines indicate which input ports are merged and sent to which output port. The groupings are defined by which output ports are patched. Unpatched output ports are ignored. The module is automatically reconfigured each time you patch or unpatch an output port. Each patched output merges the inputs from that row and above until it reaches another row with a patched output. If there are no output ports, then the module assumes the last output port will be patched.

The number of polyphonic output channels cannot exceed 16. If the sum of polyphony counts across the inputs exceeds 16, then excess channels are dropped, and the LEDs next to input ports with dropped channels glow red.

Each input port has a context menu to explicitly set the number of input channels, regardless how many are actually there. In addition there is a module context menu option to set the input channel count for all inputs. By default each input port channel count is set to Auto. The hover tooltip for each input port includes information on the current channel configuration. Mono inputs are replicated to match the specified input channel count. If the specified count is greater than the actual number of input channels, then extra channels are assigned constant 0V. If the specified count is less than the actual input channel count, then the dropped channels LED for that port glows red.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Multi Merge is bypassed.
