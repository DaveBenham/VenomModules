Split one or more poly inputs into multiple mono or poly outputs.

This split utility is extremely flexible, with many configurations possible. Each polyphonic input can be split into any combination of monophonic and polyphonic outputs. The module has default Automatic behavior for distributing the input channels to the outputs. But the default behavior can be overridden by defining a specific channel count for one or more output ports via output port context menus. The hover tooltip for each output port includes information on the current channel configuration.

The thick red lines indicate which input port is split to which set of output ports. The groupings are defined by which input ports are patched. Unpatched input ports are ignored. The module is automatically reconfigured each time you patch or unpatch an input port. Each patched input distributes its channels to the same output row downward until it reaches another row with a patched input. If no input is patched, then the module treats it as if a monophonic 0V input is patched to the top input.

Multi Split attempts to distribute the input channels evenly to the output ports. When the input channels cannot be divided evenly, higher (lower numbered) output ports take precedence over lower (higher numbered) ports. However, any output channel that is configured for a specific channel count will always output that number of channels. So if you subtract the sum of the fixed output counts from the input count, then the remainder are divided amongst the output ports that are configured for Auto assignment. If the distributor runs out of input channels, then constant 0V is used for the remainder of the output channels.

If the input channels cannot fit within the output channels, then the LED next to the input port glows red. This can only happen if all output ports in the group are configured for a specific channel count, and the sum of the channel counts is less than the input channel count.

This all probably sounds confusing. But once you start patching, it will probably start making sense quickly. Even if you cannot figure out the automatic distrubution algorithm, you can always take control by assigning a specific channel count to each output port.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Multi Split is bypassed.
