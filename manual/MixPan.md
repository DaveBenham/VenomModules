Allows panning of Mix input channels left and right via numbered knobs and inputs.

Each knob ranges from -1 (full counterclockwise) for hard left, to 0 (noon) for center, to +1 (full clockwise) for hard right.

Each CV input has an associated CV attenuverter. The CV is added to the knob value to determine the final pan level. Bipolar CV is expected, with -5V representing hard left and +5V representing hard right (assuming the knob is center panned). So -10V is guaranteed to result in pan hard left even if the knob is panned hard right. Similarly, +10V guarantees pan hard right even if the knob is panned hard left.

The CV inputs support polyphonic cables.

Pan is only available to stereo mix modules [Mix 4 Stereo](Mix4Stereo.ms) and [VCA Mix 4 Stereo](VCAMix4Stereo.md). Only one Pan module can be used per mix module.

More general information is available at the [Mix Expanders page](mix_expanders.md).