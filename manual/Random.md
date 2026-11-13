A fully polyphonic implementation of the VCV RANDOM CV generator with independent shape controls for each of the 4 different random outputs, and a triggerable sample and hold function.

### Summary of differences from the VCV Random module
- All Venom Random inputs and outputs are polyphonic. The VCV module only supports polyphony at the random outputs and the External Data input.
- Added a context menu option to directly set the polyphony channel count, overriding the value computed from the inputs.
- The VCV External input has been renamed Data input for Venom.
- The Random spread control applies to external data input as well as internally generated random values. VCV ignores the Random spread when using external data.
- Each Venom random output has its own independent shape control. VCV uses a single shared Shape control for all random outputs.
  - The Stepped output shape control displays the actual parameter step count rather than a percentage
  - The Stepped output control defaults to the same non-linear scale as the VCV module, but there is a context menu option for a linear scale.
- Venom output triggers are either 1 msec or 1/2 a clock cycle, whichever is shorter. This greatly reduces the risk of high clock rates producing a constant high trigger state due to overlapping triggers.
- Oversampling is available to enable sampling at audio rates without introducing unwanted digital aliasing.

### Polyphony
All inputs and outputs are fully polyphonic.

By default the output polyphony count is the maximum channel count found across all inputs. The "Polyphony channels" context menu option lets you override the default channel count with a specific value between 1 and 16.

Monophonic inputs are replicated to match the output channel count.

Polyphonic inputs with fewer channels use constant 0V for the missing channels.

### Oversampling
Normally aliasing is not an issue when sampling at LFO rates, or when generating random CV at any rate. So by default Venom Random does not use oversampling. But if sampling regular waveforms at audio rates then aliasing can dramatically degrade the quality of the sampled output. An "Oversample" context menu option is available where you can select oversample rates of 2x, 4x, 8x, 16x, or 32x. This allows for clean audio outputs, even if sampling at high audio rates. Only the internal clock is oversampled, and the Trig and Data inputs are up-sampled to the oversample rate. The other inputs are ***not*** up-sampled. All of the outputs are bandlimited before being down-sampled to the native sample rate.

