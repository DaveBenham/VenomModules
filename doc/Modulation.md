# Modulation

## MULTIMODE FILTER
![Multimode Filter module image](doc/SVF.png)  
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
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The left and right inputs are replicated to all outputs when Multimode Filter is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## OCTAVER
![Octaver module image](doc/Octaver.png)  
An old school octave effect in the style of the Pearl OC-7 and Boss OC-2 analog effect pedals. Feed in a monophonic signal with a regular periodic waveform and it can generate a mix consisting of the original input plus 1 octave up, 1 octave down, and 2 octaves down. Just like the old pedals, an input mix of multiple pitches will lead to problems with tracking. 

### Polyphony
Unlike the old hardware pedals, this module can work with chords using VCV style polyphony as long as there is only one pitch per polyphonic channel.

The number of output channels is strictly controled by the main IN input.

Monophonic CV inputs are replicated to match the IN channel count.

Polyphonic CV inputs with fewer channels use 0V for the missing channels. Any extra channels are ignored.

### MODE (Sub-octave mode) button
Controls the method used to generate sub-octaves. Each method has its own distinctive sound.
- **Inversion (Pearl)** ***(yellow, default)*** - the method used by the Pearl OC-7
- **Square (Boss)** ***(blue)*** - the method used by the Boss OC-2

### OVER (Oversample) button
Controls the amount of oversampling for controlling digital aliasing.
- **x2** ***(yellow, default)***
- **x4** ***(green)***
- **x8** ***(blue)***

The default x2 oversampling is adequate for almost all applications. If working with very high frequencies then x4 or x8 might be appropriate.

### Mix controls
Each mix component has a large knob plus a CV input with small attenuverter knob. The large knob sets the base amount ranging from 0% to 100%. The bipolar CV is scaled at 10% per volt, and is inverted and/or attenuated by the small attenuverter knob. The final effective amount is the sum of the scaled CV plus the base knob, clamped to a value between 0% and 100%.

#### +1 (Octave +1 mix) knob and CV input
Controls the amount of one octave up in the final mix.

The upper octave is produced by fully rectifying the input and then amplifying the result by a factor of two. The result is then converted back to a bipolar signal. This works well with sine and triangle waveforms. For saw waveforms it produces a triangle output in the same octave, and square waveforms hardly produce any output at all.

#### 0 (Dry mix) knob and CV input
Controls the amount of the original input in the final mix.

#### -1 (Octave -1 mix) knob and CV input
Controls the amount of one octave down in the final mix.

The method for producing the -1 octave depends on the current mode.
- **Inversion** - A comparator determines the zero crossing points to establish unit wave cycles. Alternating cycles are inverted to create the sub-octave.
- **Square** - A comparator converts the input into a pulse wave, and a flip flop subdivides the frequency by a factor of two. Slew is applied to round out the sound a bit, filtering out some of the higher harmonics of a square wave. An envelope follower for the input coupled with a VCA controls the -1 octave volume to match the input before applying the mix amount.

#### -2 (Octave -2 mix) knob and CV input
Controls the amount of two octaves down in the final mix.

The method for producing the -2 octave depends on the current mode.
- **Inversion** - Alternating cycles of the -1 octave are inverted and the result is summed with the original -1 octave.
- **Square** - The -1 octave passes through a second flip flop and slew is applied. An envelope follower for the input coupled with a VCA controls the -2 octave volume to match the input before applying the mix amount.

### DRIVE knob and CV input
Controls the amount of gain applied to the final mix. The large knob establishes the base gain ranging from 0 (off) to 10 (x10). The CV is scaled one for one and inverted and/or attenuated by the small attenuverter knob. The final effective gain is the sum of the scaled CV plus the base knob, clamped to a value between 0 and 10.

High gain levels result in saturation via the tanh limiter at the output.

### IN input
The input is AC coupled to make sure the input is bipolar. This is a requirement of the algorithms used to produce the octaves.

### OUT output
The final output is limited to +/-6 V via a saturating tanh function.

### Example octave waveforms
Oscilloscope traces showing the waveforms produced for each octave using different modes and inputs.

For each input and mode combination, the original waveform is shown in red to the left, and to the right are the four resultant waveforms:  
yellow = +1, red = 0, green = -1, blue = -2

![Octaver example output waveforms image](doc/OctaverWaveforms.png)

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The signal input is copied to the output when Octaver is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## POLY SCALE
![Poly Scale module image](doc/PolyScale.png)  
Provides a level control for each channel of a polyphonic signal. For each polyphonic output channel, the channel's input voltage is scaled (attenuated and/or iniverted and/or amplified) based on the Level knob for that channel, and then sent to the output.

