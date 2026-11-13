Merge Split is a compact and versatile polyphonic utility that can merge up to 4 monophonic or polyphonic inputs into one polyphonic output, and also split a single polyphonic input into any combination of monophonic and polyphonic outputs.

The top merge section can merge up to 16 channels from the four inputs. A glowing red LED next to an input port indicates a polyphonic input that has channels that could not be merged.

If the polyphonic input in the bottom split section contains the same number of channels found across all inputs in the merge section, then each split output polyphonic channel count will match that of the corresponding merge input. The LED between the merge and split sections will glow yellow to indicate a successful resplit.

If the number of channels at the split input does not match the merge input channel count, then by default each split output will be monophonic.

You can override the channel count for each split output via a port context menu. Setting any split output port to a specific channel count will disable automatic resplitting.

There is a module context menu option to restore resplit mode for all four split outputs.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Merge Split is bypassed.
