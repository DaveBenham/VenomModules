# Mixers

## CROSS FADE 3D
![Cross Fade 3D module image](doc/CrossFade3D.png)  
Eight inputs in three dimensional space are cross faded to a single output. The inputs are placed at the vertices of a virtual cube. X, Y, and Z controls independently cross fade between inputs on opposite faces of the cube. Each fader functions linearly in amplitude, ranging from 0% to 100%. The orthogonal faders are multiplicative, such that when all three controls are at an extreme, then 100% of the output comes from a single input. When all three controls are at 50% then each input contributes 12.5% to the output. A final Level control can further attenuate the final output.

### Polyphony
Every input and output is fully polyphonic. The output channel count is the maximum channel count found across all inputs. Monophonic inputs are replicated to match the final output channel count. Polyphonic inputs with fewer channels use constant 0V for any missing channels.

### INPUTS
The faceplate has a perspective view of a cube with a polyphonic input at each of the eight vertices. The inputs are not individually labeled on the faceplate, but when you hover over an input a name is displayed that identifies whether the input is left or right, bottom or top, and front or back.

#### Single polyphonic input alternative
If only the Bottom Left Front input is patched, then Cross Fade 3D treats the input as an 8 channel polyphonic signal where each channel represents a monophonic input for one of the cube vertices. Missing channels are assigned constant 0V. The 8 channels are assigned as follows.
- 1 = Bottom left front
- 2 = Bottom right front
- 3 = Top left front
- 4 = Top right front
- 5 = Bottom left back
- 6 = Bottom right back
- 7 = Top left back
- 8 = Top right back

### X, Y, Z fader controls and CV inputs
X controls the left to right ratio, and is measured as percent right.  
Y controls the bottom to top ratio, and is measured as percent top.  
Z controls the front to back ratio, and is measured as percent back.  

Each fader control ranges from 0% to 100%, with the default 50% at noon.

Each dimension has a bipolar CV input and dedicated attenuator. The CV is scaled at 10% per volt by default. The CV is summed with the control value and clamped to a value between 0% and 100%. With a dimension at 50% and the CV attenuator at 100%, a +/- 5V bipolar can modulate a dimension from one extreme to the other.

A module context menu option is available to scale the CV at +/- 200% instead of +/- 100%. This enables a simple bipolar +/- 5V sine or triangle modulator to transition to one extreme and hold, before transitioning in the other direction.

