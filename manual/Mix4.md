A compact polyphonic mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. Module functionality can be extended by a set of [Mix Expanders](mix_expanders.md).

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

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.
   
### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The MIX output is monophonic 0V if MIX 4 is bypassed.
