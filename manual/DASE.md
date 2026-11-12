A novel Attack/Decay envelope generator that functions as an unusual VCA and wave shaper via audio rate modulation of the envelope shape. It also has internal modulation that can yield delay or tremolo type effects.

DASE has two active components that provide all of the functionality.

The primary component is an Attack Decay envelope. You specify the duration of the envelope, as well as the percentage of time spent in the attack stage. The decay stage is `100% - Attack%`. The main input modulates the Attack percentage at audio rates, causing the envelope shape to oscillate at the frequency of the input. The closer the phase is to the base envelope peak, the louder the output. Because the envelope is phase driven, the modulation does not alter the overall length of the envelope. The input has an attenuator to control the overall volume.

The output is typically AC coupled to give a bipolar audio output. The output waveform is affected both by the input waveform, as well as the current phase of the envelope, providing continuous timbre changes throughout the length of the envelope. The output has a shape control that defines the response curve, from concave up, to linear, to concave down. This further shapes the oscillation output, as well as the overall volume envelope shape.

The second DASE component is an internal low frequency oscillator that also modulates the envelope attack percentage. The LFO waveform can vary from a descending ramp, to triangle, to ascending ramp. There is control over modulation depth. The effect of the modulation can sound like a cross between delay and reverb with repeating distinct attacks, or like tremolo, depending on the rate of modulation, LFO shape, and depth. Not all LFO modulation sounds good, and configuration is not always intuitive, but with experimentation you can get amazing results.

Most of the inputs are fully polyphonic. The output polyphonic channel count is the maximum channel count found across all polyphonic inputs. Monophonic inputs are replicated to match the output channel count. Polyphonic inputs with fewer channels use constant 0V for missing channels. All of the LFO modulation inputs are monophonic.

All controls have both a medium size knob to set the base value, plus a CV input with a small attenuverter for modulation. The attenuated CV is always summed with the base value to establish the effective modulated value.

All inputs can be modulated at audio rates.

### Upper Envelope controls
All envelope inputs are polyphonic.

#### ENVELOPE LENGTH
Controls the total duration of an envelope. The knob ranges from 0.01325 to 32 seconds, with the default noon value at 1 second. The attenuated CV is exponential. Each positive volt doubles the time. Each negative volt halves the time. The final effective envelope length is clamped to a value between 0.01325 and 32 seconds.

#### ENVELOPE ATTACK
Controls the overall shape of the AD envelope. The knob shows the percentage of time devoted to the attack phase. The decay percentage is simply `100% - attack%`. A value of 0% yields a descending ramp, 50% a triangle, and 100% an ascending ramp. The attenuated CV is scaled at 10% per volt. The final effective attack percentage is clamped to a value between 0% and 100%.

#### INPUT LEVEL
Controls the overall output volume by attenuating the audio input to control the depth of envelope shape modulation. The attenuated CV is scaled at 10% per volt. The final effective level is clamped to a value between 0% and 100%.

#### OUTPUT RESPONSE
Controls the shape of the envelope attack and decay stages. Fully counter-clockwise is -1 representing maximal concave down. The default noon value of 0 is linear. Fully clockwise is 1 representing maximal concave up. The attenuated CV is scaled at 0.1 per volt. The final effective value is clamped to a value between -1 and 1.

### Middle Configuration buttons

#### RTrig (Retrigger) button
Controls how the envelope is retriggered.
- **From current** ***(Yellow, default)*** - The attack begins at the output level at the time of retrigger. The starting phase is computed from the starting level.
- **From 0** ***(light blue)*** - The envelope always restarts at phase 0, level 0.
- **Off** ***(dark gray)*** - Retriggering is disabled. All triggers are ignored until the envelope has completed.

#### Sync button
Controls whether the internal LFO is free-running and in phase across all polyphonic channels, or if each polyphonic channel gets its own LFO that is reset at the start of each envelope.
- **OFF** ***(dark gray, default)*** - There is a single free-running LFO that applies to all channels.
- **ON** ***(yellow)*** - Each channel has its own LFO that is reset each time the channel is triggered.

#### Over (Oversample) button
Controls the level of oversampling used to mitigate digital aliasing that may be introduced by the many non-linear transformations. This is typically not needed, but available just in case.
- **Off** ***(dark gray, default)***
- **x2** ***(yellow)***
- **x4** ***(green)***
- **x8** ***(light blue)***
- **x16** ***(dark blue)***
- **x32** ***(purple)***

#### DC output button
Controls whether the final output is AC or DC coupled
- **Off** ***(dark gray, default)*** - the bipolar output is AC coupled
- **On** ***(yellow)*** - the unipolar output is DC coupled.

### Lower LFO modulation controls
All LFO modulation inputs are monophonic. The LFO creates repeated attacks at low rates, and tremolo at high rates.

#### REPEAT RATE
Controls the frequency of the internal LFO. The knob ranges from 7.5 BPM to 1920 BPM, with the default noon value at 120 BPM (2 Hz). The attenuated CV is scaled to be V/Oct.

#### REPEAT LEVEL
Controls the depth of the LFO modulation. The knob ranges from -1 to 1, with the default noon value at 0. The attenuated CV is scaled at 0.1 per volt. The final effective level is clamped to a value between -1 and 1.

#### REPEAT ATTACK
Controls the shape of the LFO. The knob shows the percentage of time devoted to the attack phase. The decay percentage is simply `100% - attack%`. A value of 0% yields a descending ramp, 50% a triangle, and 100% an ascending ramp. The attenuated CV is scaled at 10% per volt. The final effective repeat attack percentage is clamped to a value between 0% and 100%.

### Bottom main IO ports
All IO ports are polyphonic.

#### TRIG (Trigger) input
The rising edge of a gate at this input triggers (or retriggers) the envelope. It is a Schmitt trigger that goes high at 2V and low at 0.2V.

#### IN input
This is the audio input for the VCA functionality.

#### OUT output
This is the final output for the VCA functionality.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The output is constant monophonic 0V when DASE is bypassed.
