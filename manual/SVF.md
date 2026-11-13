Polyphonic stereo state variable filter for audio and low frequency use. It provides simultaneous outputs for low pass, high pass, band pass, and notch modes. It also provides a morph output that crossfades between different filter modes.

The filter is highly resonant, yet will not self oscillate unless feedback is applied. This makes it an excellent choice for pinging.

There is also a Spread option to apply different cutoff frequencies to the left and right inputs. Because the right input is normalled to the left, and there is an option to subtract the right output from the left, the filter can function similarly to the Rob Hordijk Twin Peaks filter.

Every parameter has a primary labeled knob to set the base value, plus an unlabeled CV input with asscociated attenuverter. The Cutoff parameter has an additional V/Oct CV input. CV input is always summed with the parent base knob value.

Some parameters have one or two small buttons beside the label to configure additional aspects of the parameter.

All CV inputs can be driven at audio rates.

### CUTOFF

Controls the point where frequency amplitudes begin to be attenuated. By default this applies to both the left and right channels. But if Spread Direction is set to Right Absolute then this only applies to the left channel.

The Cutoff has an extra unattenuated volt per octave CV input. The attenuated CV input is also 1 volt per octave if the attenuverter is at 100%.

#### Slope (left) button
Controls the slope of the amplitude dropoff beyond the cutoff frequency, measured as dB per octave.
- 12dB (yellow, default)
- 24dB (orange)
- 36dB (red)
- 48dB (pink) 
- 60dB (purple)
- 72dB (green)
- 84dB (light blue)
- 96dB (dark blue)

#### Frequency range (right) button
Controls the range of the Cutoff knob (and the Spread knob if in Right Absolute mode)
- **Audio rate** ***(yellow, default)*** 16.352 Hz to 16744 Hz (or less), with default at C4
  - The knob maximum may be less than 16744 Hz depending on the VCV sample rate
  - Also by default applies a high pass filter to all outputs to eliminate DC offset, meaning the outputs become AC coupled
- **Low frequency** ***(orange)*** 0.125 Hz to 128 Hz, with default at 2 Hz.

#### Modulated cutoff frequency upper limits
The cutoff frequency can be modulated below and often times above the knob limits. However, digital implementations of state variable filters become unstable at high cutoff frequencies, thus imposing an upper limit to the cutoff frequency. Higher sample rates are capable of processing higher cutoff frequencies. The default audio range uses oversampling to increase the upper limit.

VCV Sample Rate | Audio Range Cutoff Limit | LFO Range Cutoff Limit
--|--|--
~11 kHz|3750 Hz|1250 Hz
12 kHz|4000 Hz|1250 Hz
~22 kHz|7500 Hz|2500 Hz
24 kHz|8000 Hz|2500 Hz
~44 kHz|15000 Hz|5000 Hz
48 kHz|16000 Hz|5000 Hz
~88 kHz|30000 Hz|10000 Hz
96 kHz +|32000 Hz|10000 Hz

### RES (Resonance)

Controls the amount of emphasis (amplification) applied to the cutoff frequency.

The Multimode Filter will never self oscillate unless band pass feedback is applied to the input. But with enough feedback and resonance applied, the oscillator will self oscillate at the cutoff frequency.

### GAIN

Controls how much the input is attenuated or amplified before processing by the filter.

The Gain knob ranges from 0 to 10. The Gain CV is scaled at 1 (100%) per volt.

The small Gain VCA Polarity button controls whether the VCA is unipolar or bipolar
- **Unipolar** ***(green - default)*** - The effective gain is clamped between 0V and 10V
- **Bipolar** ***(orange)*** - The effective gain is clamped between -10V and 10V, enabling ring modulation

Since all outputs are soft clipped at +/- 10V using tanh clipping, higher gains can be used to drive the output to saturation.

### SPREAD

Creates a difference between the Left and Right cutoff frequencies. With high resonance this can create formant sounds.

