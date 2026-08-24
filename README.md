# Venom
Venom modules [version 2.16.0](changelog.md) for VCV Rack 2 are copyright 2023, 2024, 2025, 2026 Dave Benham and licensed under GNU General Public License version 3.

This is the documentation for all free Venom modules. Check out the [Venom Premium site](https://github.com/DaveBenham/VenomPremium/blob/main/README.md) for documentation about commercial Venom modules.

[Color Coded Ports](#color-coded-ports)  
[Themes](#themes)  
[Custom Names](#custom-names)  
[Parameter Locks and Custom Defaults](#parameter-locks-and-custom-defaults)  
[Venom Expander Modules](#venom-expander-modules)  
[Anti-aliasing via oversampling](#anti-aliasing-via-oversampling)  
[Acknowledgments](#acknowledgments)

#### Module Categories

- [Controllers](doc/Controllers.md)  
- [Envelope Generators](doc/EnvelopeGenerators.md)
- [Logic, Random, and/or Routing Modules](doc/LogicRandomRouting.md)
- [Mixers](doc/Mixers.md)
- [Modulation](doc/Modulation.md)
- [Oscillators](doc/Oscillators.md)
- [Patch Management](doc/PatchManagement.md)
- [Polyphony Management](doc/PolyphonyManagement.md)
- [Quantizers](doc/Quantizers.md)
- [Sequencers](doc/Sequencers.md)

#### Module Index
|[AD/ASR<br />ENVELOPE<br />GENERATOR](doc/EnvelopeGenerators.md#adasr-envelope-generator)|[AUXILLIARY<br />CLONE<br />EXPANDER](doc/PolyphonyManagement.md#auxilliary-clone-expander)|[BAY MODULES](doc/PatchManagement.md#bay-modules)|[BENJOLIN<br />OSCILLATOR](doc/Oscillators.md#benjolin-oscillator)|[BENJOLIN<br />GATES<br />EXPANDER](doc/Oscillators.md#benjolin-gates-expander)|[BENJOLIN<br />VOLTS<br />EXPANDER](doc/Oscillators.md#benjolin-volts-expander)|
|----|----|----|----|----|----|
|![AD/ASR Envelope Generator module image](doc/AD_ASR.png)|![Auxilliary Clone Expander module image](doc/AuxClone.png)|![Bay Input module image](doc/BayInput.png) &nbsp;![Bay Norm module image](doc/BayNorm.png) &nbsp;![Bay Output module image](doc/BayOutput.png)|![Benjolin Oscillator module image](doc/BenjolinOsc.png)|![Benjolin Gates Expander module image](doc/BenjolinGatesExpander.png)|![Benjolin Volts Expander module image](doc/BenjolinVoltsExpander.png)|

|[BERNOULLI<br />SWITCH](doc/LogicRandomRouting.md#bernoulli-switch)|[BERNOULLI<br />SWITCH<br />EXPANDER](doc/LogicRandomRouting.md#bernoulli-switch-expander)|[BLOCKER](doc/PatchManagement.md#blocker)|[BOUNDED<br />VCO](doc/Oscillators.md#bounded-vco)|[BYPASS<br />MODULE](doc/PatchManagement.md#bypass-module)|[CLONE<br />MERGE](doc/PolyphonyManagement.md#clone-merge)|[CROSS FADE 3D](doc/Mixers.md#cross-fade-3d)|[DYNAMIC<br />AMPLIFYING<br />SHAPING<br />ENVELOPE](doc/EnvelopeGenerators.md#dynamic-amplifying-shaping-envelope)|
|----|----|----|----|----|----|----|----|
|![Bernoulli Switch module image](doc/BernoulliSwitch.png)|![Bernoulli Switch Expander image](doc/BernoulliSwitchExpander.png)|![Blocker module image](doc/Blocker.png)|![Blocker module image](doc/BoundedVCO.png)|![Bypass module image](doc/Bypass.png)|![Clone Merge module image](doc/CloneMerge.png)|![Cross Fade 3D module image](doc/CrossFade3D.png)|![DASE module image](doc/DASE.png)|

|[ENVELOPE FACTORY](doc/EnvelopeGenerators.md#envelope-factory)|[HARMONIC<br />QUANTIZER](doc/Quantizers.md#harmonic-quantizer)|[KNOB 5](doc/Controllers.md#knob-5)|[LINEAR<br />BEATS](doc/Sequencers.md#linear-beats)|[LINEAR<br />BEATS<br />EXPANDER](doc/Sequencers.md#linear-beats-expander)|[LINEAR<br />MERGE](doc/Sequencers.md#linear-merge)|[LINEAR<br />MERGE<br />EXPANDER](doc/Sequencers.md#linear-merge-expander)|
|----|----|----|----|----|----|----|
|![Envelope Factory module image](doc/EnvelopeFactory.png)|![Harmonic Quantizer module image](doc/HQ.PNG)|![Knob 5 module image](doc/Knob5.png)|![Linear Beats module image](doc/LinearBeats.png)|![Linear Beats Expander module image](doc/LinearBeatsExpander.png)|![Linear Merge module image](doc/LinearMerge.png)|![Linear Merge module image](doc/LinearMergeExpander.png)|

|[LOGIC](doc/LogicRandomRouting.md#logic)|[MERGE<br />4x2](doc/PolyphonyManagement.md#merge-4x2)|[MERGE<br />SPLIT](doc/PolyphonyManagement.md#merge-split)|[MIX 4](doc/Mixers.md#mix-4)|[MIX 4<br />STEREO](doc/Mixers.md#mix-4-stereo)|[MIX EXPANDERS](doc/Mixers.md#mix-expanders)|
|----|----|----|----|----|----|
|![Logic module image](doc/Logic.png)|![Merge 4x2 module image](doc/Merge4x2.png)|![Merge Split module image](doc/MergeSplit.png)|![Mix 4 module image](doc/Mix4.png)|![Mix 4 Stereo module image](doc/Mix4Stereo.png)|![Mix Offset Expander module image](doc/MixOffset.png) &nbsp;![Mix Mute Expander module image](doc/MixMute.png) &nbsp;![Mix Solo Expander module image](doc/MixSolo.png) &nbsp;![Mix Fade Expander module image](doc/MixFade.png) &nbsp;![Mix Fade2 Expander module image](doc/MixFade2.png) &nbsp;![Mix Pan Expander module image](doc/MixPan.png) &nbsp;![Mix Send Expander module image](doc/MixSend.png)|

|[MOUSE<br />PAD](doc/Controllers.md#mouse-pad)|[MULTI<br />MERGE](doc/PolyphonyManagement.md#multi-merge)|[MULTI<br />SPLIT](doc/PolyphonyManagement.md#multi-split)|[MULTIMODE FILTER](doc/Modulation.md#multimode-filter)|[NON-OCTAVE REPEATING SCALE<br />INTERVALLIC QUANTIZER](doc/Quantizers.md#non-octave-repeating-scale-intervallic-quantizer)|[NORSIQ<br />CHORD<br />TO<br />SCALE](doc/Quantizers.md#norsiq-chord-to-scale)|
|----|----|----|----|----|----|
|![Mouse Pad module image](doc/MousePad.png)|![Multi Merge module image](doc/MultiMerge.png)|![Multi Split module image](doc/MultiSplit.png)|![Multimode Filter module image](doc/SVF.png)|![Non-Octave Repeating Scale Intervallic Quantizer image](doc/NORS_IQ.png)|![NORSIQ Chord To Scale module image](doc/NORSIQChord2Scale.png)|

|[NULL<br />CABLE](doc/PatchManagement.md#null-cable)|[OCTAVER](doc/Modulation.md#octaver)|[PAN 3D](doc/LogicRandomRouting.md#pan-3d)|[POLY<br />CLONE](doc/PolyphonyManagement.md#poly-clone)|[POLY FADE](doc/Mixers.md#poly-fade)|[POLY<br />MUTE](doc/Controllers.md#poly-mute)|[POLY<br />OFFSET](doc/Controllers.md#poly-offset)|[POLY<br />PRUNE](doc/PolyphonyManagement.md#poly-prune)|
|----|----|----|----|----|----|----|----|
|![Null Cable module image](doc/NullCable.png)|![Octaver module image](doc/Octaver.png)|![Pan 3D module image](doc/Pan3D.png)|![Poly Clone module image](doc/PolyClone.png)|![Poly Fade module image](doc/PolyFade.png)|![ module image](doc/PolyMute.png)|![Poly Offset module image](doc/PolyOffset.png)|![Poly Prune module image](doc/PolyPrune.png)|

|[POLY<br />SAMPLE & HOLD<br />ANALOG SHIFT<br />REGISTER](doc/LogicRandomRouting.md#poly-sample--hold-analog-shift-register)|[POLY<br />SCALE](doc/Modulation.md#poly-scale)|[POLY<br />UNISON](doc/PolyphonyManagement.md#poly-unison)|[PUSH 5](doc/Controllers.md#push-5)|[QUAD VC<br />POLARIZER](doc/Mixers.md#quad-vc-polarizer)|[RATIO](doc/Quantizers.md#ratio)|[RECURSE](doc/PolyphonyManagement.md#recurse)|[RECURSE<br />STEREO](doc/PolyphonyManagement.md#recurse-stereo)|
|----|----|----|----|----|----|----|----|
|![Poly Sample & Hold Analog Shift Register module image](doc/PolySHASR.png)|![Poly Scale module image](doc/PolyScale.png)|![Poly Unison module image](doc/PolyUnison.PNG)|![Push 5 module image](doc/Push5.png)|![Quad VC Polarizer module image](doc/QuadVCPolarizer.png)|![Ratio module image](doc/Ratio.png)|![RECURSE module image](doc/Recurse.PNG)|![RECURSE STEREO module image](doc/RecurseStereo.PNG)|

|[REFORMATION](doc/Modulation.md#reformation)|[RHYTHM EXPLORER](doc/Sequencers.md#rhythm-explorer)|[RHYTHM<br />EXPLORER<br />CV EXPANDER](doc/Sequencers.md#rhythm-explorer-cv-expander)|[SHAPED<br />VCA](doc/Modulation.md#shaped-vca)|
|----|----|----|----|
|![Reformation module image](doc/Reformation.PNG)|![Rhthm Explorer module image](doc/RhythmExplorer.PNG)|![Rhythm Explorer CV Expander module image](doc/REXCV.png)|![SHAPED VCA module image](doc/ShapedVCA.png)|

|[SLEW](doc/Modulation.md#slew)|[SPHERE<br />TO XYZ](doc/Modulation.md#sphere-to-xyz)|[SPLIT<br />4x2](doc/PolyphonyManagement.md#split-4x2)|[SPREAD<br />MERGE](doc/PolyphonyManagement.md#spread-merge)|[SPREAD<br />MERGE<br />EXPANDER](doc/PolyphonyManagement.md#spread-merge-expander)|[STEREO<br />MERGE<br />SPLIT](doc/PolyphonyManagement.md#stereo-merge-split)|[THRU](doc/PatchManagement.md#thru)|[VCA<br />MIX 4](doc/Mixers.md#vca-mix-4)|[VCA MIX 4 STEREO](doc/Mixers.md#vca-mix-4-stereo)|
|----|----|----|----|----|----|----|----|----|
|![SLEW module image](doc/Slew.png)|![Sphere To XYZ module image](doc/SphereToXYZ.png)|![Split 4x2 module image](doc/Split4x2.png)|![Spread Merge module image](doc/SpreadMerge.png)|![Spread Merge Expander module image](doc/SpreadMergeExpander.png)|![Stereo Merge Split module image](doc/StereoMergeSplit.png)|![THRU module image](doc/Thru.png)|![VCA MIX 4 module image](doc/VCAMix4.png)|![VCA Mix 4 Stereo module image](doc/VCAMix4Stereo.png)|

|[VCO LAB](doc/Oscillators.md#vco-lab)|[VCO UNIT](doc/Oscillators.md#vco-unit)|[VENOM<br />BLANK](doc/PatchManagement.md#venom-blank)|[WAVE<br />FOLDER](doc/Modulation.md#wave-folder)|[WAVE<br />MANGLER](doc/Modulation.md#wave-mangler)|
|----|----|----|----|----|
|![VCO Lab module image](doc/Oscillator.png)|![VCO Unit module image](doc/VCOUnit.png)|![VENOM BLANK module image](doc/VenomBlank.PNG)|![WAVE FOLDER module image](doc/WaveFolder.png)|![WAVE MANGLER module image](doc/WaveMangler.png)|

|[WAVE<br />MULTIPLIER](doc/Modulation.md#wave-multiplier)|[WIDGET<br />MENU<br />EXTENDER](doc/PatchManagement.md#widget-menu-extender)|[WINCOMP](doc/LogicRandomRouting.md#wincomp)|[WINCOMP 2 + LOGIC](doc/LogicRandomRouting.md#wincomp-2--logic)|[XM-OP](doc/Oscillators.md#xm-op)|
|----|----|----|----|----|
|![WAVE MULTIPLIER module image](doc/WaveMultiplier.png)|![WIDGET MENU EXTENDER module imiage](doc/WidgetMenuExtender.png)|![WINCOMP module image](doc/WinComp.PNG)|![WinComp 2 + Logic module image](doc/Compare2.png)|![XM-OP module image](doc/XM_OP.png)|

## Color Coded Ports
All polyphonic ports use brass cores, while monophonic ports use steel cores.

Input ports are on the base faceplate color, and output ports are on a contrasting background color.

[Return to Table Of Contents](#venom)

## Themes
The context menu of every module includes options to set the default theme and default dark theme for the Venom plugin, as well as a theme override for each module instance.

There are 4 themes to choose from.

|Ivory|Coal|Earth|Danger|
|---|---|---|---|
|![Ivory theme image](doc/Ivory.png)|![Coal theme image](doc/Coal.png)|![Earth theme image](doc/Earth.png)|![Danger theme image](doc/Danger.png)|

If a module instance is set to use a specific theme, then that theme will be used regardless whether VCV Rack is set to use dark panels or not. If a module is set to use the default theme, then the VCV Rack "Use dark panels if available" setting controls which default is used. If not enabled, then the default theme is used. If enabled then the default dark theme is used.

If you want the default theme to disregard the VCV Rack dark panel setting, then simply set both defaults to the same theme.

The factory default theme is ivory, and the factory default dark theme is coal.

[Return to Table Of Contents](#venom)

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

[Return to Table Of Contents](#venom)

## Venom Expander Modules
A number of Venom modules do not do anything on their own, but rather augment the functionality of a compatible base module when placed beside it. Each Venom base module that supports expanders has module context menu options to add expanders without having to open the module browser.

VCV Rack supports two different mechanisms for implementinig expander modules:
- Both the parent (base) module and the expander perform work, and they communicate with each other via messages that introduce sample delays, much as cables do in VCV Rack.
- The base module does all the work, accessing the expander inputs, outputs, and controls directly. This does not introduce any sample delays.

All Venom expanders are implemented using the second method where the base module directly accesses the expander, so Venom expanders do not introduce sample delays.

[Return to Table Of Contents](#venom)

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

[Return to Table Of Contents](#venom)

## Acknowledgments
Special thanks to Andrew Hanson of [PathSet modules](https://library.vcvrack.com/?brand=Path%20Set) for setting up my GitHub repository, providing advice and ideas for the Rhythm Explorer and plugins in general, and for writing the initial prototype code for the Rhythm Explorer.

Also a hearty thanks to Squinky Labs for their [VCV Rack Demo project](https://github.com/squinkylabs/Demo), which showed me how to implement oversampling, and also got my foot in the door to understanding how to use SIMD with plugin development.

Thanks to Jacky Ligon and Andreya Ek Frisk over on the Surge Discord server for advice on the NonOctave Repeating Scale Intervallic Quantizer, as well as help with compiling a representative set of scale presets.

Super thanks to Benjamin Dill for his open source Stoermelder PackOne plugin. I could never have developed the Widget Menu Extender module or the Bypass module without his tips and source code to study.

Thanks to Paul Dempsey for his MenuTextField struct from the pachde1 plugin that allows text entry in a menu. In turn that was developed using code/ideas from the SubmarineFree plugin by David O'Rourke.

Finally thanks to Ewan Hemingway. Through discussions and studying the Befaco Even VCO source code I was able to improve the sound quality of VCO Lab and VCO Unit by adding DPW alias suppression to supplement oversampling.

[Return to Table Of Contents](#venom)

