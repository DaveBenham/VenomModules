The Logic module provides up to 9 independent polyphonic logic gates that can be configured for any of the standard logic operations. All logic gates support one, two, or more inputs. A merge option allows polyphonic input channels to be used as inputs for the same logic gate. Compound logic operations can be created by reusing earlier outputs as inputs, without introducing sample delays. Options for oversampling, output range, and DC offset removal make Logic ideal for audio applications, with undesired aliasing limited by the oversampling.

### MERGE button
Controls how polyphonic inputs are handled.
- **Off (gray - default)**: Each polyphonic channel gets its own logic gate. The number of output channels is the maximum channel count found across all inputs for that gate. Monophonic inputs are replicated to match the output channel count. Polyphonic inputs with fewer channels assign constant 0V to the missing channels.
- **On (white)**: All logic gate outputs are monophonic. Polyphonic input channels are collected as separate inputs to a single monophonic gate.

### OVER (oversampling) button
Controls how much oversampling is applied to reduce aliasing when using the output as an audio signal. Oversampling can be CPU expensive, so should be off for normal CV usage. For audio applications, the least amount of oversampling should be used that gives satisfactory results.
- **Off (gray - default)**
- **2x (yellow)**
- **4x (green)**
- **8x (light blue)**
- **16x (dark blue)**
- **32x (purple)**

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### RANGE button
Controls the output voltages used for high and low states. Unipolar outputs are typically used for CV, and bipolar for audio.
- **0-1 (yellow)**: unipolar low = 0, high = 1
- **0-5 (green)**: unipolar low = 0, high = 5
- **0-10 (dark blue - default)**: unipolar low = 0, high = 10
- **+/- 1 (pink)**: bipolar low = -1, high = 1
- **+/- 5 (orange)**: bipolar low = -5, high = 5
- **+/- 10 (purple)**: bipolar low = -10, high = 10

### DC button
Controls whether DC offsets are removed from the outputs
- **Off (gray - default)**: Used for normal CV outputs
- **On (white)**: Useful for audio outputs

### HIGH THRESH and LOW THRESH knobs and inputs
Set the low and high thresholds for the Schmitt triggers that determine the state of each input. The effective threshold is the sum of the knob value and the corresponding input. The same thresholds are used for all inputs. An input goes high whenever the voltage rises above the high threshold. The input goes low whenever the voltage is at or below the low threshold. The state remains unchanged if the voltage lies between the thresholds.

The module automatically swaps the high and low thresholds if the high falls below the low, so the effective high threshold is always guaranteed to be greater than or equal to the low threshold.

The Low Threshold knob factory default is 0.1V, and the High Threshold knob default is 2V.

### Logic rows
There are nine rows, each consisting of two inputs, a Reuse button to recycle a previous output as an additional input, an Operation button to set the output logic, and an output for the logic result. The operation may be set to "defer" so that the row inputs are included as inputs to the row below, and the deferred row output is effectively disabled.

#### A and B inputs
Provide up to two inputs for each row. An input is ignored if it is not patched.

#### REUSE button
This button allows any of the logic outputs from rows above to be used as an additional input for the row, without introducing a sample delay. If set to None (three dashes), then the button is ignored. The first row doesn't have any row above, so its value is fixed at None.

If the output selected for reuse is deferred, then it is ignored as an input.

An output can also be used as input by patching an output from one row to input A or B from another row, except the patch cable will introduce a one sample delay.

#### OP (Operation) button
All operations other than defer have non-standard definitions so that they work with one, two, three, or more inputs. Each logic operation will operate in the standard way if there are exactly two inputs.
- **Defer (down arrow - default)**: All inputs from the row are included as inputs to the row below, and the output is unavailable for reuse. The output for the row will be constant 0V.
- **AND**: The output is high only if all inputs are high.
- **OR**: The output is high if at least one input is high.
- **XOR 1**: The output is high only if exactly one input is high and all others are low.
- **XOR ODD**: The output is high only if there are an odd number of high inputs.
- **NAND**: The output is high if at least one input is low.
- **NOR**: The output is high only if all inputs are low.
- **XNOR 1**: The output is high unless exactly one input is high.
- **XNOR ODD**: The output is high if an even number of outputs are high, or if all inputs are low

Note that AND, OR, XOR 1, and XOR ODD will all give the same output if there is only one input. A high input will produce a high output, and low input a low output.

The NAND, NOR, XNOR 1, and XNOR ODD will all function as a NOT operator if there is only one input. A high input will produce a low output, and a low input a high output.

#### OUT output
Produces the output for the selected logic operation. The output will be monophonic constant 0V if the operation is deferred, or if there are no inputs.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if LOGIC is bypassed.
