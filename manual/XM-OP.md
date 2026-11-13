Polyphonic synth voice with selectable waveform, modulation and feedback types (linear through-zero frequency, phase, ring, amplitude), and integer frequency ratios.

XM-OP includes an ADSR (Attack, Decay, Sustain, Release) envelope generator, audio rate VCO, and VCA. The VCO output is always processed by the VCA.

XM-OP is very much inspired by the Bogaudio FM-OP, offering most of the same features, but with the following differences/enhancements:
- VCO waveform can be sine, triangle, square, or saw rather than being fixed at sine
- Level VCA response is linear by default, with a context menu option for exponential (the FM-OP default)
- Envelope generator has a stage shape knob that cross-fades between linear and curved
  - Rising curves are concave down and falling curves are convcave up
- Each ADSR stage has an attenuverter for a shared modulation input rather than only having CV control over sustain
  - Attenuated CV values are summed with the base knob values
- Frequency ratio is specified by separate integer multiplier (numerator) and divisor (denominator) controls rather than a single continuous ratio control
  - This is very conveninent for establishing a wide range of musical ratios
- The ratio multiplier, divisor, and detune can be CV controlled via three attenuverters with a shared CV input
- VCO external modulation and VCO feedback have selectable modulation types rather than being fixed at phase modulation
  - Through zero linear frequency modulation (AC or DC coupled)
  - Phase modulation (called through zero linear FM by Bogaudio)
  - Ring modulation
  - Amplitude modulation with options
    - raw modulation input
    - rectified modulation input
    - modulation input offset by 5V
- Feedback is pre level by default , with a context menu option to use post level feedback (FM-OP always uses post level feedback)
- Envelope may be normal or inverted when applied to level, modulation depth, and/or feedback depth
- Level, mod depth, and feedback depth knob values (optionally attenuated by envelope) are summed with independent CV inputs with attenuverters
- The envelope (normal or inverted) is available as a separate output
- A configurable trigger input that can either sync the VCO, retrigger the envelope during decay or sustain, or both

Omri Cohen produced this video with lovely XM-OP voice examples that introduce many of the module's features.

https://www.youtube.com/watch?v=0dWUHg-dJ_0

### WAVE button
Selects the waveform for the VCO
- **SIN** (default) sine
- **TRI** triangle
- **SQR** square
- **SAW**

### XMOD (variable modulation type) button
Selects the modulation mode used for the XMOD input
- **FM AC** (default) AC coupled linear through-zero frequency modulation for audio CV only
- **FM DC** DC coupled linear through-zero frequency modulation for audio or LFO CV
- **PM** Phase modulation
- **RM** Ring modulation - the waveform is multiplied by the XMOD using a 4 quadrant VCA
- **AM** Amplitude modulation - the waveform is multiplied by the XMOD using a 2 quadrant VCA, so negative XMOD values are treated as 0
- **AM RECT** Amplitude modulation with the XMOD fully rectified to positive values before multiplying
- **AM OFF** Amplitude modulation with the XMOD offset +5V before multiplying

Note that the RM and various AM assume the XMOD input is bipolar +/- 5V. The modulation is scaled such that the result is also bipolar +/- 5V (before hitting the VCA).

### FDBK (feedback type) button
Selects the modulation type used for feedback. The options are the same as for the XMOD type.

### Envelope generator general behavior
Upon receipt of a high gate, the generator starts the attack stage and rises from 0% to 100% as long as the gate remains high. Once 100% is reached, it proceeds to the decay stage.

The decay stage falls from 100% to the sustain level as long as the gate remains high. Once the sustain level is reached it proceeds to the sustain stage.

The sustain stage maintains the sustain level as long as the gate remains high.

The envelope immediately jumps to the release stage whenever the gate goes low. This could happen during the attack, decay, or sustain stage. The release stage falls from the current value back to 0%.

