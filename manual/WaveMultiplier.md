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

Here Omri Cohen gives some sound design ideas using the Wave Multiplier:

https://www.youtube.com/watch?v=k-GukBcUE0s

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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### Polyphony

Wave Multiplier is fully polyphonic. In general, the number of output channels is computed as the maximum channel count found across all inputs.

Inputs that match the output polyphony behave as expected. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels get constant 0V for any missing channels.

The LFOs are special. If the Master LFO V/Oct input is monophonic (or unpatched), then the LFO outputs are monophonic, regardless wether there are any polyphonic inputs elsewhere. But if the V/Oct is polyphonic, then the LFO output channel count matches the channel count for the rest of the module, and missing LFO V/Oct channels are treated as constant 0V.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
If Wave Multiplier is bypassed, then all channels of the wave input are passed through unchanged to the shifted wave mix output. All other outputs are monophonic constant 0 volts.
