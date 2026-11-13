This expander module adds additional cloned poly input/output pairs to [Clone Merge](CloneMerge.md), [Poly Clone](PolyClone.md), or [Poly Unison](PolyUnison.md).

The expander must be placed immediately to the right of a Clone Merge, Poly Merge, or Poly Unison. The yellow LED in the upper left indicates whether the expander has successfully connected to a parent module.

Each set of polyphonic input channels is cloned to match the clone count of the parent module, and sent to the output. The number of polyphonic channels at the input should either match the number of input channels at the parent, or else 1. If the input is unpatched it is treated as a mono input with a single chanel at constant 0 volts.

The GRP button above and to the right of each input port controls how the cloned channels will be grouped at the corresponding output
- **Input channel** ***(yellow, default)*** - All the cloned channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be cloned at the output.

The number of polyphonic channels at each output will always match the number of poly output channels at the parent. The LED to the right of each output indicates whether the output was able to properly clone all input channels.

If the input poly count matches the parent, then each of the input channels is cloned as per the parent, and the LED is yellow.

If the input poly count is 1, then the input is replicated to match the parent input channel count, and then each of those channels is cloned. The LED is yellow.

If the input poly count is less than the parent and greater than 1, then the missing channels are treated as constant 0V, and the LED is orange.

If the input poly count is greater than the parent, then excess channels are ignored, and the LED is red.

All outputs will be constant 0V and all port LEDS will be black under any of the following conditions:
- The expander is not connected to a parent.
- The expander is bypassed.
- The parent module is bypassed.

The names of each input/output pair are linked. Changing the name of one will automatically change the name of the other.
