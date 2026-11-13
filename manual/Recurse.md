Uses polyphony to recursively process an input via SEND and RETURN up to 16 times. Polyphonic inputs may be used, which will limit the number of recursion passes to less than 16 for each input channel. There are no limits placed on any of the input or output voltages.

### Recursion Count knob and display

Specifies how many recursion passes should be applied to each input channel. The input channel count mulitiplied by the recursion count must be less than or equal to 16, else some input channels will be dropped.

The display is yellow if all input channels can recurse the requested number of times. The display is red if one or more input channels is dropped.

The Recursion Count may range between 0 and 16. A count of 0 gives the same result as bypassing the module - all input channels are passed directly to the output and the Send output is monophonic 0 volts.

If the "Allow 0 recursion" option in the module context menu is disabled then the minimum Recursion Count becomes 1. Patches created prior to Venom version 2.18 automatically disable the 0 recursion in order to preserve old behavior.

### Input and Output

The Input is the signal to be recursively processed, and the result is sent to the Output. The number of output channels will match the input unless channels had to be dropped due to channel count limitations during send and return.

### Send and Return

The Send output and Return input channel count can be computed as input channels multiplied by recursion count. The maximum number of input channels is the integral division of 16 divided by recursion count. Input channels that exceed the computed maximum are dropped.

If there are 3 input channels and a recursion count of 5, then Send/Return channels 1-5 will be for input channel 1, 6-10 for input channel 2, and 11-15 for input channel 3. If the recursion count is bumped up to 6 while the input channel count remains 3, then Send/Return channels 1-6 are assigned to channel 1, 7-12 channel 2, and channel 3 is dropped because there are not enough channels remaining to complete 6 recursive passes.

The Return input is normalled to the Send output.

### Internal Modulation

Inputs can be recursively scaled and offset within the RECURSE module itself.

The SCALE input and knob value are multiplied to establish the scale factor. The scale factor is then multiplied by the input value, so RECURSE can perform ring modulation.

The OFFSET input and knob value are added to establish the offset that is added to the input.

By default, the SCALE operation occurs before the OFFSET operation. A context menu option lets you choose to peform the OFFSET before SCALE. A small light glows yellow next to the operation that is performed first.

The SCALE and OFFSET inputs support polyphony. However, only channels that correspond to what appears on the IN input will be used, extra channels will be ignored. Each SCALE or OFFSET channel will be applied to all relevant recursive steps for the corresponding IN input.

The unlabeled Modulation Mode knob determines when the SCALE and OFFSET operations take place. There are 4 values:
- **1Pre** = Once before the first Send only
- **nPre** = Before every recursive Send
- **nPost** = After every recursive Return
- **1Post** = Once after the final Return

Since the Return is normalled to the Send, it is possible to generate a polyphonic series of constant voltages using only the RECURSE module. For example, leave all inputs and the Return unpatched, set the Recursion Count to 16, the Scale to 1, the Offset to 1V, and the Mode to nPre. The SEND output will have 16 channels of integral values from 1 to 16. Change the Mode to nPost and the values will range from 0 to 15.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The Input is passed unchanged to the Output when RECURSE is bypassed. The SEND will be monophonic 0V.