### Level knobs
The Level knobs set the scale factor for each polyphonic input channel. The default range for all Level knobs is unipolar 0-1x.

A "Level range" option in the module context menu lets you specify a different range that is used for all the knobs.
- 0-1x (default)
- 0-2x
- 0-5x
- 0-10x
- +/- 1x
- +/- 2x
- +/- 5x
- +/- 10x

The default (initialize) level for each knob always starts out at 1x, regardless of the range. Of course the default can be overriden by the standard Venom parameter context menu option. 

### Output polyphonic channel count
By default the number of output channels matches the number of input channels. Knobs for channels above the output count are ignored.

There is a "Polyphony channels" option in the module context menu that lets you override the default and select a specific output channel count. Input channels and knobs above the specified channel count are ignored. Monophonic inputs are cloned to match the selected channel count. If the input channel count is poly but less than the selected channel count, then missing channel inputs are assumed to be constant 0 volts, meaning the output will also be 0 volts.

### Channel count display
The number of polyphonic channels at the output is displayed in the LED panel. The display will be yellow if the number of output channels is greater than or equal to the input channel count. The display will be red if the selected channel count is less than the input channel count.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Scale is bypassed then the input is passed unchanged to the output.

[Return to Table Of Contents](/README.md.md#venom)


## REFORMATION
![Reformation module image](doc/Reformation.PNG)  
Transform CV or audio by mapping way point voltages to new values.

### General Operation

Reformation transforms incoming CV or audio by remapping 5 voltage way points (min, 1/4, 1/2, 3/4, max) to new values, and performing linear interpolation of intermediate values. The input may be configured to handle unipolar or bipolar signals, and the output can be independently offset to unipolar or bipolar. Each way point mapping is controlled by a combination of a slider and two attenuverted CV inputs. The resultant signal can be overdriven with hard or soft clipping at 20V peak to peak, and then attenuated back down to the desired level. CV inputs are available for both the drive and final level. Oversampling is available to control aliasing that would otherwise be introduced by the transformation and/or clipping.

Reformation is fully polyphonic, and all modulation can be driven at audio rates.

Reformation can be be used as a waveshaper and/or VCA and/or distortion effect (hard clipper or saturating limiter). CV control of each way point provides for amplitude modulation of specific regions of a waveform.

### Way Point Sliders

Each slider is assigned an input voltage according to its label above, and the slider value determines the re-mapped voltage for the given way point. The sliders are labeled in a relative way. The exact values depend on the chosen input polarity.

|Polarity|MIN|1/4|1/2|3/4|MAX|
|---|---|---|---|---|---|
|Unipolar (0V - 10V)|0V|2.5V|5V|7.5V|10V|
|Bipolar (-5V - 5V)|-5V|-2.5V|0V|2.5V|5V|

Each slider defaults to the way point input voltage (no change), which is marked by a bold line on the scale.

### Way Point CV1 and CV2

Below each slider is a pair of bipolar CV inputs, each with its own attenuverter. The attenuverted CV inputs are summed with the slider values to establish the effective mapping for each way point. The summed value is not constrained, so it is possible to modulate a signal outside the standard unipolar or bipolar voltage ranges.

### Input Polarity

The color coded IN button establishes the expected polarity of the input:

- Green (default) = Unipolar
- Red = Bipolar

### Output Polarity

The color coded OUT button establishes the polarity of the final output. The transformed signal is offset as needed to achieve the correct output polarity.

- Green (default) = Unipolar
- Red = Bipolar

Note that unipolar output is not guaranteed to be >= 0V unless one of the clipping options is enabled.

### Drive

The drive amplifies the resultant signal prior to any clipping. The drive range is from 1 (no amplification) to 10 (multiply by 10), with the default at 2. The drive value is the sum of the knob value and the Drive input, clamped to a range from 1 to 10.

Note that unipolar inputs are offset -5V to become bipolar, prior to applying the drive.

### Clipping

The color coded CLIP button specifies the type of clipping that is applied.

- Gray = Off (no clipping)
- Yellow (default) = hard clipping at +/- 10V
- Orange = soft tanh clipping at +/- 10V

### Level

The level knob and level CV input function as a typical voltage controlled attenuator that is applied after the drive and clipping. The effective attenuation is the product of the Level knob (ranging from 0 to 1) and the Level input divided by 10V. The attenuation is then clamped to a value between 0 and 1. The default level is 0.5. So in the absence of any clipping, the default drive of 2 coupled with the default level of 0.5 results in no change.

### Output

The final output is offset by +5V if the output is configured to be unipolar. It is possible for unipolar output to have negative values if clipping has not been applied.

### Oversampling

The color coded OVER button specifies the amount of oversampling that is done to mitigate aliasing that can be introduced by the mapping transformation and clipping.

- Gray (default) = No oversampling
- Light Blue = x4
- Dark Blue = x8

Oversampling is relatively CPU intensive, and should only be applied when needed. Control voltage and low to medium frequency audio typically do not need oversampling. But the quality of moderately high frequency output can be improved by oversampling.

Note that oversampling cannot remove aliasing that may be present in inputs driven at audio rates. To get the best possible results, make sure that all audio signals at the IN, CV, DRIVE, or LEVEL inputs is clean.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

### Polyphony

The number of output polyphonic channels is set by the maximum number of channels found across all inputs. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels are assigned constant 0V for the missing channels.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The Input is passed unchanged to the Output when REFORMATION is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## SHAPED VCA
![SHAPED VCA module image](doc/ShapedVCA.png)  
Shaped VCA is a stereo polyphonic voltage controlled amplifier with a variable response curve, and optional hard/soft clipping, ring modulation, amplitude modulation, and oversampling.

The Shaped VCA can function as a typical voltage controlled amplifier or attenuator, or ring modulator, or amplitude modulator, or constant voltage source, or waveshaper, depending on which inputs are patched and how parameters are configured.

### R (Level Range) button
This color coded switch establishes the range of the Level knob amplification.

- yellow (default) = 0 to 1
- green = 0 to 2
- dark blue = 0 to 10
- pink = -1 to 1
- orange = -2 to 2
- purple = -10 to 10

### M (VCA Mode) button
This color coded switch establishes the VCA mode

- dark gray (default) = Unipolar 0-10V clipped CV (2 quadrant)
- white = Bipolar +/- 10V unclipped CV (4 quadrant)
- dark blue = Unipolar 0-5V clipped CV (2 quadrant)
- green = Bipolar +/- 5V unclipped CV (4 quadrant)

When using 0-5V or +/- 5V, the CV input is amplified x2 to get better exponential or logarithmic shaping. This makes the +/- 5V mode ideal for ring modulation because +/- 5V bipolar modulator and carrier inputs will result in +/- 5V bipolar output.

### C (Output Clipping) button
This color coded switch establishes how the output is clipped

- dark gray (default) = Off - No clipping
- yellow = hard clipped at +/- 10 Volts
- orange = soft clipped at +/- 10 Volts using an approximated tanh algorithm to provide saturation.

It is highly recommended that hard or soft clipping be applied if performing ring modulation with a logarithmic response curve using the original algorithm.

### O (Oversampling) button
This color coded switch establishes the amount of oversampling used to mitigate audio aliasing that may be introduced by clipping or non-linear response curves.

- dark gray (default) = Off - No oversampling
- yellow = x4 oversampling
- green = x8 oversampling
- light blue = x16 oversampling
- dark blue = x32 oversampling

Oversampling is typically not needed for most VCA operations. But it may be useful with high frequency audio outputs when clipping and/or non-linear response curves are applied. Oversampling is highly recommended if performing ring modulation or amplitude modulation with a logarithmic response curve.

Oversampling uses significant CPU resources, so it is best to use the minimum oversampling value that gives the desired output.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

### Level knob
Sets the maximum gain applied to the input signal(s). The range is dependent on the Range paramater. The default value is unity gain, regardless which range is chosen.

### Level CV input
Attenuates or inverts and attenuates the gain. The exact behavior is dependent on the Mode parameter setting. Regardless what mode, 10V equals full maximum level, and 0V equals zero output. The effective gain for in between values is dependent on the Curve parameter and CV input. When in unipolar mode, the level input is clamped to a 0 to 10 volt range.

The Level input is normaled to 10V so that unpatched level input results in the full gain specified by the Level knob. In this way Shaped VCA can operate as an attenuator or amplifier, without voltage control.

### Bias knob

The Bias knob can add an offset ranging from -5V to 5V to the Level CV input. It is useful for converting a bipolar Level input to unipolar so it can be wave shaped by the Curve response. It is also useful for cross fading between ring modulation and amplitude modulation when the VCA Mode is set to bipolar. The bias is applied prior to any exponential or logorithmic shaping of the CV input.

### Response Curve knob
Controls the response curve of the level CV input, with full clockwise (100%) giving an approximated logarithmic response, noon (0) a linear response, and full counterclockwise (-100%) an exponential response. Intermediate values cross fade between the extremes and linear.

The default value is 0 = linear response curve.

### Curve CV input
The Curve CV input is multiplied by 10 and then summed with the Curve knob value to establish the effective response curve. The final effective curve level is clamped to +/- 100%.

The Curve CV input is normaled to 0V.

### Logarithm and Exponential algorithms

Logarithms aren't defined for negative values, but ring modulation needs to support negative values. So to make logarithmic responses work with ring modulation, the approximated logarithm of the absolute value is used and then multiplied by the sign of the original value (1 or -1).

Exponential shapes also support negative values.

The original version of Shaped VCA used different algorithms for logarithms and exponentials. The original logarithm produced wicked high voltage spikes that required clipping, and the exponential effectively used the absolute value of the input.

Starting with version 2.5.0, the improved logarithm formula was used, but the exponential formula still used the absolute value.

Negative exponential values were introduced in version 2.8.0.

The original algorithms are probably not what is wanted, but to support any old patches that relied on the old behavior, there is an **Exp/Log algorithm** context menu option to force the use of prior alogorithms.
  - **Corrected** = the newest preferred alogorithms. Shaped VCA modules placed into a new patch default to this option.
  - **Intermediate** = the corrected logarithm algorithm, but absolute value exponential algorighm. Patches created from v2.5.0 through v2.7.0 default to this option.
  - **Original** = original logarithm and absolute value exponential. Patches created prior to v2.5.0 default to this option.

Note that the Intermediate and Original options have no effect if using 0-5V unipolar or +/-5V bipolar VCA modes. These VCA modes did not exist prior to v2.8.0, so there is no prior behavior to support, and the corrected algorithms are used.

### Left and Right inputs

The Right input is normaled to the Left input. The Left input is normaled to 10V so that Shaped VCA without any patched inputs can function as a constant CV source with the Level knob setting the value. The 10V normaled input is also convenient for using Shaped VCA as a waveshaper for the Level input.

### Left and Right outputs

After applying the effective gain to the inputs, the final result is sent to the Left and Right outputs.

### O (Output Offset) button adjacent to output ports
This color coded switch can apply an offset to the final output. The output offset is applied before any clipping.

- dark gray (default) = Off - No offset
- red = -5 volt offset
- green = +5 volt offset

### Order of operations
1) Inputs are upsampled if using oversampling
2) Bias applied to Level CV input
3) Level CV input x2 if using 0-5V or +/- 5V VCA mode
4) Exponential or logarithmic shaping applied
5) Shape result attenuates the final level
6) Final level attenuates the input(s)
7) Output bias applied
8) Clipping applied
9) Output band limited and downsampled if using oversampling

