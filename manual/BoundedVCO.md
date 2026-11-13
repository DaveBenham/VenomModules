An implementation of Peter Blasser's Bound/Bounce oscillation that constrains an oscillator with variable slopes between variable upper and lower bounds. This concept was used in his Ciat-Lonbarde Sidrax, Tetrax, and Quadrax organs, as well as the Ieaskul F. Mobenthey Fourses, Denum, and Swoop Eurorack modules.

The primary output of Bounded VCO is a triangle waveform. This is a geometry definition of a triangle, not a traditional synthesizer oscillator definition. Depending on the slope configuration, the shape can range from a saw with ascending ramp, to a traditional triangle, to a saw with a descending ramp.

The oscillator starts out rising until it hits the upper bound, at which point it begins falling until it hits the lower bound and begins rising again. If the lower bound rises above the upper bound then the oscillator follows the lower bound. There is an option to automatically swap the upper and lower bounds such that the upper bound is always greater than or equal to the lower.

There are two ways to control the frequency of a Bounds/Bounce oscillation that have an inverse relationship. The frequency can be increased by either increasing the magnitude of the Bounce slopes or decreasing the distance between the upper and lower Bounds. Of course changing the distance between the bounds also changes the oscillation amplitude.

There is a secondary pulse output that is high (5V) when the triangle is rising, and low (-5V) when the triangle is falling.

Bounded VCO is fully polyphonic. The number of output channels is the maximum number of channels found across all inputs. Monophonic inputs are replicated to match the number of output channels. Polyphonic inputs that have fewer channels use constant 0V for missing channels.

All CV inputs can be driven at audio rates.

### SLOW button
Controls whether the Frequency knob is configured for audio rates or low frequency rates.
- **Off** ***(dark gray, default)*** - Frequency knob ranges from ~8 Hz to 8372 Hz, with the default noon value at 261.63 Hz (C4).
- **On** ***(yellow)*** - Frequency knob ranges from 0.0625 Hz to 64 Hz, with the default noon value at 2 Hz.

### OVER (Oversample) button
Controls the level of oversampling used to mitigate digital aliasing.
- **Off** ***(gray, default)***
- **x2** ***(yellow)***
- **x4** ***(green)***
- **x8** ***(light blue)***
- **x16** ***(dark blue)***
- **x32** ***(purple)***

Bounded VCO is typically used with modulation of the upper and lower bounds, which leads to relatively harsh sounds that mask digital aliasing. So oversampling can often be turned off, even if operating with audio frequencies.

### SWAP button
Controls whether the lower bound is allowed to rise above the upper bound.
- **Off** ***(dark gray, default)*** - The upper and lower bounds are not swapped if the lower bound exceeds the upper bound.
- **On** ***(yellow)** - The upper bounds are automatically swapped if the lower bound exceeds the upper bound.

### Slope control
The rising and falling slopes of the triangle output are specified by Frequency and Skew controls. The minimum slope magnitude is 0.001 mV per sample to prevent the oscillator from stalling.

#### FREQ (Frequency) knob and CV input with attenuverter
The Frequency knob is scaled assuming the upper and lower bounds are constant yielding a 10V peak to peak triangle waveform. The frequency range is dependent on the Slow button setting.

The bipolar Frequency CV input can modulate the knob frequency. It has an associated attenuverter that can attenuate and/or invert the signal. The CV is V/Oct when the attenuverter is at 100%.

The CV can modulate the Frequency beyond the limits of the knob.

#### SKEW knob and CV input with attenuverter
The skew controls the shape of the triangle output.

The knob is configured to show the percentage of time the oscillator spends rising. It ranges from 1% to 99%, with the default noon value at 50%.
- 1% approximates a saw wave with a descending ramp.
- 50% is a traditional perfect triangle
- 99% approximates a saw wave with an ascending ramp

The bipolar Skew CV input is additive with the knob value. It has an associated attenuverter that can attenuate and/or invert the signal. With the attenuverter at 100% the CV is scaled at 10% per volt.

### Bounds control

#### FLOOR knob and CV input with attenuverter
Defines the lower bound.

The knob ranges from -10V to 10V, with the default at -5V.

The bipolar Floor CV input is additive with the knob value. It has an associated attenuverter that can attenuate and/or invert the signal.

The final effective lower bound is clamped to a value between -12V and 12V.

#### CEILING knob and CV input with attenuverter
Defines the upper bound.

The knob ranges from -10V to 10V, with the default at +5V.

The bipolar Ceiling CV input is additive with the knob value. It has an associated attenuverter that can attenuate and/or invert the signal.

The final effective upper bound is clamped to a value between -12V and 12V.

### PULSE output

The bipolar Pulse output is high at +5V when the triangle is rising, and -5V when the triangle is falling.

### TRI (Triangle) output

The triangle output may be unipolar or bipolar, depending on the upper and lower bounds. The output is nominally clamped between the upper and lower bounds, which can range between -12V and 12V. However, the output may temporarily stray beyond those bounds.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The output is constant monophonic 0V when Bounded VCO is bypassed.
