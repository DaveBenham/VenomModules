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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### Polyphony

The number of output polyphonic channels is set by the maximum number of channels found across all inputs. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels are assigned constant 0V for the missing channels.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The Input is passed unchanged to the Output when REFORMATION is bypassed.