### Some Example Use Cases (just the tip of the iceberg)
#### "Normal" VCA
Set VCA Mode button to Unipolar 0-10V  
Set Bias knob to 0V  
Patch envelope or other CV to Level CV input  
Patch audio or CV to Left and/or Right input.  

#### Ring Modulation using +/- 5V carrier and modulation
Set VCA Mode button to Bipolar +/- 5V  
Set Level knob to 1.0  
Set Bias knob to 0V  
Patch modulator to Level CV in  
Patch carrier to input  

#### Amplitude Modulation using +/- 5V carrier and modulation
Set VCA Mode to Unipolar 0-10V (or Bipolar +/- 10V if you are worried about unwanted clipping)  
Set Level knob to 1.0  
Set Bias knob to +5V  
Patch modulator to Level CV in  
Patch carrier to input  

#### Apply symmetric exponential or logarithmic shaping to bipolar +-5V audio or CV
Set VCA Mode button to Bipolar +/- 5V  
Set Level knob to 0.5   
Adjust Response Curve knob to desired shape  
Patch bipolar signal to Level CV in  
Leave input unpatched  

#### Apply asymmetric exponential or logarithic shaping to bipolar +-5V audio or CV
Set VCA Mode button to Unipolar 0-10V (or Bipolar +/- 10V if you are worried about unwanted clipping)  
Set Level knob to 1.0  
Set Bias knob to +5V  
Adjust Response Curve knob to desired shape  
Patch bipolar signal to Level CV in  
Leave input unpatched  
Set Output Offset button to -5V  

