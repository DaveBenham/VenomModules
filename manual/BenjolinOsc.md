A complex chaotic oscillator emulating the oscillator and rungler components of a Benjolin. It produces 7 outputs: two pairs of triangle and pulse waves with exponential FM, two varying width pulse outputs, and a stepped voltage output similar to a random sample & hold. Frequency range is very wide, from slow LFO rates to high audio rates. Connect a resonant filter with excellent ping characteristics, and you have a complete functional Benjolin.

Also check out the premium [Venom Chaos Boxes Venjolin](https://github.com/DaveBenham/VenomPremium/blob/main/VenomChaosBoxes.md#venjolin) for a one module implementation of a complete Benjolin, including the filter.

The Benjolin was invented by Rob Hordijk in 2009. It is a non-traditional / experimental electronic musical instrument that uses a small number of simple circuits and a minimal number of knobs to create an astonishing range of sounds and patterns. Extensive use of feedback and cross-modulation makes the Benjolin a chaotic sound source, with the ability to create stable patterns as well.

There have been many variations of the Benjolin, but the basic functional architecture is always the same - two oscillators, a specialized shift register construct called the Rungler, and a resonant state variable filter. This module is based on the Eurorack version 2 of the Benjolin created by After Later Audio in collaboration with Rob Hordijk.

The Benjolin Oscillator is arranged in three vertical sections: OSC1, OSC2, and RUNGLER.

The oscillators are digital with triangle and pulse outputs. They are intentionally not exactly 1V/Octave.

### OSC1 (Oscillator 1)
#### FREQ1 knob
Sets the base frequency of oscillator one. Fully counterclockwise is roughly 0.03 Hz (30 seconds per cycle). The oscillator cannot be modulated below this minimum frequency. The oscillator remains in LFO territory up through noon at about 15 Hz. A bit above that and it transitions to audio rates, with a maximum fully clockwise frequency of around 7.8 kHz without any modulation. With modulation the oscillator maximum frequency is about 12.5 kHz.

#### RUNG1 (Rungler 1) knob
Controls how much the rungler signal modulates oscillator 1 frequency. The knob is a bipolar attenuverter ranging from -1 (100% inverted) to 1 (100%), with the default noon value of 0 (no modulation).

#### CV1 (Control Voltage 1) knob and input
Bipolar input with a bipolar attenuverter to modulate oscillator 1 frequency. The attenuverter ranges from -1 (100% inverted) to 1 (100%), with the default noon value of 0 (no modulation).

The CV1 input is normalled to the Oscillator 2 triangle output.

#### TRI1 (Triangle 1) output
Triangle waveform bipolar output for oscillator one, ranging from -5 to 5 volts.

#### PULSE1 output
Pulse waveform bipolar output for oscillator one, ranging from -5 to 5 volts.

### OSC2 (Oscillator 2)
#### FREQ2 (Oscillator 2 Frequency) knob
Sets the base frequency of oscillator two. Fully counterclockwise is roughly 0.03 Hz (around 30 seconds per cycle). The oscillator cannot be modulated below this minimum frequency. The oscillator remains in LFO territory up through noon at about 15 Hz. A bit above that and it transitions to audio rates, with a maximum fully clockwise frequency of around 7.8 kHz without any modulation. With modulation the oscillator maximum frequency is about 12.5 kHz.

#### RUNG2 (Rungler 2) knob
Controls how much the rungler signal modulates oscillator 2 frequency. The knob is a bipolar attenuverter ranging from -1 (100% inverted) to 1 (100%), with the default noon value of 0 (no modulation).

#### CV2 (Control Voltage 2) knob and input
Bipolar input with a bipolar attenuverter to modulate oscillator 2 frequency. The attenuverter ranges from -1 (100% inverted) to 1 (100%), with the default noon value of 0 (no modulation).

The CV2 input is normalled to the Oscillator 1 triangle output.

#### TRI2 (Triangle 2) output
Triangle waveform bipolar output for oscillator two, ranging from -5 to 5 volts.

#### PULSE2 output
Pulse waveform bipolar output for oscillator two, ranging from -5 to 5 volts.

### OVERSAMPLE switch
This small color coded switch controls how much oversampling is applied to reduce aliasing artifacts in the outputs.
- Off (gray)
- x2 (yellow)
- x4 (green)
- x8 (light blue - default)
- x16 (dark blue)
- x32 (purple)

Aliasing might not be noticeable with chaotic and/or low frequency outputs. But the aliasing can become painfully obvious when producing high frequency coherent output unless oversampling is used. But oversampling is rather CPU intensive, so you want to use the minimum amount that gives good results. An oversample value of x8 uses reasonable CPU with VCV running at 48 kHz, and provides clean output in all but the most extreme cases.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

Due to float arithmetic limitations, the oscillators would stall at the lowest frequency if the VCV sample rate is set above 48 kHz and high oversample rates are used. To compensate, the maximum allowed oversampling is reduced as the VCV sample rate increases. This enables the oscillators to cover their full range regardless what VCV sample rate is used.

### RUNGLER
The Rungler consists of an eight step shift register driven by a clock and a data input, along with comparators, logic gates, and a digital to analog converter (DAC). The rungler data input is always derived from the oscillator 1 triangle output. The clock input defaults to the oscillator 2 pulse output, but may be overridden at the clock input. When a bit shifts out of the shift register, it is XORed with the data input and fed back into the low bit of the ASR. The Rungler produces multiple output signals. Depending on configuration and the incoming data, the rungler output may be chaotic, or it may have a readily recognized pattern.

By default the Benjolin Oscillator DAC is configured to use bits 2,4,7 in order to maximize the number of available Rungler patterns. There is a module context menu option to use Rob Hordijk's original design of bits 6,7,8.

Above the Rungler label are eight LEDs representing the shift register bits. The DAC LEDs glow bright yellow when high. The remaining LEDs glow dim yellow when high.

#### PATTERN knob
Controls whether the Rungler repeats a pattern or is chaotic. When fully anticlockwise, the Rungler produces an 8 step pattern. When fully clockwise it produces a 16 step pattern, with the first 8 steps being a mirror image of the second 8 steps. At noon the rungler output is chaotic.

Below is a technical discussion of how it works.

The Pattern knob range is from -1 to 1, with a default of 0 at noon. The raw internal triangle signal also ranges from -1 to 1. If the Pattern value is greater than the instantaneous incoming triangle value, then the rungler input is high (1), else the input is low (0). The input is XORed with the recycled shift register bit. So a high input inverts the recycled bit, and a low input preserves the recycled bit.

When the Pattern knob is fully counterclockwise, the input is guaranteed to be zero, the recycled bit is preserved, and an 8 step pattern is established.

When the Pattern knob is fully clockwise, the input is guaranteed to be 1, the recycled bit is inverted, and a 16 step pattern is established.

At noon there is a roughly 50-50 chance of 0 or 1, so the rungler output is at its most chaotic, typically with no recognizeable pattern.

#### CHAOS button and input
If enabled, the Pattern knob is ignored, and the rungler output is always chaotic. Each press of the button toggles the state of the chaos button. The leading edge of a trigger at the input will also toggle the button state.

The Chaos button works by converting the 8 step shift register into a 127 step linear feedback shift register, and by ignoring the Pattern knob and always comparing against 0, so there is always a 50% chance of inversion of the recycled bit.

If Chaos is off, then the rungler may fall into a pattern over time, even if the Pattern knob is at noon. The Chaos button is useful for re-introducing chaos into the output.

#### DOUBLE button and input
If enabled, the Rungler is triggered at double the rate. Normally the Rungler is triggered by the leading edge of a clock gate. When in Double mode the Rungler is triggered by both the leading and trailing edges of a clock gate.

Each press of the Double button toggles the state of the button. The leading edge of a trigger at the input will also toggle the button state.

#### CLOCK input

This is the input that triggers the Rungler. It expects a bipolar clock input, with a high state detected above +1V, and a low state below -1V.

If oversampling is enabled, then interpolation of square unipolar clock input will have enough "bounce" to reset to a negative state. But square unipolar clocks will not work if oversampling is off. Nor will unipolar sine or triangle waves work regardless of oversampling status. There is a "Unipolar clock input" module context menu option available to work with any unipolar clock input.

The Clock input is normaled to the Oscillator 2 pulse output.


#### XOR output

This is simply the first bit of the Rungler shift register, scaled and offset to be bipolar +/-5V.

#### PWM output

Although this is under the Rungler section, it actually has nothing to do with the Rungler. This bipolar +/-5V output is produced by a comparator that outputs 5V when the TRI2 signal is greater than TRI1, and -5V otherwise. This is the signal that traditionally gets sent to the Benjolin filter input. It was called PWM by Rob Hordijk because it produces a series of variable width pulses, reminiscent of a pulse wave with pulse width modulation.

#### RUNG (Rungler) output

This output is also bipolar varying between +/-5V. It is a stepped voltage signal with 8 possible values created by the digital to analog converter in the Rungler.

### Normalled CV level options
The original release of the Benjolin Oscillator had CV1, CV2, and Clock normalled values that were 20% of what they should have been. This bug has been fixed, but just in case there are existing patches that depended on the original normalled values, there is a module context menu option "Original release normalled values" that uses the old values when enabled. Patches with the Benjolin Oscillator that were created using the original release will default to having this option enabled.

### Benjolin Expanders
There are context menu options to add [Benjolin Volts Expander](BenjolinVoltsExpander.md) and/or [Benjolin Gates Expander](BenjolinGatesExpander.md) modules that give the user direct access to the underlying Rungler shift register.

### Patching a Complete Benjolin
A minimal complete Benjolin can be patched simply by pairing the Benjolin Oscillator with a resonant filter with good ping characteristics. The Venom Multimode Filter works extremely well. Simply patch the PWM output to the filter input, and the Rungler output to the filter cuttoff input.

A Benjolin should not self oscillate unless given feedback from the filter band pass output. So ideally the cutoff frequency and resonance amount should be constrained so as to prevent self oscillation. Other things to consider are a crossfade module to allow a mix of PWM and external CV (or self patched CV) as input to the filter. Also a mixer would be good to allow a mix of external (or self patched) CV and Rungler input to the Cutoff frequency.

The patch below closely emulates the features of the Benjolin version 2 from After Later Audio. A version of the patch wired up to show a fun example sound is available at https://patchstorage.com/venom-2-14-3-all-venom-benjolin/.

![Complete Benjolin patch example image](AllVenomBenjolin.png)

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V when the Benjolin Oscillator is bypassed.
