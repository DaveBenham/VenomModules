Crossfade between channels of a polyphonic signal.

### Overview of basic functionality

A unipolar phasor from 0 to 10V drives the crossfade between the channels of a polyphonic input. The phasor can be the internal LFO, an external Phasor input, or the sum of both. The current phasor voltage determines which channel(s) are playing at that moment. An envelope controls how each channel fades in and out. For an input with N channels, the 10V phasor range is divided into N equal voltage ranges, one for each channel. That range represents 1 width unit. The width of the channel envelopes can vary, and is expressed in the channel width units. An envelope width of 1 means that the channel envelopes abut each other, but never overlap. Widths greater than 1 result in channel envelopes overlapping each other. Widths less than 1 result in gaps between channel envelopes. The shape of the channel envelopes is controlled by Hold, Skew, Rise shape, and Fall shape controls. There are outputs for the net phasor, the polyphonic channel gates, the polyphonic channel envelopes, the polyphonic final output, and the monophonic mix of final outputs. Level control and a VCA can be used to adjust the output volume and/or to apply amplitude modulation effects.

Poly Fade can run with slow LFO rates, or high audio rates, but there is no anti-aliasing applied.

Omri Cohen has created a tutorial video with patch ideas:

https://www.youtube.com/watch?v=xA_DA04hUlM

### Upper Section - Envelope and Level control

For each parameter in this section there are two knobs and one monophonic input. The left knob sets the base level, the input provides for CV modulation, and the right knob attenuates and/or inverts the CV. The attenuated CV is always added to the base knob value to establish the final value for the parameter.

Note that this section is labeled assuming that the phasor is going in a forward (ascending channel) direction. When phasing in reverse, the rise is actually the fall, and vice versa.

#### Width
Establishes the overall width of each channel's envelope, expressed as the number of channel slots to occupy. A value of 1 results in continuous sound, but without any overlap of channels. Less than 1 results in moments of silence between channels. Values greater than 1 result in channels overlapping. If the value matches the number of input channels, then each channel will play throughout the entire phasor cycle, but each channel envelope will still have a different phase.

The CV is scaled to 1 channel slot per 0.5V.

The net sum is clamped to a value between 0.0625 to 16. If the value exceeds the number of channels then it is internally truncated to match the channel count.

The default value is 2.

#### Hold
Specifies what percentage of the channel envelope is held at full volume, expressed as a percent.

A value of 100% yields a square wave, regardless what the Skew, Rise, and Fall values are. A value of 0% yields something between a saw and a symmetric triangle. Intermediate values yield some form of trapezoid.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 0%.

#### Skew
Specifies what percentage of the remaining envelope width is dedicated to the rise, with the remainder going to the fall. If the Hold value is 0%, then a 50% skew yields a symmetric triangle with equal rise and fall ramps.

A value of 0% yields a saw wave ramp down with an immediate rise, and a sloped fall.

A value of 100% yields a saw wave ramp up with a sloped rise, and immediate fall.

Again, the concept of rise and fall assumes the phasor is moving in a forward direction.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 50%.

#### Rise and Fall shapes
Specifies the amount of curve applied to the rise and fall ramps using J curves. The values range between -100% and 100%, with 0% being linear. Negative values are similar to exponential, and positive logarithmic, though the math is different.

Both CV inputs are scaled at 10% per volt.

The net sums are clamped to a value between -100% and 100%

The default values are 0% (linear).

#### Level

Specifies the maximum level of the channel and sum outputs, with value ranging from 0% to 100%.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 100%.

### Indicator LED lights

The top row of dimly lit small green LEDs indicate which input channels are used. The bright green indicates the starting channel(phase 0). Off LEDs indicate excluded channels.

The bottom row of larger yellow LEDs indicate which channels are currently producing output, with the intensity proportional to the current level of the envelope.

### Lower Section - Phasor control, channel selection, and outputs

#### Rate knob and input
Controls the frequency of the internal LFO phasor. The knob ranges from 0.0079842 Hz to 32.703 Hz (C1). The default is 2.044 Hz.

Note that the ping pong (triangle LFO) rate is actually half the displayed frequency so as to maintain a constant rate of channel switching relative to forward and backward modes. 

