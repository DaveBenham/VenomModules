A polyphonic configurable wave folder and VCA.

The incoming signal is amplified and then folded at +/-5 volts a fixed number of stages. Increasing amplification increases the number of folds, with a limit set by the Stages count. The signal can be amplified once before any folding, and/or once for each folding stage. The folding can be made asymmetric by applying a bias voltage to the signal prior to any amplification. The final result is soft clipped at +/-6V by tanh saturation.

All amplification can be controlled via CV, thus making the wavefolder a VCA as well. The VCAs can be configured to be unipolar or bipolar, and they can process audio rate CV, so the module can also perform wave folded amplitude modulation or wave folded ring modulation.

Here is a video demonstrating the basic functionality, along with some patch ideas:

https://www.youtube.com/watch?v=HLTyiIAUELs

And here Omri Cohen demonstrates some creative uses of the Wave Folder:

https://www.youtube.com/watch?v=BHY5gI--D-c

### STAGES button
Controls the maximum amount of folding stages, which in turn limits the maximum number of folds. There are five possible values
- 2
- 3 (default)
- 4
- 5
- 6

### OVERSAMPLE button
Wave folding, ring modulation, and amplitude modulation can introduce many harmonics, which can lead to aliasing. Oversampling can be used to limit the amount of aliasing.
- Off
- x2
- x4 (default)
- x8
- x16
- x32

There is also a context menu option to select the quality of the filters used for oversampling.

See [Anti-aliasing via oversampling](plugin.md#anti-aliasing-via-oversampling) for more information.

By default, oversampling is applied to all inputs.

Oversampling has a significant CPU cost, so best to apply the minimum amount that sounds good. LFO rate modulation should not need oversampling. The three CV input ports have a context menu option to disable oversampling for that port. These ports have a LED above and to the right of the port. It is off if there is no patched input, or if the OverSample button is set to Off. It is yellow if there is input and oversampling is applied. It is red if there is input and oversampling is active for the module, but disabled for that port.

### Polyphony
Wave Folder is fully polyphonic. The number of output channels is the maximum number of polyphony channels found accross all inputs.

Inputs that match the output polyphony behave as expected. Monophonic inputs are replicated to match the output polyphony count. Polyphonic inputs with fewer channels get constant 0V for any missing channels.


### PRE-AMP knob and CV input
Sets the amount of amplification applied to the input prior to any folding stages.

The total amplification is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Pre-amp CV Amount knob. CV can be driven at audio rates.

The knob ranges from 0 to 10, with a default of 1 (unity). But the CV is not limited, so the net amplification is unconstrained.

By default, the Pre-Amp VCA is unipolar, meaning any net amplification level less than zero is treated as zero. The port has a context menu option to use a bipolar VCA instead that can process negative values and invert the signal. The LED above and to the left of the port glows yellow if bipolar mode is enabled. 

The Pre-Amp CV port also has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.


### STAGE AMP knob and CV input
Sets the amount of amplification applied at each stage of folding. See the note at the bottom of this section if you are interested in the exact mathematical formula.

The total stage amplification is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Stage Amp CV Amount knob. CV can be driven at audio rates.

The knob ranges from 0.5 to 10, but the CV is not limited, so the net amplification is unconstrained.

Note that the Stage Amp knob is scaled exponentially, but the CV is linear. The exponential knob scale makes it easier to dial-in the most useful values between 1 and 2.

By default, the Stage Amp VCA is unipolar, meaning any net amplification level less than zero is treated as zero. The port has a context menu option to use a bipolar VCA instead that can process negative values and invert the signal. The LED above and to the left of the port glows yellow if bipolar mode is enabled. 

The Stage Amp CV port also has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.

#### Actual stage folding formula

Without any stage amplification, the folding formula for each stage is **V***out* **= clip( V***in* **) - (V***in* **- clip( V***in* **))**, where clipping occurs at +/-5 volts.  
This can be re-written as **V***out* **= 2 x clip(V***in* **) - V***in*

The stage amplification is only applied to the clip expression as follows: **V***out* **= 2 x clip( V***in* **x Amp***stage* **) - V***in*

### BIAS knob and CV input

Sets the amount of bias voltage to add to the input signal prior to any amplification. Non-zero bias leads to asymmetric folding.

The total bias is the sum of the knob and CV values. The CV can be attenuated and/or inverted by the Bias CV Amount knob. CV can be driven at audio rates.

The knob ranges from -5 to 5, but the CV is not limited, so the net bias is unconstrained.

The Bias CV port has the context menu option to disable oversampling, with the LED above and to the right indicating oversampling state.

### IN input
This is the input signal to be folded.

The input is always oversampled at the level selected by the OverSample button.

### OUT output
This is the final result of the wave folding.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
The poly input is passed through to the output if Wave Folder is bypassed.