If you prefer to work with spherical coordinates, then the [Sphere To XYZ module](Modulation.md#sphere-to-xyz) is available to convert r, theta, phi spherical coordinates into X, Y, Z cartesian coordinates. 

### MONO OUTPUT button
By default, polyphonic channels are preserved at the output. If the Mono Output button is enabled, then polyphonic output channels are summed to a monophonic output signal.

### LEVEL control
The final output level can be attenuated with the Level control. This may be especially useful when working with polyphonic inputs that are summed to a mono output.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The output is constant monophonic 0V if the module is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## MIX 4
![Mix 4 module image](doc/Mix4.png)  
A compact polyphonic mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. Module functionality can be extended by a set of [Mix Expanders](#mix-expanders).

### General Operation
There are four numbered inputs, each of which can be attenuated, inverted, and/or amplified by a level knob. The level knobs can be configured to different scales. The modulated inputs are then summed to create a mix that can also be attenuated, inverted, and/or amplified by a mix level knob. Finally there are options to hard or soft clip the mix and/or remove DC offset, before sending the final mix to the Mix output. Oversampling is available for soft clipping to control any aliasing that might otherwise be introduced.

### LEVEL Knobs 1 - 4
These knobs control the attenuation, inversion, or amplification of the input channels. The exact behavior of the knobs depends on the setting of the Mode button.

### IN Channel Inputs 1 - 4
The channel inputs that are modulated by the Level knob. Each input may be effectively normalled to a constant CV value, depending on the current Level Mode.

### Mix Level knob
This knob controls the attenuation, inversion, or amplification of the final mix, consisting of the sum of the modulated channel inputs. The exact behavior of the Mix Level knob depends on the setting of the Level Mode button.

### MIX output
The final mix is output here. The final mix output may be clipped and/or have DC offset removed, depending on the settings of the Clip and DC buttons.

### Polyphony
The number of output polyphonic channels for the final mix is normally the maximum channel count found across all four channel inputs. Monophonic inputs are replicated to match the output polyphony. But polyphonic inputs with fewer channels than in the output send 0V for any missing channels.

There is one major exception when the Level Mode is set to poly sum. In this mode, polyphonic input signals are summed to a monophonic signal before any modulation.

### M (Level Mode) button
The color coded mode button determines how the 4 channel and the mix knobs behave. The mode button cycles through 5 possible modes. The button context menu allows direct selection of any mode. Each mode is labeled for audio or CV according to typical usage, but all modes can be applied to both audio and CV.
- **Unipolar dB (audio x2)** (pink - default): Each knob ranges from -inf dB (0V) to +6.0206 dB (x2 amplification), with the default of 0 dB (unity) at noon. Unpatched channel inputs are normalled to 0V.
- **Unipolar poly sum dB (audio x2)** (purple): Same as Unipolar audio dB, except polyphonic inputs are summed to a single monophonic signal before being attenuated or amplified by the input level knob. Unpatched channel inputs are normalled to 0V.
- **Bipolar % (CV)** (green): Each input level knob ranges from -100% (inversion) to 100% (unity) if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpatched channel inputs are normalled to 10V. Each input level knob defaults to 0% (or 0V) at noon. The Mix knob always ranges from -100% to 100%, with a default of 100%.
- **Bipolar x2 (CV)** (light blue): Each input level knob ranges from -2x to 2x if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpatched channel inputs are normalled to 5V. Each input level knob defaults to 0x (or 0V) at noon. The Mix knob always ranges from -2x to 2x, with a default of 1x (unity).
- **Bipolar x10 (CV)** (dark blue): Each input level knob ranges from -10x to 10x if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpateched channel inputs are normalled to 1V. Each input level knob defaults to 0% (or 0V) at noon. The Mix knob always ranges from -10x to 10x, with a default of 1x (unity).

### D (DC Block) button
The color coded DC block button determines when (or if) a high pass filter is applied to the final mix to remove any DC offset. DC offset removal always occurs after the final mix is attenuated (or amplified) by the Mix level knob.
- **Off** (dark gray - default): DC offset is not removed.
- **Before clipping** (yellow): DC offset is removed prior to any clipping. Note that subsequent clipping may re-introduce some DC offset.
- **Before and after clipping** (green): DC offset is removed both before and after any clipping. Note that only one DC offset removal is performed if clipping is not applied.
- **After clipping** (light blue): DC offset is removed after any clipping.

The last three DC offset options give identical results when no clipping is applied.

### C (Clip) button
The color coded clip button determines how (or if) the final output is clipped.
- **Off** (dark gray - default)
- **Hard post-level at 10V** (white)
- **Soft post-level at 10V** (yellow)
- **Soft oversampled post-level at 10V** (orange)
- **Hard pre-level at 10V** (green)
- **Soft pre-level at 10V** (light blue)
- **Soft oversampled pre-level at 10V** (dark blue)
- **Saturate (Soft oversampled post-level at 6V)** (purple)

If clipping is off then there is no limit to the output voltage.

Hard clipping can produce significant aliasing if applied to audio signals.

Soft clipping provides tanh saturation. At moderate saturation levels there is little to no audible aliasing. But very hot signals can still lead to signfcant aliasing.

Soft oversampled clipping also provides saturation, but aliasing is greatly reduced.

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.
   
### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The MIX output is monophonic 0V if MIX 4 is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## MIX 4 STEREO
![Mix 4 Stereo module image](doc/Mix4Stereo.png)  
A stereo compact polyphonic mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. Module functionality can be extended by a set of [Mix Expanders](#mix-expanders).

Mix 4 Stereo is identical to [Mix 4](#mix-4) except each of the inputs and outputs is doubled to support left and right channels so as to support stereo signals. A single input level knob controls each stereo input pair, and a single Mix level knob controls the stereo output pair.

Each right input is normaled to the corresponding left input. When in CV mode, each input level knob produces constant CV only if both the left and right input are unpatched.

The number of polyphonic output channels is determined by the maximum polyphonic channel count found across all inputs. The output channel count always matches for the left and right Mix outputs.

When Mix 4 Stereo is bypassed, both the left and right Mix outputs are monophonic constant CV.

All other behaviors are the same as for Mix 4.

[Return to Table Of Contents](/README.md.md#venom)


## MIX EXPANDERS
![Mix Offset Expander module image](doc/MixOffset.png) &nbsp;![Mix Mute Expander module image](doc/MixMute.png) &nbsp;![Mix Solo Expander module image](doc/MixSolo.png) &nbsp;![Mix Fade Expander module image](doc/MixFade.png) &nbsp;![Mix Fade2 Expander module image](doc/MixFade2.png) &nbsp;![Mix Pan Expander module image](doc/MixPan.png) &nbsp;![Mix Send Expander module image](doc/MixSend.png)  
A collection of expander modules that extend the functionality of the four Mix modules: [Mix 4](#mix-4), [Mix 4 Stereo](#mix-4-stereo), [VCA Mix 4](#vca-mix-4), and [VCA Mix 4 Stereo](#vca-mix-4-stereo).

Mix expanders must be placed to the right of the main mix module. Multiple expanders can be used for one mix module as long as they form a contiguous chain to the right. Each expander has an LED in the upper left that glows yellow if successfully connected to a mix module.

A main mix module may have up to 16 expanders. However, any combination of Mix Solo, Mix Mute, Mix Fade, and Mix Fade 2 counts as only one expander toward the 16 expander limit. Also any Mix Offset does not count toward the 16 expander limit.

Bypassing an expander disables that expander without disrupting expanders to the right.

All expanders except Offset are processed after the main Mix input channel Levels are applied, and before the main final Mix level is applied. When used with VCA Mix 4 or VCA Mix 4 Stereo, the expanders are processed after each channel has already been sent to the VCA channel output. Again, the Offset expander is an exception.

The order of operations generally proceeds from left to right. The order typically does not affect the end result unless Aux Send modules are used. However, some expanders have restrictions on where they are placed, and/or how many can be used for a single mix module. Refer to each expander for details.

CV inputs and outputs are generally all monophonic, with the effect applied equally to all polyphonic channels in the main mix module. The two exceptions are the Aux Send module and Mix Pan module, both of which fully support polyphonic cables. Extra poly channels beyond what is seen in the parent module are ignored.

Venom expanders are written so that communication between the parent module and the expander does not add any sample delays.

Some of the expanders have hidden options that are only available in the main mix module context menu. These expander related menu options only appear when the relevant expander is connected to the mix module.

#### Soft mute/solo &nbsp;(Mute and/or Solo and/or Send expander)
 * If enabled (default), then mute/solo transitions are slewed to take 25 msec (or more if fades are applied).
 * If disabled, then mute/solo transitions are immediate (unless fades are applied)

#### Mute/solo CV toggles on/off &nbsp;(Mute expander)
 * If disabled (default), then Mute CV and Solo CV function as a gate
 * If enabled, then Mute CV and Solo CV function as a toggle

#### Mono input pan law &nbsp;(Pan expander)
Controls how left and right channels are attenuated/amplified as a mono input is panned. This affects the perceived loudness as a signal is panned left and right versus center.
 * **0 dB (linear: center overpowered)**  
   While panning left, the left gain is held constant while the right is attenuated. While panning right, the right gain is held constant while the left is attenuated. Mono signals sound softer when panned left or right compared to when panned center.
 * **+1.5 dB side (compromise: center overpowered)**  
   When panning left, the left channel is amplified slightly to +1.5 dB when panned full left. Likewise when panning right the right channel is amplified slightly. Mono signals still sound softer when panned left or right, but to a lesser degree.
 * **+3 dB side (default - equal power)**  
   While panning left, the left channel is amplified until it reaches +3 dB when panned full left. The right channel is similarly amplified when panning right. Mono signal loudness is perceived to be constant regardless whether panned left, center, or right.
 * **+4.5 dB side (compromise: side overpowered)**  
   When panning left, the left channel is amplified until it reaches +4.5 dB when panned hard left. The right channel is similarly amplified when panning right. Mono signals sound slightly louder when panned left or right compared to center.
 * **+6 dB side (linear: side overpowered)**  
   When panning left or right, one side is amplifed and the other attenuated in equal amounts, such that the net gain is always 1. Mono signals sound louder when panned left or right compared to center.
 * **-1.5 dB center (compromise: center overpowered)**  
   Same as +1.5 dB side except the center is attenuated rather than amplify the side.
 * **-3 dB center (equeal power)**  
   Same as +3 dB side except the center is attenuated rather than amplify the side.
 * **-4.5 dB center (compromise: side overpowered)**  
   Same as +4.5 dB side except the center is attenuated rather than amplify the side.
 * **-6 dB center (linear: side overpowered)**  
   Same as +6 dB side except the center is attenuated rather than amplify the side.

#### Stereo input pan law &nbsp;(Pan expander)
Controls how left and right channels are attenuated/amplified as a stereo input is panned. All of the mono options are available, plus the following
 * **True panning (transfer content)**  
   Right channel input is mixed in with the left channel while panning left, and vice versa while panning right.
 * **Follow mono law (default)**  
   The mono pan law setting is used for stereo inputs as well

### MIX OFFSET EXPANDER
Gives the ability to add constant offset voltages immediately before and/or after a level gain is applied on the main mixer. Offsets are always the first expander to be applied, and so the Offset expander must be adjacent to the main mix module.

Offsets are available for the individual numbered input channels, as well as the final mix output.

The Pre offsets are applied immediately before each level gain.

The Post offsets are applied immediately after each level gain.

Unlike other expanders, offsets are included in VCA channel outputs when used with VCA Mix or VCA Mix Stereo.

Offsets are often used for converting unipolar signals to bipolar, or vice-versa.

Only one Offset expander can be used per mix module.

### MIX MUTE EXPANDER

Provides buttons and CV inputs to mute any of the four numbered channels, as well as the final mix. The channel is muted whenever the button is glowing bright red.

The CV inputs function as Schmitt triggers, switching to high when the voltage rises to 1 volt or more, and switching to low when dropping to 0.1 volt or less.

A context menu on the main Mix module determines whether the CV inputs function as gates or triggers. By default the CV functions as a gate, meaning the mute is turned on when the CV goes high, and turns off when the CV goes low. If configured as a trigger, then the mute state toggles on or off each time the CV transitions to high.

Manual button presses can override the CV inputs - every button press is guaranteed to toggle the mute state.

Mute states can be overriden by Solo expander buttons.

Since Solo and Mute are applied at the same time, they must be adjacent if both expanders are used. It does not matter which appears first.

Only one Mute expander can be used per mix module.

### MIX SOLO EXPANDER

Provides buttons and CV inputs to solo one or more of the numbered mix channels. If none of the solo buttons are lit, then the expander has no effect. If at least one solo button is lit (bright green), then all channels that are not lit are muted. Numbered channels on a Mute expander are ignored if at least one solo button is lit.

The CV inputs function as Schmitt triggers, switching to high when the voltage rises to 1 volt or more, and switching to low when dropping to 0.1 volt or less.

A context menu on the main Mix module determines whether the CV inputs function as gates or triggers. By default the CV functions as a gate, meaning the solo is turned on when the CV goes high, and turns off when the CV goes low. If configured as a trigger, then the solo state toggles on or off each time the CV transitions to high.

Manual button presses can override the CV inputs - every button press is guaranteed to toggle the solo state.

Since Solo and Mute are applied at the same time, they must be adjacent if both expanders are used. It does not matter which appears first.

Only one Solo expander can be used per mix module.

### MIX FADE EXPANDER

Converts mute/unmute transitions from Mute and Solo expanders to timed fade transitions.

A single Time control for each numbered Mix channel controls the fade in (rise) time and the fade out (fall) time. The fade time ranges from 0 to 30 seconds.

Each channel also has a Shape control, with full counterclockwise (-100%) representing exponential, center (0) representing linear, and full clockwise (100%) representing logarithmic. Points in between are a proportional blend of linear and logorithmic/exponential.

The fade level outputs provide CV representing the current fade level of each channel. The output is 0 V when fully muted, and 10 V when fully unmuted.

Fade is actually an expander for the Mute and Solo expanders, and thus must appear adjacent and to the right of either a Mute or Solo expander.

Only one Fade expander can be used per mix module. Fade and Fade 2 are mutually exclusive. Using Fade precludes the use of Fade 2.

### MIX FADE 2 EXPANDER

Fade 2 is identical to Fade except it gives independent control over the fade rise and fall times.

Each Time control from Fade is replaced by a pair of Rise and Fall controls on Fade 2.

Only one Fade 2 expander can be used per mix module. Fade 2 and Fade are mutually exclusive. Using Fade 2 precludes the use of Fade.

### MIX PAN EXPANDER

Allows panning of Mix input channels left and right via numbered knobs and inputs.

Each knob ranges from -1 (full counterclockwise) for hard left, to 0 (noon) for center, to +1 (full clockwise) for hard right.

Each CV input has an associated CV attenuverter. The CV is added to the knob value to determine the final pan level. Bipolar CV is expected, with -5V representing hard left and +5V representing hard right (assuming the knob is center panned). So -10V is guaranteed to result in pan hard left even if the knob is panned hard right. Similarly, +10V guarantees pan hard right even if the knob is panned hard left.

The CV inputs support polyphonic cables.

Pan is only available to stereo mix modules Mix 4 Stereo and VCA Mix 4 Stereo. Only one Pan module can be used per mix module.

### MIX AUX SEND EXPANDER

Provides an auxilliary mix of the four mix inputs that can be sent to the Send output(s), and optionally returned to the Return input(s)

Each numbered knob can attenuate the channel to between 0% and 100%, before the channels are mixed and sent to the Send output(s).

The Return input(s) are attenuated by the Return knob before being mixed in with the final Mix module mix. The return is mixed in prior to applying the final Level gain at the main Mix module.

Both Send oututs and Return inputs support polyphonic cables.

The Mute button sets the Send output(s) to constant 0V when lit.

Both Left and Right Send and Return inputs and outputs are used when the main Mix module is stereo (Mix 4 Stereo, or VCA Mix 4 Stereo).

If the main Mix module is not stereo (Mix 4, or VCA Mix 4), then the Right Send output is constant monophonic 0V, and any right Return is ignored.

The position of the Send module in a chain of Mix expanders is important. Expanders to the left of the Send expander affect the Send output. Expanders to the right of the Send expander do not affect the Send output.

At most 16 Send modules can be used with a single mix module. The maximum is fewer than 16 if other expander types are used in addition to the Send modules.

The Return inputs have a Chain button that changes the behavior of the expander for use with chained mixers. If enabled, then the Return knob is disabled, and the Left Return and Right Return inputs receive the chained Send from a prior Aux Send expander. For example, suppose you have three VCA Mix 4 Stereo modules chained together named Mix1, Mix2, and Mix3, and each has an Aux Send module, named Send1, Send2, and Send3. Leave the chain option off on Send1, and enable chain on Send2 and Send3. Patch the Send1 Send outputs to Send2 Return (Chain) inputs, and Send2 Send outputs to Send3 Return (Chain) inputs. Finally patch the Send3 Send outputs to the effect module inputs, and the effect outputs to the Send1 Return inputs. The Send1 Return knob controls the return level, and the Send3 Mute button can be used to mute the entire send chain.
![Chained Send example](doc/ChainedSend.PNG)

[Return to Table Of Contents](/README.md.md#venom)


## POLY FADE
![Poly Fade module image](doc/PolyFade.png)  
Crossfade between channels of a polyphonic signal.

### Overview of basic functionality

A unipolar phasor from 0 to 10V drives the crossfade between the channels of a polyphonic input. The phasor can be the internal LFO, an external Phasor input, or the sum of both. The current phasor voltage determines which channel(s) are playing at that moment. An envelope controls how each channel fades in and out. For an input with N channels, the 10V phasor range is divided into N equal voltage ranges, one for each channel. That range represents 1 width unit. The width of the channel envelopes can vary, and is expressed in the channel width units. An envelope width of 1 means that the channel envelopes abut each other, but never overlap. Widths greater than 1 result in channel envelopes overlapping each other. Widths less than 1 result in gaps between channel envelopes. The shape of the channel envelopes is controlled by Hold, Skew, Rise shape, and Fall shape controls. There are outputs for the net phasor, the polyphonic channel gates, the polyphonic channel envelopes, the polyphonic final output, and the monophonic mix of final outputs. Level control and a VCA can be used to adjust the output volume and/or to apply amplitude modulation effects.

Poly Fade can run with slow LFO rates, or high audio rates, but there is no anti-aliasing applied.

### Upper Section - Envelope and Level control

For each parameter in this section there are two knobs and one monophonic input. The left knob sets the base level, the input provides for CV modulation, and the right knob attenuates and/or inverts the CV. The attenuated CV is always added to the base knob value to establish the final value for the parameter.

Note that this section is labeled assuming that the phasor is going in a forward (ascending channel) direction. When phasing in reverse, the rise is actually the fall, and vice versa.

#### Width
Establishes the overall width of each channel's envelope, expressed as the number of channel slots to occupy. A value of 1 results in continuous sound, but without any overlap of channels. Less than 1 results in moments of silence between channels. Values greater than 1 result in channels overlapping. If the value matches the number of input channels, then each channel will play throughout the entire phasor cycle, but each channel envelope will still have a different phase.

The CV is scaled to 1 channel slot per 0.5V.

The net sum is clamped to a value between 0.0625 to 16. If the value exceeds the number of channels then it is internally truncated to match the channel count.

The default value is 2.

#### Hold
Specifies what percentage of the channel envelope is held at full volume, expressed as a percent.

A value of 100% yields a square wave, regardless what the Skew, Rise, and Fall values are. A value of 0% yields something between a saw and a symmetric triangle. Intermediate values yield some form of trapezoid.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 0%.

#### Skew
Specifies what percentage of the remaining envelope width is dedicated to the rise, with the remainder going to the fall. If the Hold value is 0%, then a 50% skew yields a symmetric triangle with equal rise and fall ramps.

A value of 0% yields a saw wave ramp down with an immediate rise, and a sloped fall.

A value of 100% yields a saw wave ramp up with a sloped rise, and immediate fall.

Again, the concept of rise and fall assumes the phasor is moving in a forward direction.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 50%.

#### Rise and Fall shapes
Specifies the amount of curve applied to the rise and fall ramps using J curves. The values range between -100% and 100%, with 0% being linear. Negative values are similar to exponential, and positive logarithmic, though the math is different.

Both CV inputs are scaled at 10% per volt.

The net sums are clamped to a value between -100% and 100%

The default values are 0% (linear).

#### Level

Specifies the maximum level of the channel and sum outputs, with value ranging from 0% to 100%.

The CV is scaled at 10% per volt.

The net sum is clamped to a value between 0% and 100%.

The default value is 100%.

### Indicator LED lights

The top row of dimly lit small green LEDs indicate which input channels are used. The bright green indicates the starting channel(phase 0). Off LEDs indicate excluded channels.

The bottom row of larger yellow LEDs indicate which channels are currently producing output, with the intensity proportional to the current level of the envelope.

### Lower Section - Phasor control, channel selection, and outputs

#### Rate knob and input
Controls the frequency of the internal LFO phasor. The knob ranges from 0.0079842 Hz to 32.703 Hz (C1). The default is 2.044 Hz.

Note that the ping pong (triangle LFO) rate is actually half the displayed frequency so as to maintain a constant rate of channel switching relative to forward and backward modes. 

The 1V/Oct CV input can drive the rate both lower and higher. The minimum rate is 0.0015 Hz (11.1 minutes per cycle). The maximum rate is 12 kHz, however there is no anti-aliasing. So mid to high audio rates can lead to harsh results with lots of aliasing.

#### Direction button and monophonic input

Controls the shape of the internal phasor, and thus the direction of the fade.

Patched CV input actually sets the value of the button, so any CV is effectively disabled if the button is locked.

There are four possible values
- Forward (right arrow, default) = a rising ramp saw wave = 0V CV
- Backward (left arrow) = a falling ramp saw wave = 1V CV
- Ping Pong (bidirectional arrow) = a triangle wave = 2V CV
- Off (dashed line) = disables (freezes) the internal phasor = 3V CV

CV values are rounded to the closest valid value.

If the direction is Off then by default the internal phasor is reset to 0. The module has a context menu option to disable the "Reset if direction off" so that you can freeze the internal phasor at its current position and then later pick up where you left off.

#### Chan (Channel count) knob and monophonic input

Controls how many of the polyphonic channels participate in the fade. The value can range from 0 to 16.

A value of 0 represents Automatic, meaning all channels appearing at the In input are used.

If the set value is less than the number of input channels, then the unused channels are ignored.

If the set value exceeds the number of input channels, then the missing channels default to constant 0V.

Patched CV input actually sets the value of the knob, so any CV is effectively disabled if the knob is locked. 

CV is scaled at 0.5V per channel, and values are rounded to the nearest valid multiple of 0.5.

#### Start knob and monophonic input

Controls which input channel represents phase 0. It can be any value between 1 and 16.

Patched CV input actually set the value of the knob, so any CV is effectively disabled if the knob is locked.

CV is scaled at 0.5V per channel, with 0.5 representing 1. Values are rounded to the nearest valid multiple of 0.5.

#### Phasor monophonic input

The phasor input is summed with the internal LFO phasor to establish the effective phasor that controls the fade. Bipolar signals can be used. A value of 0 represents phase 0, and the phase increases linearly as the voltage increases until a value of 10V loops back to phase 0. The phase is cyclical, so values above 10V or below 0V effectively wrap around.

#### Phasor input slew rate button (unlabled)
A small color coded button below and to the right of the Phasor input controls slew that can be applied to the phasor input before it is summed with the internal LFO. Slew can be helpful for reducing audio pops created by sudden changes within the incoming phasor signal.
- **Off** (gray - default)
- **3 msec/V** (yellow)
- **6 msec/V** (orange)
- **10 msec/V** (purple)

#### Reset monphonic input

The rising edge of a trigger at this input instantly resets the internal LFO phasor back to phase 0.

#### In polyphonic input

This input represents the polyphonic channels that are crossfaded.

#### Sum monophonic output

This is the sum of the polyphonic crossfaded channel outputs.

#### Phasor monophonic output

This is the effective phasor - the sum (unity mix) of the internal LFO phasor and the phasor input. This is guaranteed to have a unipolar output between 0V and 10V.

#### Gates polyphonic output

This port outputs a high 10V gate for each channel that currently has a non-zero envelope.

#### Envs (envelopes) polyphonic output

This port outputs the envelope for each of the channels.

#### Out polyphonic output

This port outputs the crossfaded outputs for each of the channels.

### Polyphony Rules

#### Polyphonic Input
The effective number of input channels is the greatest of the following values:
- The number of poly channels at the input
- The selected number of crossfaded channels
- The selected start channel.

If the actual input is monophonic, then the input is replicated to match the effective input channel count.

If the actual input is polyphonic with fewer channels than the effective input channel count, then missing channels are assigned constant 0V.

#### Polyphonic Outputs
By default the number of output channels is minimized to match the selected number of crossfaded channels. In this case the start channel is always assigned to output channel 1, and there are no unused channels in the output.

A module context menu option is available to disable the output channel minimization. In this case the effective input channels map directly to the output channels. Unused input channels become constant 0V in the output.

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0 volts when Poly Fade is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## QUAD VC POLARIZER
![Quad VC Polarizer module image](doc/QuadVCPolarizer.png)  
Compact polyphonic bipolar VCA (ring modulator) and mixer inspired by Mutable Instruments Blinds.

### General operation - Blinds emulation

By default this module behaves the same as the Audible Instruments Quad VC Polarizer, which in turn emulates the [Mutable Instruments Blinds hardware](https://modulargrid.net/e/mutable-instruments-blinds). All the functionality has been shrunk down to 5hp, very similar to the Southpole Bandana module that was never officially ported to VCV 2.

There are 4 independent module channels, each with an Input, Output, Level attenuverter, and Level CV with Level Amount attenuverter.

The formula for the output is ***V<sub>out</sub> = V<sub>in</sub> x NetLevel%***, where ***NetLevel% = (Level% + V<sub>CV</sub>/V<sub>unity</sub> x CV%)***, clamped to +/- 200%

The input port is normalled to 5V, and the unity voltage is 5V. The way the math works, you can attenuate an input signal and add an offset by patching the input to the Level CV input, and leaving the Input port unpatched. The Level knob becomes the offset, and the CV Level knob the attenuator.

Each output is normalled to the output below so you can mix the outputs.

The Venom Quad VC Polarizer extends the Blinds functionality with support for polyphony, plus a number of options.

### Polyphony

For each output port, the number of polyphony channels is the maximum channel count found across all inputs that contribute to the output. So if all inputs are monophonic except for 5 channel polyphony at the first CV input, and the first output is unpatched, and the second output is patched, then the output will be 5 channel polyphony.

The polyphony acts as would be expected when the input and output channel counts match.

Monophonic input is automatically replicated to match the output channel count.

Polyphonic inputs with fewer channels assume constant 0V for any missing channels.

### O (Oversample) button
Sets the level of oversamping to apply to all inputs and outputs. Oversampling can be useful for controlling aliasing that would otherwise be introduced by ring modulation and/or clipping.
- **Off** (dark gray, default)
- **x2** (yellow)
- **x4** (green)
- **x8** (light blue)
- **x16** (dark blue)
- **x32** (purple)

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.

### N (Normal input value) button
Sets the input voltage if the port is not patched
- **5V** (yellow, default)
- **10V** (light blue)

### V (VCA CV) button
Determines the type of CV input that is accepted, where unity is 5V or 10V. The unipolar mode effectively creates more typical 2 quadrant VCAs instead of the default 4 quadrant VCAs (ring modulators)
- **Unipolar clamped** (green) = 0V to unity
- **Bipolar clamped** (orange) = -unity to unity
- **Bipolar unlimited** (purple, default)

### U (Unity) button
Determines the CV voltage that represents 100%
- **5V** (yellow, default)
- **10V** (light blue)

### C (Clippping) button
Optional clipping applied to the output
- **Off** (dark gray, default)
- **Hard +/- 10V** (white)
- **Hard +/- 5V** (yellow)
- **Soft +/- 12V** (light blue) = tanh saturation
- **Soft +/- 6V** (dark blue) = tanh saturation

### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V when Quad VC Polarizer is bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## VCA MIX 4
![VCA Mix 4 module image](doc/VCAMix4.png)  
A compact polyphonic VCA, mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. The module includes options for bipolar VCA (ring mod), hard or soft clipping, and DC offset removal. Module functionality can be extended by a set of [Mix Expanders](#mix-expanders).

### General Operation
There are four numbered inputs, each of which can be attenuated, inverted, and/or amplified by a level knob and CV input. Each modulated input can then be output to a dedicated numbered channel outupt and/or the modulated inputs can be summed to create a mix. There is also a 5th chain input, without modulation, that can be added to the mix. The mix can also be attenuated, inverted, and/or amplified by a mix level knob and CV input. Finally there are options to hard or soft clip the mix and/or remove DC offset, before sending the final mix to the Mix output. Oversampling is available for soft clipping to control any aliasing that might be introduced. The VCAs can be configured to have a linear or exponential response, and they can be unipolar or bipolar. Audio rate CV is supported so the VCA MIX 4 can do amplitude or ring modulation.

### LEVEL Knobs 1 - 4
These knobs control the base level of each of the input channel's VCAs. The exact behavior of the knobs depends on the setting of the Mode button. Each base level may be modulated by the corresponding CV input.

### CV Inputs 1 - 4
Each level knob has a corresponding polyphonic CV input that is normalled to 10V. The CV attenuates the knob base level (or possibly amplifies and/or inverts, depending on VCA mode), with 0V representing off, and 10V representing unity. The exact behavior of the CV depends on the VCA mode.

### IN Channel Inputs 1 - 4
The channel inputs for the VCAs that are subsequently summed in the MIX. Each input may be effectively normalled to a constant CV value, depending on the current Level Mode.

### OUT Channel Outputs 1 - 4
Each of the modulated inputs is sent to its corresponding numbered output, so the channel can function as a simple VCA, amplitude modulator, ring modulator, or constant CV source. Patched outputs may be removed from the final mix, depending on the setting of the Exclude button.

### Mix Level knob
This knob controls the base level of the final mix. The incoming mix consists of the sum of the numbered channel outputs and the Chain input. The exact behavior of the Mix Level knob depends on the setting of the Level Mode button. The mix base level may be modulated by the mix CV input.

Patched channel outputs may be excluded from the mix, depending on the setting of the Exclude button.

### MIX CV input
The polyphonic mix CV input is normalled to 10V, and it attenuates the mix level (or possibly amplifies and/or inverts, depending on VCA mode). 0v represents off, and 10V is unity. The exact behavior of the CV depends on the VCA mode.

### CHAIN input
The polyphonic chain input allows multiple VCA MIX 4 to be connected in series, without consuming any of the numbered inputs. The chain input is normalled to 0V, and is added to the mix prior to the Mix level modulation.

### MIX output
The final mix is output here. The final mix output may be clipped and/or have DC offset removed, depending on the settings of the Clip and DC buttons.

### Polyphony
The polyphonic channel count for each of the four numbered channel outputs is normally set to the maximum number of channels found across each CV and channel input pair. The count is set independently for each numbered channel output.

The final mix polyphonic channel count is normally the maximum channel count across the numbered outputs that are not excluded, as well as the chain and mix CV inputs.

Monophonic inputs are replicated to match the output polyphony. But polyphonic inputs with fewer channels than in the output send 0V for any missing channels.

There is one major exception when the Level Mode is set to poly sum, causing all outputs to be monophonic. In this mode, polyphonic signals at the numbered inputs and chain input are summed into a monophonic signal before any modulation. CV inputs are monophonic in this mode, meaning polyphonic CV channels 2 and above will be ignored.

### M (Level Mode) button
The color coded mode button determines how the 4 channel and the mix knobs behave. The mode button cycles through 5 possible modes. The button context menu allows direct selection of any mode. Each mode is labeled for audio or CV according to typical usage, but all modes can be applied to both audio and CV.
- **Unipolar dB (audio x2)** (pink - default): Each knob ranges from -inf dB (0V) to +6.0206 dB (x2 amplification), with the default of 0 dB (unity) at noon. Unpatched channel inputs are normalled to 0V.
- **Unipolar poly sum dB (audio x2)** (purple): Same as Unipolar audio dB, except all polyphonic channel and chain inputs are summed to a single monophonic signal before being attenuated or amplified by the corresponding level knob. Unpatched channel inputs are normalled to 0V. Note that all CV inputs are treated as monophonic in this mode.
- **Bipolar % (CV)** (green): Each input level knob ranges from -100% (inversion) to 100% (unity) if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpatched channel inputs are normalled to 10V. Each input level knob defaults to 0% (or 0V) at noon. The Mix knob always ranges from -100% to 100%, with a default of 100%.
- **Bipolar x2 (CV)** (light blue): Each input level knob ranges from -2x to 2x if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpatched channel inputs are normalled to 5V. Each input level knob defaults to 0x (or 0V) at noon. The Mix knob always ranges from -2x to 2x, with a default of 1x (unity).
- **Bipolar x10 (CV)** (dark blue): Each input level knob ranges from -10x to 10x if the channel input is patched, or -10V to 10V constant CV when unpatched. Effectively this means unpateched channel inputs are normalled to 1V. Each input level knob defaults to 0% (or 0V) at noon. The Mix knob always ranges from -10x to 10x, with a default of 1x (unity).

### V (VCA Mode) button
The color coded VCA mode button determines how CV inputs are interpreted. The button cycles through 6 possible modes. The button context menu allows direct selection of any mode.
- **Unipolar linear - CV clamped 1-10V** (pink - default): A "normal" VCA with linear response. The base level can only be attenuated, and negative CV values are treated as 0V.
- **Unipolar exponential - CV clamped 1-10V** (purple): A "normal" VCA with exponential response. The base level can only be attenuated, and negative CV values are treated as 0V.
- **Bipolar linear - CV unclamped** (light blue): A VCA with linear response that is capable of ring modulation. Negative CV values invert the base level, and the base level is amplified by CV with magnitude greater than 10V. High frequency inputs can introduce aliasing.
- **Bipolar exponential - CV unclamped** (dark blue): A VCA with exponential response that is capable of ring modulation. Negative CV values invert the base level, and the base level is amplified by CV with magnitude greater than 10V. High frequency inputs can introduce aliasing.
- **Bipolar linear band limited - CV unclamped** (yellow): A VCA with linear response that is capable of ring modulation. Negative CV values invert the base level, and the base level is amplified by CV with magnitude greater than 10V. The channel input and CV input are band limited in an attempt to minimize audio aliasing when performing ring modulation or amplitude modulation.
- **Bipolar exponential band limited - CV unclamped** (green): A VCA with exponential response that is capable of ring modulation. Negative CV values invert the base level, and the base level is amplified by CV with magnitude greater than 10V. The channel input and CV input are band limited by a low pass filter in an attempt to minimize audio aliasing when performing ring modulation or amplitude modulation. However, the exponential response introduces significantly more high frequency content, so the low pass filter does not help very much.

### D (DC Block) button
The color coded DC block button determines when (or if) a high pass filter is applied to the final mix to remove any DC offset. DC offset removal always occurs after the final mix is attenuated (or amplified) by the Mix level knob.
- **Off** (dark gray - default): DC offset is not removed.
- **Before clipping** (yellow): DC offset is removed prior to any clipping. Note that subsequent clipping may re-introduce some DC offset.
- **Before and after clipping** (green): DC offset is removed both before and after any clipping. Note that only one DC offset removal is performed if clipping is not applied.
- **After clipping** (light blue): DC offset is removed after any clipping.

The last three DC offset options give identical results when no clipping is applied.

### C (Clip) button
The color coded clip button determines how (or if) the final output is clipped.
- **Off** (dark gray - default)
- **Hard post-level at 10V** (white)
- **Soft post-level at 10V** (yellow)
- **Soft oversampled post-level at 10V** (orange)
- **Hard pre-level at 10V** (green)
- **Soft pre-level at 10V** (light blue)
- **Soft oversampled pre-level at 10V** (dark blue)
- **Saturate (Soft oversampled post-level at 6V)** (purple)

If clipping is off then there is no limit to the output voltage.

Hard clipping can produce significant aliasing if applied to audio signals.

Soft clipping provides tanh saturation. At moderate saturation levels there is little to no audible aliasing. But very hot signals can still lead to signfcant aliasing.

Soft oversampled clipping also provides saturation, but aliasing is greatly reduced.

### X (eXclude) button
The color coded exclude button determines if patched channel outputs are excluded or included in the final mix.
- **Off** (dark gray - default): patched output channels are included in the final mix
- **On** (red): patched output channels are excluded from the final mix

### Oversampling filter quality options
Oversampling is used by some Clip options as well as the VCA bandlimitted options. These options always use 4x oversampling.

There is a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](/README.md.md#anti-aliasing-via-oversampling) for more information.


### Standard Venom Context Menus
[Venom Themes](/README.md.md#themes), [Custom Names](/README.md.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The numbered channel inputs are passed unchanged to their corresponding outputs when VCA Mix 4 is bypassed. The Mix output is monophonic 0V when bypassed.

[Return to Table Of Contents](/README.md.md#venom)


## VCA MIX 4 STEREO
![VCA Mix 4 module image](doc/VCAMix4Stereo.png)  
A stereo compact polyphonic VCA, mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. The module includes options for bipolar VCA (ring mod), hard or soft clipping, and DC offset removal. Module functionality can be extended by a set of [Mix Expanders](#mix-expanders).

VCA Mix 4 Stereo is a stereo version of the [VCA MIX 4](#vca-mix-4), sharing the same features, but with the following differences:
- Each of the channel inputs and outputs, as well as the Chain input and Mix output are doubled to support left and right channels. Each stereo pair is controlled by its own single Level knob and CV input.
- Each right input is normaled to the corresponding left input. When using the bipolar Level mode, each input level knob produces constant CV only if both the left and right inputs are unpatched.
- The output channel count for each numbered channel is the maximum polyphony found across the corresponding left, right, and CV inputs.
- The output channel count for the Mix output is the maximum polyphony found across the chain and Mix CV inputs, as well as each numbered channel output that is not excluded from the mix.
- When bypassed, each channel's left input is sent unchanged to the left output, and right input to the right output. The right inputs are still normaled to the left inputs when bypassed. The Mix outputs remain monophonic 0V when bypassed.

All other behaviors are the same as for Mix 4.

[Return to Table Of Contents](/README.md.md#venom)