Oversampling is CPU intensive, so you should use the minimum oversample rate that gives good results. Typically oversample rates of 2x, 4x, or 8x are adequate. See [Anti-aliasing via oversampling](https://github.com/DaveBenham/VenomModules/blob/mainplugin.md#anti-aliasing-via-oversampling) for more information.

### RATE control
Controls the rate of the internal clock that drives the sample and hold functionality.

The Rate slider sets the base clock rate, and ranges from 0.002 Hz to 2000 Hz, with a default value of 2 Hz. The slider LED flashes yellow each time the clock triggers. The LED intensity is proportional to the percentage of polyphonic channels that are firing at that time.

The Rate CV attenuverter attenuates and or inverts the Rate CV input. The CV input is V/Oct when the attenuverter is at 100%.

The Rate control is ignored if an external clock is patched into the TRIG input. However, the Rate slider LED still flashes with each external clock trigger.

If using the internal clock and the Rate CV is either monophonic or unpatched, then a single clock is used and all sample & hold triggers are guaranteed to be in phase with each other. But if the Rate CV is polyphonic, then separate clocks are used that may be out of phase, even if all clocks are running at the same rate.

### PROB (Probability) control
Controls the probability that a clock trigger will trigger the sample and hold.

The Rate slider ranges from 0% to 100%, with the default at 100%. The slider LED flashes yellow each time the sample and hold is triggered. The LED intensity is proportional to the percentage of polyphonic channels that are firing at that time.

The Probability CV is additive with the slider value, and is scaled at 10% per volt. The attenuverter attenuates and/or inverts the CV before summing with the slider value.

The final effective probability is clamped to a value between 0 and 100%.

### RAND (Random spread) control
Controls how much variability there is in the random outputs by cross fading the current sampled value with the previous sampled value.

The Random slider ranges from 0% (outputs never change from the previous value) to 100% (the current sampled value is used without any dependency on the previous value). For intermediate values, the output moves closer to the previous value as the slider moves toward 0%.

The Random spread CV is additive with the slider value, and is scaled at 10% per volt. The attenuverter attenuates and/or inverts the CV before summing with the slider value.

The final effective Random spread is clamped to a value between 0 and 100%.

Unlike the VCV module, the Random spread is applied regardless whether sampling the internal white noise, or external Data input. If you want standard sample and hold functionality without Random spread control then make sure the Random spread is set to 100%.

### OFF (Internal noise Offset)
Controls whether the internal noise is bipolar or unipolar. This has no effect on external Data input.
- **Unlit** ***(default)*** - Bipolar -5V to 5V
- **Lit** - Unipolar 0V to 10V

### DATA input
This is the data that is sampled by the sample and hold. The voltage range for the Data input is unconstrained.

If the Data input is not patched then internal white noise is used instead.

### TRIG (clock Trigger) input
Overrides the internal clock with an external trigger source. The input is a Schmitt trigger that goes high at 2 V and goes low at 0.1 V.

The Rate control is ignored when using an external clock.

### TRIG (sample Trigger) output
Produces a 1 msec trigger every time the sample and hold is triggered to capture a new value. If the sampling rate is faster than 500 Hz (2 msec cycle) then the trigger length is shortened to 1/2 clock cycle to avoid overlapping triggers. However, triggers may still overlap if using an extremely fast internal clock, or an irregular external clock.

## *Random outputs*
Each of the random outputs has its own shape control that specifies how the output transitions from one sampled value to the next over time. All outputs behave the same when the shape is at the minimum value - the output immediately jumps to the new sampled value and holds until the next sample. But they all produce different outputs when not at the minimum shape value. In these cases the output moves toward the new sampled value over time. The output should reach the sampled value by the time the next sample is triggered. In order for this to work perfectly, Random must know the exact phase of each sample cycle. As long as you are using the internal clock, or if the external clock is triggering at a constant rate, then the phase computation is always correct, and the sampled value is always reached by the end of the sample cycle. But the phase computation will not be correct if using an irregular external clock, which will distort the appearance of the random output waveform.

### STEP (Step count) control and (Stepped) output
The output divides the transition into evenly spaced intervals, both from a time and voltage perspective.

The Step count slider specifies the number of steps, ranging from 1 to 16. The attenuated Step count CV is additive with the slider value, and the final effective step count is clamped to a value between 1 and 16.

By default the scale used for the slider and CV is non-linear, which can be awkward to work with when trying to dial in a specific step count using CV.

If the module "Linear step count scale" context menu option is enabled, then the slider and CV use a linear scale. Each 2/3 volt of CV (after attenuation) equates to 1 step when using the linear scale.

### LIN (Linear) shape control and output
The Linear output consists of straight line segments.

The slider specifies how long it takes to transition from the previous sampled value to the current one, ranging from 0% to 100% of a sample cycle. The output rises or falls linearly until it reaches the current sampled value, and then holds for the remainder of the cycle.

The CV is additive with the slider value and is scaled at 10% per volt. The CV can be attenuated and/or inverted by the attenuverter. The final effective shape is clamped to a value between 0 and 100%.

### EXP (Exponential) shape control and output
The Exponential output uses exponential curves when falling, and logarithmic curves when rising.

The default slider value of 100% equates to a linear transition that takes the full sample cycle. The minimum value of 0% equates to instantaneous steps. The intermediate values give the exponential/logarithmic curves.

The CV is additive with the slider value and is scaled at 10% per volt. The CV can be attenuated and/or inverted by the attenuverter. The final effective shape is clamped to a value between 0 and 100%.

### SMTH (Smooth) shape control and output
The smooth output uses sinusoidal curves to transition between sampled values.

The slider specifies what percentage of the sample cycle is used to transition, after which the output is held at the sampled level for the remainder of the cycle. The default slider value of 100% results in a continuous smooth curve through the entire sample cycle. The value of 0% results in an instantaneous step.

The CV is additive with the slider value and is scaled at 10% per volt. The CV can be attenuated and/or inverted by the attenuverter. The final effective shape is clamped to a value between 0 and 100%.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are constant monophonic 0V when Random is bypassed.