The generator can be retriggered during the decay and release stages, in which case the attack stage is re-started from the current level instead of 0%.

### CURVE knob
Establishes the shape of the envelope attack, decay, and release stages. Fully counter-clockwise is linear, and fully clockwise has the most severe curvature. The knob is scaled to show the amount of curvature.

Curves are concave down for the attack phase, and concave up for the decay and release phases.

Changing the curvature does not change the stage times.

### ATK (envelope attack time) knob
Establishes the time it takes the attack stage to rise from 0% to 100%. The knob range is 0.98 msec to 11.3 sec.

If the envelope is retriggered, then the attack stage can start above 0%, in which case the attack time is shortened proportionally.

### DEC (envelope decay time) knob
Establishes the time it takes the decay stage to fall from 100% down to the sustain level. The knob range is 0.98 msec to 11.3 sec.

### SUS (envelope sustain level) knob
Establishes the level of the sustain stage. The knob range is 0% to 100%.

### REL (envelope release time) knob
Establishes the time it takes the release stage to fall from the sustain level to 0%. The knob range is 0.98 msec to 11.3 sec.

If the gate goes low before the sustain stage is reached, then the release start level will not be the sustain level. If the release start is below the sustain level, then the release time will be decreased proportionally. If the release start is above the sustain level, then the release time will be for the release start down to 0%.

### SMOD (envelope stage modulation) input
This is a shared input that can be used to modulate any of the envelope stages. Each stage has its own attenuverter to attenuate and/or invert the SMOD CV. The attenuated CV is summed with the knob value.

The attack, decay, and release stages scale the CV such that for each positive volt of CV, the time is doubled, and for each negative volt the time is halved. The stage times can be modulated beyond the knob values. The absolute minimum stage time is 0.24 msec, and the maximum is 181 seconds.

The sustain level CV is scaled at 10% per volt, and the effective sustain level is clamped between 0% and 100%.

### VCO frequency ratio
XM-OP is intended to be used as a modulation operator, where one XM-OP modulates another. When performing modulation, the most musical results occur when there is an integral ratio relationship between the frequencies of the two operators. There are three controls to establish this ratio. XM-OP also has a V/Oct input where 0V always represents 261.63 Hz, or C4. There isn't any general tuning knob or octave knob. If there were, then it would disturb the ratio relationships.

### MULT (frequency multiplier) knob
Establishes the numerator of the frequency ratio. The knob ranges from 1 to 64.

### DIV (frequency divisor) knob
Establishes the denominator of the frequency ratio. The knob ranges from 1 to 64.

### QUANT (quantize ratio) button
Controls whether the multiplier and division values are quantized to integer values or not. The default is On (quantize enabled).

### DTUNE (detune) knob
Allows you to detune the ratio from the perfect integral ratio. The knob ranges from -100 cents to 100 cents.

### RMOD (frquency ratio modulation) input
This is a shared input that can be used to modulate any of the frequency ratio parameters. MULT, DIV, and DTUNE each have their own attenuverter to attenuate and/or invert the RMOD CV. The attenuated CV is summed with the knob value.

MULT and DIV CV are scaled at 1 integer per 0.1 volt. The Mult and Div values cannot be modulated below 1.

DTUNE CV is scaled at 10 cents per volt. The CV can modulate the detune amount beyond the knob limits.

### LEVEL (VCA level) knob and CV input
The knob establishes the base level of the internal VCA output. It ranges from 0% to 100%.

By default the Level VCA responds linearly. An exponential response is available via the context menu "Exponential level response" option.