#### Spread Direction (left) button 
Determines how the spread is applied to the left and right cutoffs.
- **Bipolar** ***(orange, default)*** - Half the spread is added to the right cutoff, and half the spread is subtracted from the left cutoff.
- **Unipolar** ***(green)*** - The entire spread value is added to the right cutoff, and the left cutoff is unchanged.
- **Right Absolute** ***(blue)*** - The Cutoff knob and CV only applies to the left channel, and the Spread is transformed into the right cutoff.

In Bipolar and Unipolar modes the Spread knob ranges from -2 to 2 octaves.

In Right Absolute mode the Spread knob is configured the same as the Cutoff knob.

The Spread CV is always 1 volt per octave when the attenuator is at 100%. The CV can modulate the Spread beyond the knob limits.

#### Spread Mono Mode (right) button
Determines how the left and right outputs are merged into the left when the right output is unpatched.
- **Additive** ***(green, default)*** - The left and right are averaged (summed and divided by two)
- **Subtractive** ***(orange)*** - The right is subtracted from the left. This effectively converts the low and high pass outputs into band pass with two resonant peaks.

Note that Subtractive mono mode should only be used when the left and right inputs differ and/or the left and right cutoffs differ. Subtractive mode will effectively kill all mono output if the left and right inputs and cutoffs are identical.

This button has no effect if the right output is patched.

### FDBK (band pass feedback)

Makes the filter more resonant by internally feeding back a portion of the band pass output to the filter input. Note that the internal feedback is not affected by the Gain.

The filter will self oscillate with high feedback and resonance.

### MORPH

Cross-fades between different filter modes. 

#### Morph Mode button
Controls which filter modes are used for the cross-fade.
- **LP <-> BP** ***(red)*** - low pass to band pass
- **LP <-> BP <-> HP** ***(orange)*** - low pass to band pass to high pass
- **LP <-> HP** ***(green, default)*** - low pass to high pass
- **BP <-> HP** ***(blue)*** - band pass to high pass
- **BP <-> Notch** ***(purple)*** - band pass to notch
- **Dry <-> Wet LP** ***(pink)*** - raw input to low pass
- **Dry <-> Wet HP** ***(light blue)*** - raw input to high pass
- **Dry <-> Wet BP** ***(yellow)*** - raw input to band pass
- **Dry <-> Wet Notch** ***(white)*** - raw input to notch

The different filter modes effect signal phase differently. The phase relationship between the different filter modes varies depending on the selected filter slope. The differential phase shifts could lead to phase cancellation when cross-fading. To mitigate this, some filter modes are inverted in the morph cross fade, depending on the current slope setting.

Note that raw audio input may be slightly modified by the internal upsample/downsample process, as well as the output DC offset block (high pass filter).

### INPUT
The right input is normalled to the left input, meaning the right will receive the left input if the right is unpatched.

The small Input Coupling option button to the left can be used to eliminate DC offset from the input
- **DC** ***(off - default)*** - Input DC offset is preserved
- **AC** ***(yellow)*** - Input DC offset is removed.

AC coupling is useful for removing saturation asymmetry.

### Outputs

There is a left and right output for each filter mode. If the right output is unpatched, then the right output is added to or subtracted from the left output, depending on the Spread Mono Mode selection.

All outputs are soft clipped (tanh saturation) at +/- 10V.

The filter algorithm has a tendency to add a DC offset to audio outputs. To mitigate this, when using the audio range all outputs will remove DC offset via a high pass filter. There is a context menu option to disable the Audio DC block.

#### MORPH output

This is the result of the Morph cross fade.

#### LOW PASS output

Frequencies above the cutoff are attenuated.

#### HIGH PASS output

Frequencies below the cutoff are attenuated.

#### BAND PASS output

Frequencies above and below the cutoff are attenuated.

#### NOTCH output

Frequencies at or near the cutoff are attenuated.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The left and right inputs are replicated to all outputs when Multimode Filter is bypassed.
