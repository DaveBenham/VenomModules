A stereo compact polyphonic VCA, mixer, attenuator, inverter, amplifier, and/or offset suitable for both audio and CV. The module includes options for bipolar VCA (ring mod), hard or soft clipping, and DC offset removal. Module functionality can be extended by a set of [Mix Expanders](mix_expanders.md).

VCA Mix 4 Stereo is a stereo version of the [VCA MIX 4](VCAMix4.md) module, sharing the same features, but with the following differences:
- Each of the channel inputs and outputs, as well as the Chain input and Mix output are doubled to support left and right channels. Each stereo pair is controlled by its own single Level knob and CV input.
- Each right input is normaled to the corresponding left input. When using the bipolar Level mode, each input level knob produces constant CV only if both the left and right inputs are unpatched.
- The output channel count for each numbered channel is the maximum polyphony found across the corresponding left, right, and CV inputs.
- The output channel count for the Mix output is the maximum polyphony found across the chain and Mix CV inputs, as well as each numbered channel output that is not excluded from the mix.
- When bypassed, each channel's left input is sent unchanged to the left output, and right input to the right output. The right inputs are still normaled to the left inputs when bypassed. The Mix outputs remain monophonic 0V when bypassed.

All other behaviors are the same as for Mix 4.