#### Apply symmetric exponential or logarithmic shaping to unipolar 0-10V signal
Set VCA Mode button to Bipolar +/- 5V  
Set Level knob to 0.5  
Set Bias knob to -5V  
Adjust Response Curve knob to desired shape  
Patch unipolar signal to Level CV in  
Leave input unpatched  
Set Output Offset button to +5V  

#### Apply asymmetric exponential or logarithmic shaping to unipolar 0-10V signal
Set VCA Mode button to Unipolar 0-10V (or Bipolar +/- 10V if you are worried about unwanted clipping)  
Set Level knob to 1.0  
Set Bias knob to 0V
Adjust Response Curve knob to desired shape  
Patch unipolar signal to Level CV in  
Leave input unpatched  

### Polyphony

The number of output polyphonic channels is set by the maximum number of channels found across all inputs. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels are assigned constant 0V for the missing channels.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The Left and Right inputs are passed unchanged to the Left and Right outputs when the module is bypassed. The Right input remains normaled to the Left input while bypassed. However, the left input is not normaled to 10V while bypassed.

[Return to Table Of Contents](/README.md.md#venom)

## SLEW
![SLEW module image](doc/Slew.png)  
Polyphonic slew limiter and slope detector for both CV and audio processing.

When you slew an input signal, you limit the maximum rate at which the signal can change voltage. The slew rate is measured as the number of milliseconds to rise or fall 10 Volts. The Slew module provides independent Rise and Fall times that can be linear or curved. Slew times and shapes can be modulated at audio rates. A V/Oct input is provided to proportionally scale the Rise/Fall times as the input frequency changes.

There are many possible uses for this behavior
- Adding portamento (glide) to V/Oct pitch sequences
- As a crude low pass filter
- If using gate inputs, as an Attack, Sustain, Release envelope generator
- If the input is fully rectified to positive voltages, then a fast rise time with slow fall time can function as an envelope follower
- As a waveshaper, with modulated time and/or shape providing a dynamic shifting sound

In addition to slewing an input signal, the Slew module also provides gate outputs indicating whether the slewed output is rising, falling, or flat.

### FAST (Speed) button
When enabled (green), the Fall and Rise time knobs are scaled to be suitably fast for slewing audio rate inputs.

### OVER (Oversample) button
This color coded button sets the oversampling rate used to mitigate aliasing. Oversampling is usually only needed when slewing audio inputs.

- Off (gray - default)
- x2 (yellow)
- x4 (green)
- x8 (light blue)
- x16 (dark blue)
- x32 (purple)

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

### FALL and RISE time
The Fall and Rise times specify how many milliseconds it takes to rise or fall 10 Volts.

Each parameter has a top knob to set the base value and a CV input with attenuverter knob to modulate the base value.

The time knobs are scaled exponentially, and have two different scales depending on whether Fast is enabled or not.

#### Normal (slow)
- Minimum (counterclockwise) - 7.8125 msec
- Noon (default) - 250 msec
- Maximum (clockwise) - 8000 msec

With both Rise and Fall set to the default value of 250 msec, Slew will convert a 2 Hz 10V peak to peak square wave into a 10V peak to peak triangle wave.

#### Fast (audio rate)
- Minimum (counterclockwise) - 0.060223 msec
- Noon (default) - 1.9111 msec
- Maximum (clockwise) - 61.155 msec

With both Rise and Fall set to the default value of 1.9111 msec, Slew will convert a 261.63 Hz (C4) 10V peak to peak square wave into a 10V peak to peak triangle wave.

The Rise and Fall CV inputs can be inverted and/or attenuated by attenuverter knobs that range from -100% to 100%, with the default noon value at 0%.

Each +1V will double the time, and -1V will halve the time.

Note that Rise and Fall times are precise when using a linear shape (0% curve). If using a 100% curve shape, then the time represents approximately a 9 Volt change instead of 10 Volts.

### FALL and RISE shapes
The slewed Rise and Fall can be independently set to be linear or curved. If curved, then large input changes move faster than small input changes.

Rise and Fall each have a shape knob with fully counter-clockwise being linear, fully clockwise curved, and noon a blend of the two. The knobs are scaled to represent the percentage of curvature, with 0% being linear.

The shapes can be modulated via shape CV input ports with attenuverter knobs. The attenuated CV is summed with the knob value. The modulation is scaled at 10% curve per Volt.

### IN (Raw) input
This is the raw input you want to slew.

### V/Oct input
This CV input is used to scale slew rates proportionally as your input frequency changes. Each +1V halves the Rise and Fall times, and -1V doubles the Rise and Fall times. This modulation is great for using Slew as a waveshaper.

### Gate outputs
There are three gate output ports that indicate the slope of the slewed output
- **RISE** - 10V when rising, else 0V
- **FALL** - 10V when falling, else 0V
- **FLAT** - 10V when steady (not changing), else 0V

Between the Rise and Fall labels is a small button to control the polarity of the gate outputs

**Gate polarity**
- **Unipolar** (green default)  0V - 10V
- **Bipolar** (red) -5V - +5V

Note that gate outputs can be noisy when processing audio inputs. It is best to use Slew oversampling when working with audio inputs. In an effort to reduce gate noise, the slope detector sensitivity is normally reduced when oversampling is enabled. There is a context menu option to set the oversampled slope sensitivity, expressed as minimum delta voltage per sample needed to detect a rising or falling slope.

**Oversampled slope sensitivity (min delta)**
- 10 mV (default)
- 1 mV
- 0.1 mV
- 0.01 mV
- 0.001 mV

The sensitivity is always 0.001 mV whenever oversampling is disabled.

### OUT (Slewed) output
This is the final result of the slew processing.

### Polyphony
All inputs and outputs are fully polyphonic. The number of output polyphonic channels is set by the maximum number of channels found across all inputs. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels are assigned constant 0V for the missing channels.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The IN (raw) input is replicated at the OUT (slew) output when Slew is bypassed. All other outputs are constant monophonic 0V.

[Return to Table Of Contents](/README.md.md#venom)


## SPHERE TO XYZ
![Sphere To XYZ module image](doc/SphereToXYZ.png)  
Converts spherical coordinates r, theta, phi into cartesian coordinates X, Y, Z using standard physics definitions:

> x = r x sin(theta) x cos(phi)  
> y = r x sin(theta) x sin(phi)  
> z = r x cos(theta)  

When applied to audio rate inputs, it has an effect similar to ring modulation.

### Polyphony
All inputs and outputs are fully polyphonic. The number of output channels is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the output channel count. Polyponic inputs with fewer channels use constant 0V for any missing channels.

### OverSample button
This color coded switch establishes the amount of oversampling used to mitigate audio aliasing that may be introduced by the conversion process.

- dark gray (default) = Off - No oversampling
- yellow = x2 oversampling
- green = x4 oversampling

Oversampling is typically not needed for most conversions. But it may be useful with high frequency audio outputs.

Oversampling uses significant CPU resources, so it is best to use the minimum oversampling value that gives the desired output.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.


### r input
This represents the radial distance r. Negative r values are accepted.

### Theta input
This represents the polar angle theta. It is scaled at 36V/degree, meaning 5V = 180 degrees. Any angle is allowed.

### Phi input
This represents the azimuthul angle phi. It is scaled at 36V/degree, meaning 5V = 180 degrees. Any angle is allowed.

### Scale switch
Specifies the scale factor used for converting spherical radial distances into cartesian distances.
- 1:1 = +/-5V radial distance range specifies a 10V diameter sphere centered about the origin that is inscribed within a 10V x 10V x 10V cube.
- sqrt(3):1 = +/-5V radial distance range specifies a ~17.32V diameter sphere centered about the origin with a 10V x 10V x 10V cube inscribed within it.

Assuming that inputs are bipolar +/-5V, then a 1:1 ratio guarantees that all converted X, Y, and Z outputs are within +/-5V. However, not all possible +/-5V X, Y, Z coordinates are covered.

The sqrt(3):1 ratio guarantees that a bipolar +/-5V radial distance can cover all possible +/-5V X, Y, Z coordinates. However, some converted values may exceed the +/-5V range, depending on the input angles.

### X output
This represents the x cartesian coordinate, after conversion.

### Y output
This represents the y cartesian coordinate, after conversion.

### Z output
This represents the z cartesian coordinate, after conversion.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are constant monophonic 0V when Sphere To XYZ is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## WAVE FOLDER
![WAVE FOLDER module image](doc/WaveFolder.png)  
A polyphonic configurable wave folder and VCA.

The incoming signal is amplified and then folded at +/-5 volts a fixed number of stages. Increasing amplification increases the number of folds, with a limit set by the Stages count. The signal can be amplified once before any folding, and/or once for each folding stage. The folding can be made asymmetric by applying a bias voltage to the signal prior to any amplification. The final result is soft clipped at +/-6V by tanh saturation.

All amplification can be controlled via CV, thus making the wavefolder a VCA as well. The VCAs can be configured to be unipolar or bipolar, and they can process audio rate CV, so the module can also perform wave folded amplitude modulation or wave folded ring modulation.

Watch [this video for a brief introduction and some example use cases](https://www.youtube.com/watch?v=HLTyiIAUELs). But please do read the rest of the documentation for some more details.
Also watch [this Omri Cohen video](https://www.youtube.com/watch?v=BHY5gI--D-c) demonstrating a number of applications for the Wave Folder.

### STAGES button
Controls the maximum amount of folding stages, which in turn limits the maximum number of folds. There are five possible values
- 2
- 3 (default)
- 4
- 5
- 6

### OVERSAMPLE button
Wave folding, ring modulation, and amplitude modulation can introduce many harmonics, which can lead to aliasing. Oversampling can be used to limit the amount of aliasing.
- Off
- x2
- x4 (default)
- x8
- x16
- x32

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

By default, oversampling is applied to all inputs.

Oversampling has a significant CPU cost, so best to apply the minimum amount that sounds good. LFO rate modulation should not need oversampling. The three CV input ports have a context menu option to disable oversampling for that port. These ports have a LED above and to the right of the port. It is off if there is no patched input, or if the OverSample button is set to Off. It is yellow if there is input and oversampling is applied. It is red if there is input and oversampling is active for the module, but disabled for that port.

### Polyphony
Wave Folder is fully polyphonic. The number of output channels is the maximum number of polyphony channels found accross all inputs.

Inputs that match the output polyphony behave as expected. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels get constant 0V for any missing channels.


### PRE-AMP knob and CV input
Sets the amount of amplification applied to the input prior to any folding stages.

The total amplification is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Pre-amp CV Amount knob. CV can be driven at audio rates.

The knob ranges from 0 to 10, with a default of 1 (unity). But the CV is not limited, so the net amplification is unconstrained.

By default, the Pre-Amp VCA is unipolar, meaning any net amplification level less than zero is treated as zero. The port has a context menu option to use a bipolar VCA instead that can process negative values and invert the signal. The LED above and to the left of the port glows yellow if bipolar mode is enabled. 

The Pre-Amp CV port also has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.


### STAGE AMP knob and CV input
Sets the amount of amplification applied at each stage of folding. See the note at the bottom of this section if you are interested in the exact mathematical formula.

The total stage amplification is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Stage Amp CV Amount knob. CV can be driven at audio rates.

The knob ranges from 0.5 to 10, but the CV is not limited, so the net amplification is unconstrained.

Note that the Stage Amp knob is scaled exponentially, but the CV is linear. The exponential knob scale makes it easier to dial-in the most useful values between 1 and 2.

By default, the Stage Amp VCA is unipolar, meaning any net amplification level less than zero is treated as zero. The port has a context menu option to use a bipolar VCA instead that can process negative values and invert the signal. The LED above and to the left of the port glows yellow if bipolar mode is enabled. 

The Stage Amp CV port also has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.

***Actual stage folding formula***

*Without any stage amplification, the folding formula for each stage is V<sub>out</sub> = clip(V<sub>in</sub>) - (V<sub>in</sub> - clip(V<sub>in</sub>)), where clipping occurs at +/-5 volts.  
This can be re-written as V<sub>out</sub> = 2 x clip(V<sub>in</sub>) - V<sub>in</sub>*

*The stage amplification is only applied to the clip expression as follows: V<sub>out</sub> = 2 x clip(V<sub>in</sub> x Amp<sub>stage</sub>) - V<sub>in</sub>*

### BIAS knob and CV input

Sets the amount of bias voltage to add to the input signal prior to any amplification. Non-zero bias leads to asymmetric folding.

The total bias is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Bias CV Amount knob. CV can be driven at audio rates.

The knob ranges from -5 to 5, but the CV is not limited, so the net bias is unconstrained.

The Bias CV port has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.

### IN input
This is the input signal to be folded.

The input is always oversampled at the level selected by the OverSample button.

### OUT output
This is the final result of the wave folding.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The poly input is passed through to the output if Wave Folder is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## WAVE MANGLER
![WAVE MANGLER module image](doc/WaveMangler.png)  
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

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

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
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
If Wave Mangler is bypassed then all channels of the Wave input are passed through to the Wave output unchanged.

[Return to Table Of Contents](/README.md.md#venom)


## WAVE MULTIPLIER
![WAVE MULTIPLIER module image](doc/WaveMultiplier.png)  
A polyphonic waveshaper inspired by the [Doepfer A-137-2 Wave Multiplier II](https://doepfer.de/a1372.htm) with integrated LFO modulation. Its primary use is to fatten the sound of simple wave forms like sine, triangle, and saw. It does not work well with square or pulse waves.

The Wave Multiplier works by mixing 4 copies of the incoming wave form with the original input. Each copy is compared to a different threshold, and if the current value is less than or equal to the threshold, then the value is shifted up as much as 5V. If greater than the threshold then the value is shifted down as much as 5V. If the input is a saw wave and the threshold is constant, then the net effect is a phase shift. If the input is a sine or triangle it is more like a fold operation. Applying modulation to the threshold adds movement to the sound.

The overall design is functionally very similar to the Doepfer module, with the following enhancements:
- Four bipolar triangle LFOs are added, with the outputs normalled to the shift threshold CV inputs to provide built in modulation.
- Rather than adjusting the input level, Depth controls and CV input are provided for the shift amount.
- The 8 internal outputs and 5 jumpers on the back of the Doepfer module are exposed as additional outputs and mutes on this Venom module.
- An option is added to remove DC offset from outputs.
- Level control and a VCA are provided for the final mix output.
- Oversampling options are provided to mitigate aliasing introduced by the digital implementation.

The Wave Multiplier default configuration is designed to produce pleasing fat sounds with lots of movement using only one input and one output. Patching the V/Oct CV used on the source input into the Wave Multiplier LFO V/Oct may yield more consistent results. Obviously more range is available by experimenting with the various controls and patching additional CV inputs.

The Wave Multiplier can be divided into three vertical sections
- Top LFO section
- Middle shift section
- Bottom Mix and I/O section 

### *Top LFO section*

### Master frequency knob

Sets the base frequency voltage of all four LFO oscillators using a volt per octave scale.

### V/Oct input

CV input for the base frequency voltage of all four oscillators. The CV is summed with the Master voltage.

### Frequency offset knobs 1-4

Each knob sets an offset frequency voltage between -1 and 1 that is summed with the base frequency to establish the final LFO frequency. The knob defaults are uncorrelated, and range a bit over 1.5 octaves.

### LFO outputs 1-4

The +/-5V bipolar triangle LFO outputs

### *Middle Shift section*

### Depth (shift amount) controls and input

Controls the magnitude of the voltage shift that is applied to the four wave copies.

#### Depth CV input

Supports audio rate modulation

#### Depth CV amount knob

Attenuates and/or inverts the Depth CV

#### Depth knob

Sets the base shift magnitude, ranging from 0 to 5 volts. The final attenuated CV is added to the knob value to establish the net shift magnitude.

### Shift Threshold

Establishes the voltage threshold where the wave copy is either shifted up or down. Voltages above the threshold are shifted down, and voltages equal to or below the threshold are shifted up.

#### Shift threshold CV inputs 1-4

Allows modulation of the shift threshold via CV. Each input is normalled to the LFO output above. Supports audio rate modulation.

#### Shift Threshold CV amount knobs 1-4

Attenuates and/or inverts the threshold CV.

#### Shift Threshold knobs 1-4

Sets the base threshold voltage for shift operations. The base value is added to the attenuated CV value to get the net threshold.

### Pulse outputs 1-4

The outputs for the four shift comparators. These are +/- 5V bipolar pulse waves. The shift depth controls are not applied to the pulse outputs.

Modulation to the shift thresholds produce pulse width modulation.

### Shifted Wave outputs 1-4

The outputs of the shifted waves. The pulse wave is attenuated by the depth control before being summed with the input wave copy to create the output shifted wave.

### *Bottom Mix and I/O section*

### In (wave input)

The main input, typically an audio signal.

### Shifted Mutes 1-4

Controls which of the shifted waves are included in the final output mix

### In Mute

Controls whether the raw input is included in the final output mix

### Output Level VCA

#### Output Level CV input

10V corresponds to 100%. Audio rate modulation is supported. Negative values invert the output, so the VCA can function as a ring modulator.

#### Output Level CV amount knob

Attenuates and/or inverts the level CV

#### Output Level knob

Sets the base level amount (bias) between 0 and 100%. The base level is summed with the attenuated CV to establish the final output level.

### OUT (shifted wave mix) output

The final output consisting of the mix of unmuted shifted waves and the unumuted input wave, attenuated by the Output Level.

### DC Block button

If enabled, then DC offset will be removed from the Pulse, Shifted Wave, and Out outputs

### Over (oversample) button

Wave shifting and or audio rate level modulation can introduce undesireable inharmonic digital aliasing artifacts. Activating oversampling can mitigate those effects to yield a cleaner, more musical result.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

### Polyphony

Wave Multiplier is fully polyphonic. In general, the number of output channels is computed as the maximum channel count found across all inputs.

Inputs that match the output polyphony behave as expected. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels get constant 0V for any missing channels.

The LFOs are special. If the Master LFO V/Oct input is monophonic (or unpatched), then the LFO outputs are monophonic, regardless wether there are any polyphonic inputs elsewhere. But if the V/Oct is polyphonic, then the LFO output channel count matches the channel count for the rest of the module, and missing LFO V/Oct channels are treated as constant 0V.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
If Wave Multiplier is bypassed, then all channels of the wave input are passed through unchanged to the shifted wave mix output. All other outputs are monophonic constant 0 volts.

[Return to Table Of Contents](/README.md.md#venom)


