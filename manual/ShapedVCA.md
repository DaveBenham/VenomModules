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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

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
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The Left and Right inputs are passed unchanged to the Left and Right outputs when the module is bypassed. The Right input remains normaled to the Left input while bypassed. However, the left input is not normaled to 10V while bypassed.
