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

### Expander module
The module context menu includes an option to add the [Bernoulli Switch Expander](BernoulliSwitchExpander.md) which adds additional CV inputs with attenuverters.

### Active Route Monitoring
A pair of yellow lights indicate the current routing configuration. A yellow light glowing to the left of the PROB knob indicates a no-swap configuration. A glowing yellow light to the right indicates a swap configuration.

The yellow lights only monitor a single channel - by default they monitor channel one. The context menu has a Monitor Channel option to switch to a different channel. If the monitored channel is Off, or greater than the number of coin flip channels, then the yellow lights will remain dark - no monitoring will be done.

### Audio Processing
By default Bernoulli Switch is configured for switching gates or CV signals, but it can also process audio signals. If you switch audio at slow rates you may get unwanted pops. If you switch audio at audio rates then you may get unwanted aliasing. The module context menu has Audio Process options to reduce or eliminate these artifacts: Antipop crossfade for slow switching, and various oversampling options for audio rate switching. A small LED between the OUTPUT ports glows red when Anti-Pop Switching is in effect, and blue when any of the oversampling options is enabled. The LED is off (black) when the Audio Process is set to Off (the default).

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### Factory Presets
The following factory presets are available that emulate the four configurations available to the Mutable Instruments Branches module:
- Bernoulli Gate - Sends 10V Schmitt trigger gate to A or B
- Latched Bernoulli Gate - Sends constant 10V to either A or B
- Latched Toggled Bernoulli Gate - Toggles constant 10V between A and B
- Toggled Bernoulli Gate - Toggles 10V Schmitt trigger gate between A and B

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass Behavior
If Bernoulli Switch is bypassed then the A input is passed unchanged to the A output, and likewise the B input to the B output. The B input is still normalled to the A input while bypassed.
