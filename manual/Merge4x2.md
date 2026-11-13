Merge 4x2 is a compact and versatile polyphonic merge utility that can combine monophonic and/or polyphonic inputs into a polyphonic output.

There are two independent sections, each capable of merging up to four inputs into one output. If the top output is not patched, then the two sections are merged, thus allowing up to eight inputs to be merged into a single polyphonic output at the bottom output port.

Input ports should be patched starting at the top and working down within a section. Unused input ports below the last patched port are ignored. Unused ports above the last port are treated as a single channel of constant 0V.

At most 16 channels can be merged, starting at the top, and working down. A glowing red LED indicates a polyphonic input with channels that could not be merged.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Merge 4x2 is bypassed.
