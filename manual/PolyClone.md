Poly Clone replicates each channel from a polyphonic input and merges the result into a single polyphonic output. It is especially useful with the Recurse modules when using polyphonic inputs. Poly Clone provides a convenient way to replicate channels in polyphonnic CV inputs to match the recursion count.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](AuxClone.md).

### CLONE knob
Selects the number of times to clone or replicate each input channel. Possible values range from 1 to 16.

### POLY input
Each channel from the polyphonic input is replicated based on the Clone count as long as the total replicated channel count does not exceed 16. Channels that cannot be replicated the full amount are ignored.

No input is treated as monophonic constant 0V.

For each channel appearing at the input, the corresponding LED above glows yellow if the channel could be successfully replicated, and red if it could not be replicated. LEDs beyond the input channel count remain off (black).

### GRP (Output grouping) button
The GRP button controls how the cloned channels will be grouped at the output
- **Input channel** ***(yellow, default)*** - All the cloned channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be cloned at the output.

### POLY output
All of the replicated channels are merged into the single polyphonic output. The order of the channels is dependent on the GRP button.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Clone is bypassed then the output is constant monophonic 0 volts.
