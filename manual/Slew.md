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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

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
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The IN (raw) input is replicated at the OUT (slew) output when Slew is bypassed. All other outputs are constant monophonic 0V.
