Ten row polyphonic sample and hold combined with a shift register, with oversampling options.

Each row has its own polyphonic Trigger and Data inputs, and a polyphonic Hold output. In total that is 10 independent polyphonic sample and hold circuits. However, the inputs are normaled in a way that enables consecutive rows to function as a shift register.

If no input is provided, then values are sampled from an internal random number generator.

### TRIG (Trigger) button
Manually triggers the first row only

### OVER (Oversample) button
This color coded button controls how much oversampling is applied to minimize aliasing when triggering the sample & hold at audio rates. Oversampling is CPU expensive, so should only be applied when needed.
- **Off (gray - default)**
- **2x (yellow)**
- **4x (green)**
- **8x (light blue)**
- **16x (dark blue)**
- **32x (purple)**

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### RND (Random Range) button
This color coded button controls the output range of the internal random number generator
- **0-1 V (yellow)**
- **0-5 V (green)**
- **0-10 V (dark blue - default)**
- **+/- 1 V (pink)**
- **+/- 5 V (orange)**
- **+/- 10 V (purple)**

### CLR (Clear) button
Resets all polyphonic channels of all 10 Hold outputs to 0 V.

### Sample & Hold row

Each row functions as an independent sample and hold circuit.

#### TRIG (Trigger) input
The rising edge of a trigger input causes the row to sample and hold the current value at the Data input. The trigger is a Schmitt trigger that goes high above 2V and goes low below 0.1 V.

The Trig input is polyphonic - each polyphonic channel can be triggered independently.

For the first row only the sample can be triggered by the Trig input or the Trig button.

All TRIG inputs from the 2nd row onward are normaled to the TRIG input from the row above. So a trigger at row one can trigger all rows if none of the other rows are patched.

#### DATA input
This is the source that is sampled.

If the Trig input for the row is patched, then the Data input is normaled to the internal random number generator. Every row gets its own random value. Also each polyphonic channel gets its own random value.

If the Trig input is not patched, then the Data input is normaled to the Hold output from the row above. This is what enables the module to function as a shift register.

#### HOLD output
This output holds the last value that was sampled. Normally the value remains constant until the next trigger. However, when oversampling is enabled the value will wobble a bit for a few samples before stabilizing.

The number of polyphonic channels that are sampled and held at the output depends on the number of polyphonic channels found at the row inputs. The output polyphony count is the maximum count found between the Trig and Data inputs.

#### Performance optimization
If you are using oversampling and you do not require the Trigger button, then consider patching from the bottom and work your way up. For example, if you only need a 4 step shift register, then patch the trigger and data signals to the 7th row. If you patch the top row, then all 10 rows are triggered, and the module needs to do more work and consumes more CPU. The CPU usage can be dramatically different when oversampling is involved.

#### Polyphony behavior
The polyphonic channel count for each output is the maximum channel count found across both inputs from that same row. If an input is not patched than the polyphony count may be determined by a normalled value from the row above. Remember that the normalled Data input is dependent on whether the Trig input on that row is patched or not.

If the Trig input is monophonic, and the Data input is polyphonic, then all Data channels will be sampled simultaneously upon receipt of a trigger.

If the Trig input is polyphonic, and the Data input is monophonic, then each channel will sample the input when the channel receives a trigger. If normaled to the random number generator, then each channel will receive its own random value.

If both Trig and Input are polyphonic with the same number of channels, then each channel trigger will sample the appropriate data channel.

If both are polyphonic but the Data has fewer channels, then the missing data channels will be treated as constant 0 V.

If both are polyphonic but the Trig input has fewer channels, then the extra channels at the Data will never be sampled.

### Save Held Values context menu option
By default all held values are stored with the patch and restored upon patch load. This feature can be disabled via the "Save held values" module context menu option.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly S&H ASR is bypassed then all outputs are monophonic constant 0 V.