See [Level and Depth Modulation](#level-and-depth-modulation) for information on the associated Env button, CV input, and attenuverter.

### DEPTH (XMOD modulation depth) knob and CV input
The knob and CV establish the depth of the XMod modulation. It ranges from -100% to 100%. The knob and CV value are summed. The type of modulation is controlled by the square **XMOD** button at the top.

The FM and PM  depth scaling is fairly tame. A "High XM depth for FM & PM" context menu option exists that multiplies the depth by a factor of 5. This only applies to XM frequency and phase modulation.

See [Level and Depth Modulation](#level-and-depth-modulation) for information on the associated Env button, CV input, and attenuverter.

### FDBK (feedback modulation depth) knob and CV input
The knob establishes the depth of the feedback modulation. It ranges from -100% to 100%. The type of modulation is controlled by the square **FDBK** button at the top.

By default the feedback source is the raw internal waveform, before any Level is applied. The context menu "Post level feedback" option uses the internal waveform after the Level has been applied.

See [Level and Depth Modulation](#level-and-depth-modulation) for information on the associated Env button, CV input, and attenuverter.

### Level and Depth modulation

The Level, Depth, and Feedback controls each have an associated Envelope mode button above, and CV input below with attenuverter.

#### CV input and attenuverter
The CV input is scaled at 10% per volt. The value is attenuated and/or inverted by the small attenuverter knob, and the effective CV value is summed with the parent knob value. The final value is clamped to values between 0% and 100% for the Level. The final value is unconstrained for the XMod and Feedback depths.

#### ENV (envelope mode) button
Controls how the internal envelope further modulates the control
- **Off** (dark gray, default) The envelope is not used
- **Knob** (yellow) The envelope attenuates the larger control knob value
- **CV** (blue) The envelope attenuates the effective CV value
- **Both** (green) The envelope attenuates both the larger control knob and the effective CV value
- **Knob Inverted** (red) The inverted envelope (computed as 100% - EnvelopeValue) attenuates the larger control knob value
- **CV Inverted** (purple) The inverted envelope (computed as 100% - EnvelopeValue) attenuates the effective CV value
- **Both Inverted** (orange) The inverted envelope (computed as 100% - EnvelopeValue) attenuates both the larger control knob and the effective CV value.

### OVER (oversample amount) button
Modulation can introduce unwanted inharmonic audio aliasing that can be mitigated by oversampling. The OVER button provides for the following oversampling levels
- **Off** (dark gray)
- **x2** (yellow)
- **x4** (green, default)
- **x8** (light blue)
- **x16** (dark blue)
- **x32** (purple)

Note that only the XMOD input is upsampled to the oversample rate. The other inputs can be driven at audio rates, but they are not upsampled.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

### GATE (envelope gate) input
The internal envelope is triggered on the leading edge of a gate. The envelope proceeds through the attack, decay, and sustain stages for as long as the gate remains high. The gate may also sync the VCO depending on how the SYNC/RTRG input is configured.

### SYNC/RTRG (VCO sync or envelope retrigger) input and mode button
This trigger input has different behavior depending on the small mode button below the label
- **VCO sync** (blue, default) - The VCO is hard synced
- **Envelope retrigger and VCO sync** (green) - The envelope can be retriggered during the decay and sustain stages while the gate remains high, at which point the VCO is also hard synced. The gate input also hard syncs the VCO.
- **Envelope retrigger, No VCO sync** (yellow) - The envelope can be retriggered during the decay and sustain stages while the gate remains high.

### V/OCT (volt per octave) input
Establishes the base frequency of the VCO before applying any frequency ratio, XMOD or feedback modulation. 0V represents 261.63 Hz, or C4.

### XMOD (variable modulation) input
This is the CV that modulates the VCO, with the type of modulation controled by the square XMOD button at the top. Typically audio signals are used.

### ENV (envelope) output
The 0-10V envelope is output here. The mode of the output is controlled by the small button beside the label.
- **Normal** (dark gray, default) - The envelope starts at 0V and rises to 10V.
- **Inverted** (red) - The envelope is 10V at rest and falls to 0V during the attack stage.

### OUT output
The output of the VCA is output here. This is the modulated VCO output, after being attenuated by the VCA.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If XM-OP is bypassed then all outputs are constant monophonic 0V.
