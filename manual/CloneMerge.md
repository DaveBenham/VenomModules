Clone Merge clones up to 8 monophonic inputs and merges the resultant channels into a single polyphonic output. It is especially useful with [Recurse](Recurse.md) when using polyphonic inputs. Clone Merge provides a convenient way to replicate CV inputs to match the recursion count.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](AuxClone.md).

### CLONE knob
Selects the number of times to clone or replicate each input. Possible values range from 1 to 16.

### MONO inputs
The 8 monophonic inputs should be populated from top to bottom. Each input is replicated based on the Clone count as long as the total channel count across all replicated inputs does not exceed 16. Inputs that cannot be replicated the full amount are ignored.

An LED glows yellow for each input that is successfully replicated. The LED glows red if the input cannot be replicated. Unpatched inputs below the last patched input are ignored and the corresponding LED is off (black).

### GRP (Output grouping) button
The GRP controls how the cloned inputs will be grouped at the corresponding output
- **Input channel** ***(yellow, default)*** - All the clones for a given input will be grouped together at the output.
- **Input set** ***(blue)*** - The inputs will be grouped together in order as a set, and then the set will be cloned at the output.

### POLY output
All of the replicated inputs are merged into the single polyphonic output. The order of the channels is dependent on the GRP button.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Clone Merge is bypassed then the output is constant monophonic 0V.
