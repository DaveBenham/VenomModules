This is the documentation for the Venom plugin [version 2.17.6](changelog.md) containing free modules for VCV Rack. This page documents features that are common to all Venom modules. The [Module List](#module-list) at the end includes documentation links for every module in the plugin.

[Color Coded Ports](#color-coded-ports)  
[Themes](#themes)  
[Custom Names](#custom-names)  
[Parameter Locks and Custom Defaults](#parameter-locks-and-custom-defaults)  
[Venom Expander Modules](#venom-expander-modules)  
[Anti-aliasing via oversampling](#anti-aliasing-via-oversampling)  
[Acknowledgments](#acknowledgments)  
[Module List](#module-list)

## Color Coded Ports
All polyphonic ports use brass cores, while monophonic ports use steel cores.

Input ports are on the base faceplate color, and output ports are on a contrasting background color.

[Return to Table Of Contents](#venom)

## Themes
The context menu of every module includes options to set the default theme and default dark theme for the Venom plugin, as well as a theme override for each module instance.

There are 4 themes to choose from.

|Ivory|Coal|Earth|Danger|
|---|---|---|---|
|![Ivory theme image](images/Ivory.png)|![Coal theme image](images/Coal.png)|![Earth theme image](images/Earth.png)|![Danger theme image](images/Danger.png)|

If a module instance is set to use a specific theme, then that theme will be used regardless whether VCV Rack is set to use dark panels or not. If a module is set to use the default theme, then the VCV Rack "Use dark panels if available" setting controls which default is used. If not enabled, then the default theme is used. If enabled then the default dark theme is used.

If you want the default theme to disregard the VCV Rack dark panel setting, then simply set both defaults to the same theme.

The factory default theme is ivory, and the factory default dark theme is coal.

## Custom Names
Nearly every port (input or output), and every parameter (module knob, switch, or button etc.) within the Venom plugin has its own context menu option to set a custom name. Custom names only appear in context menus and hover text - they do not change the faceplate graphics.

If a parameter or port is given a custom name, then an additional option is added to restore the factory default name.

Custom names are saved with the patch and with presets, and restored upon patch or preset load. Custom names are also preserved when duplicating a module.

## Parameter Locks and Custom Defaults
Nearly every parameter (module knob, switch, or button etc.) within the Venom plugin has its own parameter context menu options to lock the paramenter as well as set a custom default value. In addition, most modules have module context menu options to lock and unlock all parameters within that instance of the module. Likewise there is a "Restore all factory names" context menu option that restors factory names for all parameters and ports within that instance of the module.

Parameter lock and custom default settings are saved with the patch and with presets, and restored upon patch or preset load. Parameter lock and custom default settings are also preserved when duplicating a module.

### Parameter Locks
The parameter tooltip includes the word "locked" below the parameter name when hovering over a locked parameter.

The parameter value cannot be changed by any means while the parameter is locked. All of the normal means of changing a parameter value are blocked:
- The parameter cannot be dragged or pushed
- Context menu value keyins are ignored
- Double click and context menu initialization are ignored
- Randomization requests are ignored

### Custom Defaults
A custom default value overrides the factory default whenever a parameter is initialized. An additional parameter menu option is added to restore the factory default whenever a custom default is in effect.

## Venom Expander Modules
A number of Venom expander modules do not do anything on their own, but rather augment the functionality of a compatible base module when placed beside it. Each Venom base module that supports expanders has module context menu options to add expanders without having to open the module browser.

VCV Rack supports two different mechanisms for implementinig expander modules:
- Both the parent (base) module and the expander perform work, and they communicate with each other via messages that introduce sample delays, much as cables do in VCV Rack.
- The base module does all the work, accessing the expander inputs, outputs, and controls directly. This does not introduce any sample delays.

All Venom expanders are implemented using the second method where the base module directly accesses the expander, so Venom expanders do not introduce sample delays.

## Anti-aliasing via oversampling
All digital synthesis techniques must deal with anti-aliasing to get the best possible audio output. Frequencies above 50% of the sample rate (the Nyquist frequency) cannot be represented, and instead are reflected back below the Nyquist frequency. Harmonically rich wave forms like a saw wave can have harmonics above the Nyquist frequency that create inharmonic audible tones when reflected, called aliasing. These reflections are typically not wanted, so most digital systems use one or more techniques to greatly reduce the amplitude of aliased harmonics so as to make them inaudible.

Venom uses oversampling as the primary anti-alias technique. Incoming signals are upsampled by factors of 2 with interpolation to get an effective sample rate with a much higher Nyquist frequency. All the digital computations are done at this higher frequency. The end result is filtered by a low pass filter with a cutoff below the true sample rate Nyquist frequency. This process removes the high frequency content before it can be aliased, and then it is downsampled to return to the actual sample rate. Note that any pre-existing aliasing at the inputs cannot be removed - high frequency content must be removed before it is ever aliased.

The higher the degree of oversampling, the better the result, but at the cost of higer CPU usage. Another factor is the slope of the low pass filters that are used for both the upsampling interpolation and the bandlimited downsampling. The higer the filter order, the steeper the slope, which allows for preservation of more high frequency content below the Nyquist frequency. But the higher filter orders also require more CPU. So it is a balancing act to select the minimum oversampling rate and filter order that gives good results.

Most Venom modules that use oversampling have parameters on the faceplate to chose the oversample rate. A few modules have context menu options instead. For most applications, an oversample rate of 4x or 8x gives good results, without using excessive CPU. But don't be afraid to try out more or less. In some cases you may be able to turn off oversampling entirely, and still get good results, thus saving considerable CPU.

All of the Venom modules with oversampling also have a context menu option to specify the quality of the filter used. There are three options available:
- 10th order with a cutoff at 80% of the Nyquist frequency. This is the default value used by all Venom modules.
- 8th order with a cutoff at 80% of the Nyquist frequency.
- 6th order with a cutoff at 50% of the Nyquist frequency.

Again, feel free to experiment to find what works best for you.

## Acknowledgments
Special thanks to Andrew Hanson of [PathSet modules](https://library.vcvrack.com/?brand=Path%20Set) for setting up my GitHub repository, providing advice and ideas for the Rhythm Explorer and plugins in general, and for writing the initial prototype code for the Rhythm Explorer.

Also a hearty thanks to Squinky Labs for their [VCV Rack Demo project](https://github.com/squinkylabs/Demo), which showed me how to implement oversampling, and also got my foot in the door to understanding how to use SIMD with plugin development.

Thanks to Jacky Ligon and Andreya Ek Frisk over on the Surge Discord server for advice on the NonOctave Repeating Scale Intervallic Quantizer, as well as help with compiling a representative set of scale presets.

Super thanks to Benjamin Dill for his open source Stoermelder PackOne plugin. I could never have developed the Widget Menu Extender module or the Bypass module without his tips and source code to study.

Thanks to Paul Dempsey for his MenuTextField struct from the pachde1 plugin that allows text entry in a menu. In turn that was developed using code/ideas from the SubmarineFree plugin by David O'Rourke.

Finally thanks to Ewan Hemingway. Through discussions and studying the Befaco Even VCO source code I was able to improve the sound quality of VCO Lab and VCO Unit by adding DPW alias suppression to supplement oversampling.

## Module List

[AD/ASR Envelope Generator](AD_ASR.md)  
[Auxilliary Clone Expander](AuxClone.md)  
[Bay Input](BayInput.md)  
[Bay Normal](BayNorm.md)  
[Bay Output](BayOutput.md)  
[Benjolin Oscillator](BenjolinOsc.md)  
[Benjolin Gates Expander](BenjolinGatesExpander.md)  
[Benjolin Volts Expander](BenjolinVoltsExpander.md)  
[Bernoulli Switch](BernoulliSwitch.md)  
[Bernoulli Switch Expander](BernoulliSwitchExpander.md)  
[Blocker](Blocker.md)  
[Bounded VCO](BoundedVCO.md)  
[Bypass](Bypass.md)  
[Clone Merge](CloneMerge.md)  
[Cross Fade 3D](CrossFade3D.md)  
[DASE - Dynamic Amplifying Shaping Envelope](DASE.md)  
[Envelope Factory](EnvelopeFactory.md)  
[HQ - Harmonic Quantizer](HQ.md)  
[Knob 5](Knob5.md)  
[Linear Beats](LinearBeats.md)  
[Linear Beats Expander](LinearBeatsExpander.md)  
[Linear Merge](LinearMerge.md)  
[Linear Merge Expander](LinearMergeExpander.md)  
[Logic](Logic.md)  
[Merge 4x2](Merge4x2.md)  
[Merge Split](MergeSplit.md)  
[Mix 4](Mix4.md)  
[Mix 4 Stereo](Mix4Stereo.md)  
[Mix Fade Expander](MixFade.md)  
[Mix Fade 2 Expander](MixFade2.md)  
[Mix Offset Expander](MixOffset.md)  
[Mix Mute Expander](MixMute.md)  
[Mix Pan Expander](MixPan.md)  
[Mix Send Expander](MixSend.md)  
[Mix Solo Expander](MixSolo.md)  
[Mouse Pad](MousePad.md)  
[Multi Merge](MultiMerge.md)  
[Multi Split](MultiSplit.md)  
[Multimode Filter](SVF.md)  
[NORSIQ - Non-Octave Repeating Scale Intervallic Quantizer](NORS_IQ.md)  
[NORSIQ Chord To Scale](NORSIQ_Chord2Scale.md)  
[Null Cable](NullCable.md)  
[Octaver](Octaver.md)  
[Pan 3D](Pan3D.md)  
[Poly Clone](PolyClone.md)  
[Poly Fade](PolyFade.md)  
[Poly Mute](PolyMute.md)  
[Poly Offset](PolyOffset.md)  
[Poly Prune](PolyPrune.md)  
[Poly Sample & Hold Analog Shift Register](PolySHASR.md)  
[Poly Scale](PolyScale.md)  
[Poly Unison](PolyUnison.md)  
[Push 5](Push5.md)  
[Quad VC Polarizer](QuadVCPolarizer.md)  
[Random](Random.md)  
[Ratio](Ratio.md)  
[Recurse](Recurse.md)  
[Recurse Stereo](RecurseStereo.md)  
[Reformation](Reformation.md)  
[Rhythm Explorer](RhythmExplorer.md)  
[Rhythm Explorer CV Expander](REXCV.md)  
[Shaped VCA](ShapedVCA.md)  
[Slew](Slew.md)  
[Sphere To XYZ](SphereToXYZ.md)  
[Split 4x2](Split4x2.md)  
[Spread Merge](SpreadMerge.md)  
[Spread Merge Expander](SpreadMergeExpander.md)  
[Stereo Merge Split](StereoMergeSplit.md)  
[Thru](Thru.md)  
[VCA Mix 4](VCAMix4.md)  
[VCA Mix 4 Stereo](VCAMix4Stereo.md)  
[VCO Lab](Oscillator.md)  
[VCO Unit](VCOUnit.md)  
[Venom Blank](VenomBlank.md)  
[Wave Folder](WaveFolder.md)  
[Wave Mangler](WaveMangler.md)  
[Wave Multiplier](WaveMultiplier.md)  
[Widget Menu Extender](WidgetMenuExtender.md)  
[WinComp](WinComp.md)  
[WinComp 2 + Logic](Compare2.md)  
[XM-OP](XM_OP.md)  
