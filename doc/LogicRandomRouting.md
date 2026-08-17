# Logic, Random, and/or Routing
- [BERNOULLI SWITCH](#bernoulli-switch)
- [BERNOULLI SWITCH EXPANDER](#bernoulli-switch-expander)
- [LOGIC](#logic)
- [PAN 3D](#pan-3d)
- [POLY SAMPLE & HOLD ANALOG SHIFT REGISTER](#poly-sample--hold-analog-shift-register)
- [WINCOMP](#wincomp)
- [WINCOMP 2 + LOGIC](#wincomp-2--logic)

[Venom top](/README.md#venom)

## BERNOULLI SWITCH
![Bernoulli Switch module image](BernoulliSwitch.png)  
The Bernoulli Switch stochastically routes two inputs to two outputs.

### General Operation
Upon receiving a trigger or gate, a virtual coin toss determines if input A goes to output A and B to B (no-swap), or if A goes to B and B to A (swap). Each input can be attenuated and/or inverted by a bipolar SCALE knob ranging from -1 to 1, and offset by an OFFSET knob, ranging from -10 to 10. The A input is normalled to the TRIG input and the B input is normalled to 0V, so if both inputs are left unpatched, the Bernoulli Switch will function as a "traditional" Bernoulli Gate. A "latched" mode may be achieved by leaving the B input at 0V and setting the A input SCALE to 0 and the A OFFSET to 10V.

### Probability
The PROB knob and PROB input determine the probability that a particular routing operation will occur. If there is no PROB input, then a fully counterclockwise PROB knob yields a 0% chance of the the routing operation, and fully clockwise is 100% chance. The range is linear, with 50% at noon. The probability can be modulated by bipolar PROB input, with each volt equating to 10% chance.

### Routing Operation or Mode
The actual routing operation is controlled by the unlabeled 3 position sliding switch, with the following possible values:
- **TOGGLE**: The probability is the chance that the routing will toggle from the current routing to the opposite. If currently swapped, then a positive result will switch to un-swapped. If currently unswapped, then a positive result will switch to swapped. A negative result yields no change.
- **SWAP**: The probabiliy is the chance that the routing will be set to swapped, regardless of the current routing. A positive result yields a swapped routing, a negative result yields a no-swap routing. 
- **GATE**: The routing is always in a swap configuration whenever the TRIG input is low. Upon transition to high, a positive coin toss results in a no-swap routing throughout the TRIG high state. A negative result remains in a swap configuration.

### Triggers
The module generally responds to a leading edge transition from low to high of the TRIG input or the manual TRIG button. The TRIG button works by adding 10V to the TRIG input.

Bernoulli Switch uses Schmitt trigger logic to determine when a trigger starts and stops. The RISE knob sets the threshold for a trigger transition to high, and the FALL knob sets the threshold for a transition to low. By default the RISE is set to 1V, and the FALL to 0.1V. If currently low, then a TRIG input >= the RISE threshold transistions to HIGH. The input remains high until the input falls below the FALL threshold, upon which it returns to a low state.

If the RISE threshold is less than the FALL threshold, then the roles are reversed, and the Bernoulli Switch is triggered by a trailing transition from high to low. If using GATE mode, the routing will always have a swap configuration whenever the input is high, and the configuration may switch to no-swap upon transition to low.

The TRIG button is not guaranteed to always trigger a coin toss - it depends on how the RISE and FALL are configured, as well as the current TRIG input value.

### Normal value button
The small color coded button next to the Trig input determines exactly what value is normalled to the A input.
 - **Raw trigger input - red (default)**: The trigger input is passed unmodified
 - **Schmitt trigger result - blue**: The Schmitt trigger gate is sent instead of the raw input. A low state is always 0V. Normally a high state is 10V. But the "high" gate will be -10V if the rise threshold is below the fall threshold.

### Polyphony
Bernoulli Switch is fully polyphonic. There are two modes available from the context menu that determine how many virtual coin tosses are performed based on the number of channels on each input:
- **TRIG and PROB only** (default)

  The number of coin flips is the maximum channel count found across the TRIG and PROB inputs. If one of the inputs is monophonic, and the other polyphonic, then the monophonic input is replicated to match the channel count of the polyphonic input. If a polyphonic input is missing channels, then the missing channels are treated as 0V.

  If the coin flip count is 1 (both TRIG and PROB are mono), then polyphonic inputs to A and/or B are treated as a whole - all of the channels on the A input are directed to either the A or B output. The same for the B input. If either A or B input has fewer channels than the other, then the missing channels are made up with 0V so that A and B always send the same number of channels.

  But if either of TRIG or PROB are poly, resulting in multiple coin flips, then each coin flip is applied to the appropriate channels in A and B inputs. This can result in A and B input channels being scrambled across the A and B outputs. Monophonic A and/or B inputs are replicated to match the coin flip channel count. Missing channels in polyphonic A and/or B are treated as 0V. Extra input channels in A or B are ignored.

- **All inputs**

  The number of coin flips is the maximum channel count found across all four inputs - TRIG, PROB, A, and B. Monophonic inputs are replicated to match the maximum channel count. Polyphonic channels with missing channels treat the missing channel as 0V.

  Polphonic inputs in A and/or B can always be scrambled across A and B outputs.

A small LED between the INPUT ports glows yellow when polyphony detection is configured for "All inputs". The LED is off (black) when configured for "TRIG and PROB only".

### Active Route Monitoring
A pair of yellow lights indicate the current routing configuration. A yellow light glowing to the left of the PROB knob indicates a no-swap configuration. A glowing yellow light to the right indicates a swap configuration.

The yellow lights only monitor a single channel - by default they monitor channel one. The context menu has a Monitor Channel option to switch to a different channel. If the monitored channel is Off, or greater than the number of coin flip channels, then the yellow lights will remain dark - no monitoring will be done.

### Audio Processing
By default Bernoulli Switch is configured for switching gates or CV signals, but it can also process audio signals. If you switch audio at slow rates you may get unwanted pops. If you switch audio at audio rates then you may get unwanted aliasing. The module context menu has Audio Process options to reduce or eliminate these artifacts: Antipop crossfade for slow switching, and various oversampling options for audio rate switching. A small LED between the OUTPUT ports glows red when Anti-Pop Switching is in effect, and blue when any of the oversampling options is enabled. The LED is off (black) when the Audio Process is set to Off (the default).

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md#anti-aliasing-via-oversampling) for more information.

### Factory Presets
The following factory presets are available that emulate the four configurations available to the Mutable Instruments Branches module:
- Bernoulli Gate - Sends 10V Schmitt trigger gate to A or B
- Latched Bernoulli Gate - Sends constant 10V to either A or B
- Latched Toggled Bernoulli Gate - Toggles constant 10V between A and B
- Toggled Bernoulli Gate - Toggles 10V Schmitt trigger gate between A and B

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass Behavior
If Bernoulli Switch is bypassed then the A input is passed unchanged to the A output, and likewise the B input to the B output. The B input is still normalled to the A input while bypassed.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)

## BERNOULLI SWITCH EXPANDER
![Bernoulli Switch Expander image](BernoulliSwitchExpander.png)  
Adds CV inputs with attenuverters for all of the Bernoulli Switch parameters, all of which can be driven at audio rates.

The expander must be placed to the right of the Bernoulli Switch. The LED near the top glows yellow when a successful connection is made.

All the expander inputs are additive with their associated Bernoulli Switch parameters, with one exception. The mode input supersedes the mode parameter when patched. 
- Toggle mode <= -1V
- Swap mode >-1V and <1V
- Gate mode >= 1V

An attenuator for the Bernoulli Switch Probability CV was added rather than an attenuator for the expander mode CV.

All expander CV inputs are monophonic, with the signal applied equally to any polyphony found at the Bernoulli Switch.

None of the expander inputs are oversampled, even if the Bernoulli Switch has oversampling enabled.

All expander inputs as well as the probability CV attenuator are ignored when the expander is bypassed.

[Watch this video](https://www.youtube.com/watch?v=VRERXi6RuPE) to see how the Bernoulli Switch coupled with the Bernoulli Switch Expander can transform a simple sine wave into a dynamic complex stereo sound.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)


## LOGIC
![Logic module image](Logic.png)  

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

See [Anti-aliasing via oversampling](/README.md#anti-aliasing-via-oversampling) for more information.

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
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if LOGIC is bypassed.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)


## PAN 3D
![Pan 3D module image](Pan3D.png)  

A single input is panned across eight outputs in three dimensional space. The output ports are placed at the vertices of a virtual cube. X, Y, and Z panner controls indepently pan the input between outputs on opposite faces of the cube. The panners function linearly in amplitude, ranging from 0% to 100%. The three orthogonal panners are multiplicative. When all three controls are at an extreme, 100% of the input is panned to a single output. When all three controls are at 50%, then each output receives 12.5% of the input. A final Level control can further attenuate the final outputs.

### Polyphony
Every input and output is fully polyphonic. The output channel count is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the final output channel count. Polyphonic inputs with fewer channels use constant 0V for any missing channels.

### OUTPUTS
The faceplate has a perspective view of a cube with an output at each of the eight vertices. The outputs are not individually labeled on the faceplate, but when you hover over an output a name is displayed that identifies whether the output is left or right, bottom or top, and front or back.

### X, Y, Z panner controls and CV inputs
X controls the left to right ratio, and is measured as percent right.  
Y controls the bottom to top ratio, and is measured as percent top.  
Z controls the front to back ratio, and is measured as percent back.  

Each panner control ranges from 0% to 100%, with the default 50% at noon.

Each dimension has a bipolar CV input and dedicated attenuator. The CV is scaled at 10% per volt by default. The CV is summed with the control value and clamped to a value between 0% and 100%. With a dimension at 50% and the CV attenuator at 100%, a +/- 5V bipolar can modulate a dimension from one extreme to the other.

A module context menu option is available to scale the CV at +/- 200% instead of +/- 100%. This enables a simple bipolar +/- 5V sine or triangle modulator to transition to one extreme and hold, before transitioning in the other direction.

If you prefer to work with spherical coordinates, then the [Sphere To XYZ module](Modulation.md#sphere-to-xyz) is available to convert r, theta, phi spherical coordinates into X, Y, Z cartesian coordinates. 

### MONO OUTPUT button
By default, polyphonic channels are preserved at the outputs. If the Mono Output button is enabled, then polyphonic output channels are summed to a monophonic output signal.

### LEVEL control
The final output levels can be attenuated with the Level control. Each output is attenuated the same amount. This control may be especially useful when working with polyphonic outputs that are summed to a mono output.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V if Pan 3D is bypassed.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)


## POLY SAMPLE & HOLD ANALOG SHIFT REGISTER
![Poly Sample & Hold Analog Shift Register module image](PolySHASR.png)  
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

See [Anti-aliasing via oversampling](/README.md#anti-aliasing-via-oversampling) for more information.

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
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly S&H ASR is bypassed then all outputs are monophonic constant 0 V.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)

## WINCOMP
![WINCOMP module image](WinComp.PNG)  
A windowed polyphonic comparator inspired by the VCV Free COMPARE module, including the following enhancements:
- A tolerance factor to determine equivalency (the window)
- Options to rectify and/or invert signal outputs
- Gate output voltage options
- Additional gate outputs for A>=B and A<=B
- Oversampling options for audio applications
- An option to normal the B input to the previous A input sample so WinComp can function as a crude slope detector.
                                                                                                                   
### Polyphony
WINCOMP is fully polyphonic - the number of output channels is the maximum number of channels found across all three inputs.
Monophonic inputs are replicated to match the number of output channels. Polyphonic inputs that have fewer channels use 0V for missing channels.

### Inputs with Offsets
- **A** = A input, with OFFSET knob
- **B** = B input, with OFFSET knob
- **TOL** = Tolerance, with OFFSET knob. The tolerance specifies how close A must be to B in order to be considered equal.

Each input is summed with the corresponding OFFSET value. The OFFSETS are bipolar +/-10V. The resultant values are unconstrained.

The absolute value of TOL is used in all computations.
The tolerance affects all outputs except MIN and MAX.

### Signal Outputs

- **MIN** = Minimum output - the instantaneous minimum of the A and B inputs. TOL does not affect MIN.
- **MAX** = Maximum output - the instantaneous maximum of the A and B inputs. TOL does not affect MAX.
- **CLAMP** = A clamped to within B +/- TOL
- **OVER** = The overflow (positive or negative) from the CLAMP operation, computed as A - CLAMP.

Each of the signal output ports have context menu options to take the absolute value and or invert the output. The absolute value operation is performed prior to the inversion, so the output is guaranteed to be <=0V if both absolute value and invert are enabled.

A glowing green light in the lower left corner of the port indicates the output has the absolute value option enabled.
A glowing red light in the lower right corner indicates the output has the invert option enabled.

### Gate Outputs

- **A=B** = This gate is high if A is within B +/- TOL. OVER will be 0V when A=B is high. Computed as |A-B| <= |TOL|
- **A<>B** = This gate is high if A is not within B +/- TOL. OVER will be non-zero when A<>B is high. Computed as |A-B| > |TOL|
- **A<=B** = This gate is high if A is less than or equal to B +/- TOL. Computed as A <= B + |TOL|
- **A>=B** = This gate is high if A is greater than or equal to B +/- TOL. Computed as A >= B - |TOL|
- **A<B** = This gate is high if A is less than B +/- TOL. Computed as A < B - |TOL|
- **A>B** = This gate is high if A is greater than B +/- TOL. Computed as A > B + |TOL|

Each gate output has a small light in the lower right corner that glows yellow when the gate is high and the ouput is monophonic.
The light glows blue if the output is polyphonic and at least one channel is high.

The gate high and low values are 0V and 10V by default. The module context menu includes an option to specify any of the following alternate values
- 0,1
- +/-1
- 0,5
- +/-5
- 0,10
- +/-10

### Slope Detector mode
The context menu has a "B normalled to A -1 sample" option so WinComp functions as a crude slope detector. When enabled, the small LED between the A and B inputs glows blue.

If enabled and the B input is unpatched, then B will receive the previous sample from the A input. In this mode the following gates indicate the current slope of the A input.
- **A>B** - Positive (rising) slope
- **A<B** - Negative (falling) slope
- **A=B** - Zero (flat) slope

The slope detection works well with LFO inputs. But with audio inputs anti-aliasing can lead to noisy gates. Careful adjustment of the Tolerance may improve the quality of the slope detection.

### Oversampling
By default WINCOMP is configured to output CV values, without any anti-aliasing. But if producing audio output, then the output may have unacceptable aliasing artifacts. The context menu has an option to enable oversampling to greatly reduce aliasing in audio outputs. The oversampling applies to all the outputs, including gate outputs.

Oversampling uses significant CPU, so there are multiple options to choose from: x2, x4, x8, and x16. The higher the oversample rate, the better the result, but more CPU is used.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md#anti-aliasing-via-oversampling) for more information.

An LED glows blue above the output ports if oversampling is enabled. The LED is black when oversampling is off.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are monophonic 0V if the module is bypassed.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)


## WINCOMP 2 + LOGIC
![Compare 2 module image](Compare2.png)  
A dual windowed comparator combined with logic operations inspired by the [Joranalogue Compare 2 Eurorack module](https://joranalogue.com/collections/eurorack/products/compare-2). The Venom module implements all the features of the Joranalogue hardware, and then adds:
- polyphony, all inputs and outputs are fully polyphonic
- options for gate output voltage levels
- tripled the number of outputs
  - Joranalogue derives all outputs from whether an input is within the window
  - Venom adds outputs for when the input is greater than the window, and less than the window.
- oversampling options to mitigate aliasing introduced by the digital implementation (obviously not needed for the analog Joranalogue module)

### General Operation
There are two identical comparators, each with an input plus controls and CV inputs to define a voltage window based on window center (shift) and window size. Each comparator produces gates for when the input voltage is either within the window (=), above the window (>), or below the window (<), as well as their negated values. Logic is then applied to the paired =, >, and < gate outputs using AND, OR, and XOR operations. The XOR outputs are then used to drive three Flip Flop outputs.

### Use Cases
There is a [Joranalogue Compare 2 Practical User Guide](https://cdn.shopify.com/s/files/1/1594/2421/files/Compare_2_Practical_User_Guide_v1.5_300_dpi.pdf?v=1663597287) that shows a number of use cases for the Joranalogue hardware that also apply to Venom WinComp 2 + Logic. Of course the Venom module can do things that the Joranalogue module can't, but the practical guide is a good starting point.

### Inputs and Controls

#### SHIFT 1 and 2 knobs and CV inputs
Specifies the center of the comparator window. The knob value is summed with the CV input to establish the effective window center.

The Shift 2 CV input is normalled to the Shift 1 CV value.

#### SIZE 1 and 2 knobs and CV inputs
Specifies the size of the comparator window. The knob value is summed with the CV input to establish the effective window size.

The Size 2 CV input is normalled to the Size 1 CV value.

The window maximum is simply the Shift value plus 1/2 the Size value, and the minimum is the Shift value minus 1/2 the Size value.

#### IN 1 and 2 inputs

The IN inputs are the values that are compared against the comparator windows.

The IN 2 input is normalled to the IN 1 value.

#### RANGE (Output Range) square button

Specifies the low and high values for all gate outputs. The following unipolar and bipolar values are available:
- 0-1V
- 0-5V
- 0-10V (default)
- +/- 1V
- +/- 5V
- +/- 10V

#### OVER (Oversample) square button

Specifies the amount of oversampling used to mitigate aliasing that can be introduced by the digital processing. This is generally only useful if working with relatively high frequency audio inputs.
- Off (default)
- x2
- x4
- x8
- x16
- x32

There is also a module context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md#anti-aliasing-via-oversampling) for more information.

### Comparator Outputs

#### OUT 1 and 2 outputs
Each comparator has three OUT outputs
- \> - High when the input is above the window (> window max)
- = - High when the input is within the window (>= window min and <= window max)
- < - High when the input is below the window (< window min)

Note that if the effective window size is < 0, then the window max becomes lower than the window min, and = OUT can never be in a high state.

#### NOT 1 and 2 outputs
Each comparator has three NOT outputs
- \> - High when the input is not above the window (<= window max)
- = - High when the input is not within the window (> window max or < window min)
- < - High when the input is not below the window (>= window min)

### Logic Outputs

Each logic operation has three outputs

#### AND outputs
- \> - High when both > OUT1 and > OUT2 are high, low when either is low
- = - High when both = OUT1 and = OUT2 are high, low when either is low
- < - High when both < OUT1 and < OUT1 are high, low when either is low

#### OR outputs
- \> - High when either > OUT1 or > OUT2 is high, low when both are low
- = - High when either = OUT1 or = OUT2 is high, low when both are low
- < - High when either < OUT1 or < OUT2 is high, low when both are low

#### XOR outputs
- \> - High when > OUT1 and > OUT2 have different states, low when they are the same
- = - High when = OUT1 and = OUT2 have different states, low when they are the same
- < - High when < OUT1 and < OUT2 have different states, low when they are the same

#### FF (flip flop) outputs
- \> - Changes state upon the leading edge of each high > XOR gate
- = - Changes state upon the leading edge of each high = XOR gate
- < - Changes state upon the leading edge of each high < XOR gate

### Polyphony

All inputs and outputs are fully polyphonic. The number of output channels is the maximum number of input channels found across all inputs.

If an input is monophonic, then the single input channel is replicated to match the output channel count.

If an input has fewer channels then the outputs, then missing channels are assigned constant 0V.

### LED lights

Every gate output has an associated LED in the upper right corner that is off (dark gray) when the gate is low, and on (yellow) when the gate is high.

For polyphonic outputs, the default behavior is to set the LED brightness proportional to the percentage of channels in a high state.

There is a module context menu option to change which polyphonic channels are monitored.
- **Off** - all LEDS are permanently off (dark gray)
- **All** (default) - Each LED brightness is proportional to the percentage of channels in a high state.
- **Single channel 1 through 16** - Only the specified channel is monitored

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If WinComp 2 + Logic is bypassed then all outputs are constant monophonic 0V.

[Logic, Random & Routing top](#logic-random-andor-routing)|[Venom top](/README.md#venom)


