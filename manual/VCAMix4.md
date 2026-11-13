A compact polyphonic VCA, mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. The module includes options for bipolar VCA (ring mod), hard or soft clipping, and DC offset removal. Module functionality can be extended by a set of [Mix Expanders](mix_expanders.md).

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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.


### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The numbered channel inputs are passed unchanged to their corresponding outputs when VCA Mix 4 is bypassed. The Mix output is monophonic 0V when bypassed.