The 1V/Oct CV input can drive the rate both lower and higher. The minimum rate is 0.0015 Hz (11.1 minutes per cycle). The maximum rate is 12 kHz, however there is no anti-aliasing. So mid to high audio rates can lead to harsh results with lots of aliasing.

#### Direction button and monophonic input

Controls the shape of the internal phasor, and thus the direction of the fade.

Patched CV input actually sets the value of the button, so any CV is effectively disabled if the button is locked.

There are four possible values
- Forward (right arrow, default) = a rising ramp saw wave = 0V CV
- Backward (left arrow) = a falling ramp saw wave = 1V CV
- Ping Pong (bidirectional arrow) = a triangle wave = 2V CV
- Off (dashed line) = disables (freezes) the internal phasor = 3V CV

CV values are rounded to the closest valid value.

If the direction is Off then by default the internal phasor is reset to 0. The module has a context menu option to disable the "Reset if direction off" so that you can freeze the internal phasor at its current position and then later pick up where you left off.

#### Chan (Channel count) knob and monophonic input

Controls how many of the polyphonic channels participate in the fade. The value can range from 0 to 16.

A value of 0 represents Automatic, meaning all channels appearing at the In input are used.

If the set value is less than the number of input channels, then the unused channels are ignored.

If the set value exceeds the number of input channels, then the missing channels default to constant 0V.

Patched CV input actually sets the value of the knob, so any CV is effectively disabled if the knob is locked. 

CV is scaled at 0.5V per channel, and values are rounded to the nearest valid multiple of 0.5.

#### Start knob and monophonic input

Controls which input channel represents phase 0. It can be any value between 1 and 16.

Patched CV input actually set the value of the knob, so any CV is effectively disabled if the knob is locked.

CV is scaled at 0.5V per channel, with 0.5 representing 1. Values are rounded to the nearest valid multiple of 0.5.

#### Phasor monophonic input

The phasor input is summed with the internal LFO phasor to establish the effective phasor that controls the fade. Bipolar signals can be used. A value of 0 represents phase 0, and the phase increases linearly as the voltage increases until a value of 10V loops back to phase 0. The phase is cyclical, so values above 10V or below 0V effectively wrap around.

#### Phasor input slew rate button (unlabled)
A small color coded button below and to the right of the Phasor input controls slew that can be applied to the phasor input before it is summed with the internal LFO. Slew can be helpful for reducing audio pops created by sudden changes within the incoming phasor signal.
- **Off** (gray - default)
- **3 msec/V** (yellow)
- **6 msec/V** (orange)
- **10 msec/V** (purple)

#### Reset monphonic input

The rising edge of a trigger at this input instantly resets the internal LFO phasor back to phase 0.

#### In polyphonic input

This input represents the polyphonic channels that are crossfaded.

#### Sum monophonic output

This is the sum of the polyphonic crossfaded channel outputs.

#### Phasor monophonic output

This is the effective phasor - the sum (unity mix) of the internal LFO phasor and the phasor input. This is guaranteed to have a unipolar output between 0V and 10V.

#### Gates polyphonic output

This port outputs a high 10V gate for each channel that currently has a non-zero envelope.

#### Envs (envelopes) polyphonic output

This port outputs the envelope for each of the channels.

#### Out polyphonic output

This port outputs the crossfaded outputs for each of the channels.

### Polyphony Rules

#### Polyphonic Input
The effective number of input channels is the greatest of the following values:
- The number of poly channels at the input
- The selected number of crossfaded channels
- The selected start channel.

If the actual input is monophonic, then the input is replicated to match the effective input channel count.

If the actual input is polyphonic with fewer channels than the effective input channel count, then missing channels are assigned constant 0V.

#### Polyphonic Outputs
By default the number of output channels is minimized to match the selected number of crossfaded channels. In this case the start channel is always assigned to output channel 1, and there are no unused channels in the output.

A module context menu option is available to disable the output channel minimization. In this case the effective input channels map directly to the output channels. Unused input channels become constant 0V in the output.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0 volts when Poly Fade is bypassed.
