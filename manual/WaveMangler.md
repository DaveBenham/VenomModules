A polyphonic distortion / waveshaper inspired by the [Doepfer A-136 Eurorack module](https://modulargrid.net/e/doepfer-a-136).

The Venom module implements most of the features of the Doepfer hardware, though not necessarily in exactly the same way, and then adds polyphony and additional modulation options.

### General operation
Wave Mangler is designed to operate on bipolar +/- 5V signal inputs. An input offset is available to transform unipolar input to bipolar, and another offset is available to transform bipolar output to unipolar.

High and Low thresholds divide the +/- 5V input range into three regions: High, Middle, and Low. Each region can have a different amplification applied to it and the resultant amplified output components are summed to a final output that can be radically different than the original input. There are many CV modulation options to make the output extremely dynamic. All inputs work equally well with both low frequency and audio rate signals.

### *I/O configuration section above the top horizontal line*

### IN DC BLOCK button
By default this button is disabled, and the main input is DC coupled.

If this button is enabled, then the main input is AC coupled, meaning a high pass filter is applied to eliminate DC bias from the input.

### OVER SMPL (oversample) button

Wave shaping can introduce undesired digital aliasing in the output. Aliasing can be mitigated via oversampling. The following oversampling options are available
- Off (gray - default)
- x2 (yellow)
- x4 (green)
- x8 (light blue)
- x16 (dark blue)
- x32 (purple)

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### OUT CLIP (output clipping) button

The final output may optionally be hard clipped, or soft tanh clipped. There are 4 options.
- Off (gray)
- Hard +/- 5V (yellow - default)
- Soft +/- 5V (light blue)
- Soft +/- 6V (dark blue)

### OUT DC BLOCK button
By default this button is disabled, and the main output is DC coupled, meaning the output may have a low frequency DC bias.

If this button is enabled, then the main output is AC coupled, meaning a high pass filter is applied to eliminate DC bias from the output.

### INPUT OFFSET knob and attenuated CV input
Applies an offset to the main input before any wave shaping takes place.

The knob ranges from -5 to +5 volts. The attenuated input is summed with the knob value to get the effective offset.

The primary function is to offset unipolar main inputs to bipolar signals. But interesting effects can be had by applying dynamic CV modulation.

### OUTPUT OFFSET knob and attenuated CV input
Applies an offset to the output after all wave shaping is complete (after output clipping, but before any output DC bias removal).

The knob ranges from -5 to +5 volts. The attenuated input is summed with the knob value to get the effective offset.

The primary function is to offset the bipolar main output to a unipolar signal.

### *Wave shaping section between the horizontal lines*

The wave shaping is accomplished by independently computing high, mid, and low output components that get summed to create the final output.

Each value related to wave shaping has a value knob plus a CV input with attenuator.

Threshold knobs are bipolar with values ranging from -5 to +5 volts.

Amplifier knob values are bipolar with values ranging from -10x to +10x amplification.

Attenuated CV is summed with the corresponding knob value to get the effective value. The summed value is free to exceed the range of the knob.

### HIGH THRESHOLD
Voltages above the High Threshold are considered the high region.

### LOW THRESHOLD
Voltages below the Low Threshold are considered the low region.

If the computed Low Threshold is above the computed High Threshold, then the values are internally swapped to ensure that the effective Low Threshold is never above the High Threshold.

### MID AMPLIFIER
Determines the amplification used for the middle output component. The main input is multiplied by the effective Mid Amplifier level to establish the middle output component.

The unlabeled small button determines whether the middle component is clipped at the high and low thresholds. It has 4 possible values:
- **Off** ***(dark gray)*** - No clipping is applied
- **Pre amp** ***(yellow, default)*** - The input is hard clipped at the high and low thresholds before applying the mid amplification.
- **Post amp** ***(blue)*** - The result of the mid amplifiction is hard clipped at the high and low thresholds.
- **Pre and post amp** ***(green)*** - The input is hard clipped at the high and low thresholds before applying the mid amplifcation, and then the result is hard clipped at the high and low thresholds.

If the Mid Amplifier is <= 1, then "Post amp" and "Pre and post amp" yield the same result.

If the Mid Amplifier is >= 1, then "Pre amp" and "Pre and post amp" yield the same result.

### HIGH AMPLIFIER
Determines the amplification used for the high output component.

If the input voltage is above the High Threshold, then the High Threshold is subtracted from the input before applying the high amplifier level to get the high output component.

If the input is not above the High Threshold, then the high output component is zero.

### LOW AMPLIFIER
Determines the amplification used for the low output component.

If the input voltage is below the Low Threshold, then the Low Threshold is subtracted from the input before applying the low amplifier level to get the low output component.

If the input is not below the Low Threshold, then the low output component is zero.

### *Main input and output section below the bottom horizontal line*

### IN (main wave) input

This is the input signal to be wave shaped

### OUT (wave shaped) output

This is the final wave shaped output that is the sum of the high, mid, and low output components.

### Polyphony
All inputs and the output are fully polyphonic.

The number of poly channels in the output is the maximum channel count found across all inputs.

Monophonic inputs are replicated to match the output channel count.

Polyphonic inputs with fewer channels than the output are assigned constant 0V for the missing channels.

### Order of operations
It can be difficult to decipher how the input is transformed into the final wave shaped output. Most users probably don't care and just twist knobs until the output is to their liking.

The summarized order of operations below is for those intrepid few that want to truly understand how the Wave Mangler works.

1) All inputs are upsampled with interpolation if oversamping is enabled
2) If In DC Block is enabled then the main Wave input is run through a high pass filter to remove DC bias
3) Input Offset is applied to the main Wave input
4) The mid output component is computed by multiplying the Wave input times the mid amplifier level. The mid component can optionally be clipped to the high and low thresholds before and/or after amplification.
5) If the Wave input is > the High Threshold, then the (Wave input minus the High Threshold) is multiplied by the High amplifier level, else the high output component is zero.
6) If the Wave input is < the Low Threshold, then the (Wave input minus the Low Threshold) is multiplied by the Low amplifier level, else the low ouptut component is zero.
7) The mid, high, and low output components are summed to get the final wave shaped output.
8) Any selected output clipping is applied to the wave shaped output
9) Output Offset is applied to the wave shaped output
10) If Out DC Block is enabled, then the wave shaped output is run through a high pass filter to remove DC bias from the output
11) If oversampling is enabled then the output is passed through a band limiting low pass filter to remove high frequency output that would be aliased after downsampling, and then the output is downsampled

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
If Wave Mangler is bypassed then all channels of the Wave input are passed through to the Wave output unchanged.
