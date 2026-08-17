# Polyphony Management
- [AUXILLIARY CLONE EXPANDER](#auxilliary-clone-expander)
- [CLONE MERGE](#clone-merge)
- [MERGE 4x2](#clone-4x2)
- [MERGE SPLIT](#merge-split)
- [MULTI MERGE](#multi-merge)
- [MULTI SPLIT](#multi-split)
- [POLY CLONE](#poly-clone)
- [POLY PRUNE](#poly-prune)
- [POLY UNISON](#poly-unison)
- [RECURSE](#recurse)
- [RECURSE STEREO](#recurse-stereo)
- [SPLIT 4x2](#split-4x2)
- [SPREAD MERGE](#spread-merge)
- [SPREAD MERGE EXPANDER](#spread-merge-expander)
- [STEREO MERGE SPLIT](#stereo-merge-split)

[Venom top](/README.md#venom)

## AUXILLIARY CLONE EXPANDER
![Auxilliary Clone Expander module image](AuxClone.png)  
This expander module adds additional cloned poly input/output pairs to [Clone Merge](x#clone-merge), [Poly Clone](x#poly-clone), or [Poly Unison](x#poly-unison).

The expander must be placed immediately to the right of a Clone Merge, Poly Merge, or Poly Unison. The yellow LED in the upper left indicates whether the expander has successfully connected to a parent module.

Each set of polyphonic input channels is cloned to match the clone count of the parent module, and sent to the output. The number of polyphonic channels at the input should either match the number of input channels at the parent, or else 1. If the input is unpatched it is treated as a mono input with a single chanel at constant 0 volts.

The GRP button above and to the right of each input port controls how the cloned channels will be grouped at the corresponding output
- **Input channel** ***(yellow, default)*** - All the cloned channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be cloned at the output.

The number of polyphonic channels at each output will always match the number of poly output channels at the parent. The LED to the right of each output indicates whether the output was able to properly clone all input channels.

If the input poly count matches the parent, then each of the input channels is cloned as per the parent, and the LED is yellow.

If the input poly count is 1, then the input is replicated to match the parent input channel count, and then each of those channels is cloned. The LED is yellow.

If the input poly count is less than the parent and greater than 1, then the missing channels are treated as constant 0V, and the LED is orange.

If the input poly count is greater than the parent, then excess channels are ignored, and the LED is red.

All outputs will be constant 0V and all port LEDS will be black under any of the following conditions:
- The expander is not connected to a parent.
- The expander is bypassed.
- The parent module is bypassed.

The names of each input/output pair are linked. Changing the name of one will automatically change the name of the other.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## CLONE MERGE
![Clone Merge module image](CloneMerge.png)  
Clone Merge clones up to 8 monophonic inputs and merges the resultant channels into a single polyphonic output. It is especially useful with the Recurse modules when using polyphonic inputs. Clone Merge provides a convenient way to replicate CV inputs to match the recursion count.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](x#auxilliary-clone-expander).

### CLONE knob
Selects the number of times to clone or replicate each input. Possible values range from 1 to 16.

### MONO inputs
The 8 monophonic inputs should be populated from top to bottom. Each input is replicated based on the Clone count as long as the total channel count across all replicated inputs does not exceed 16. Inputs that cannot be replicated the full amount are ignored.

An LED glows yellow for each input that is successfully replicated. The LED glows red if the input cannot be replicated. Unpatched inputs below the last patched input are ignored and the corresponding LED is off (black).

### GRP (Output grouping) button
The GRP controls how the cloned inputs will be grouped at the corresponding output
- **Input channel** ***(yellow, default)*** - All the clones for a given input will be grouped together at the output.
- **Input set** ***(blue)*** - The inputs will be grouped together in order as a set, and then the set will be cloned at the output.

### POLY output
All of the replicated inputs are merged into the single polyphonic output. The order of the channels is dependent on the GRP button.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Clone Merge is bypassed then the output is constant monophonic 0V.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## MERGE 4x2
![Merge 4x2 module image](Merge4x2.png)  
Merge 4x2 is a compact and versatile polyphonic merge utility that can combine monophonic and/or polyphonic inputs into a polyphonic output.

There are two independent sections, each capable of merging up to four inputs into one output. If the top output is not patched, then the two sections are merged, thus allowing up to eight inputs to be merged into a single polyphonic output at the bottom output port.

Input ports should be patched starting at the top and working down within a section. Unused input ports below the last patched port are ignored. Unused ports above the last port are treated as a single channel of constant 0V.

At most 16 channels can be merged, starting at the top, and working down. A glowing red LED indicates a polyphonic input with channels that could not be merged.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Merge 4x2 is bypassed.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)

## MERGE SPLIT
![Merge Split module image](MergeSplit.png)  
Merge Split is a compact and versatile polyphonic utility that can merge up to 4 monophonic or polyphonic inputs into one polyphonic output, and also split a single polyphonic input into any combination of monophonic and polyphonic outputs.

The top merge section can merge up to 16 channels from the four inputs. A glowing red LED next to an input port indicates a polyphonic input that has channels that could not be merged.

If the polyphonic input in the bottom split section contains the same number of channels found across all inputs in the merge section, then each split output polyphonic channel count will match that of the corresponding merge input. The LED between the merge and split sections will glow yellow to indicate a successful resplit.

If the number of channels at the split input does not match the merge input channel count, then by default each split output will be monophonic.

You can override the channel count for each split output via a port context menu. Setting any split output port to a specific channel count will disable automatic resplitting.

There is a module context menu option to restore resplit mode for all four split outputs.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Merge Split is bypassed.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## MULTI MERGE
![Multi Merge module image](MultiMerge.png)  
Merge one or more sets of mono and/or poly inputs into polyphonic outputs.

This merge utility is extremely flexible, with many configurations possible. It can merge a set of monophonic inputs into one polyphonic output. It can also merge a set of polyphonic inputs into one polyphonic ouput. Or it can merge a mixture of mono and poly inputs into one polyphonic output. Finally, there can be multiple sets of merges, each with its own poly output.

The thick red lines indicate which input ports are merged and sent to which output port. The groupings are defined by which output ports are patched. Unpatched output ports are ignored. The module is automatically reconfigured each time you patch or unpatch an output port. Each patched output merges the inputs from that row and above until it reaches another row with a patched output. If there are no output ports, then the module assumes the last output port will be patched.

The number of polyphonic output channels cannot exceed 16. If the sum of polyphony counts across the inputs exceeds 16, then excess channels are dropped, and the LEDs next to input ports with dropped channels glow red.

Each input port has a context menu to explicitly set the number of input channels, regardless how many are actually there. In addition there is a module context menu option to set the input channel count for all inputs. By default each input port channel count is set to Auto. The hover tooltip for each input port includes information on the current channel configuration. Mono inputs are replicated to match the specified input channel count. If the specified count is greater than the actual number of input channels, then extra channels are assigned constant 0V. If the specified count is less than the actual input channel count, then the dropped channels LED for that port glows red.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Multi Merge is bypassed.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)

## MULTI SPLIT
![Multi Split module image](MultiSplit.png)  
Split one or more poly inputs into multiple mono or poly outputs.

This split utility is extremely flexible, with many configurations possible. Each polyphonic input can be split into any combination of monophonic and polyphonic outputs. The module has default Automatic behavior for distributing the input channels to the outputs. But the default behavior can be overridden by defining a specific channel count for one or more output ports via output port context menus. The hover tooltip for each output port includes information on the current channel configuration.

The thick red lines indicate which input port is split to which set of output ports. The groupings are defined by which input ports are patched. Unpatched input ports are ignored. The module is automatically reconfigured each time you patch or unpatch an input port. Each patched input distributes its channels to the same output row downward until it reaches another row with a patched input. If no input is patched, then the module treats it as if a monophonic 0V input is patched to the top input.

Multi Split attempts to distribute the input channels evenly to the output ports. When the input channels cannot be divided evenly, higher (lower numbered) output ports take precedence over lower (higher numbered) ports. However, any output channel that is configured for a specific channel count will always output that number of channels. So if you subtract the sum of the fixed output counts from the input count, then the remainder are divided amongst the output ports that are configured for Auto assignment. If the distributor runs out of input channels, then constant 0V is used for the remainder of the output channels.

If the input channels cannot fit within the output channels, then the LED next to the input port glows red. This can only happen if all output ports in the group are configured for a specific channel count, and the sum of the channel counts is less than the input channel count.

This all probably sounds confusing. But once you start patching, it will probably start making sense quickly. Even if you cannot figure out the automatic distrubution algorithm, you can always take control by assigning a specific channel count to each output port.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Multi Split is bypassed.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## POLY CLONE
![Poly Clone module image](PolyClone.png)  
Poly Clone replicates each channel from a polyphonic input and merges the result into a single polyphonic output. It is especially useful with the Recurse modules when using polyphonic inputs. Poly Clone provides a convenient way to replicate channels in polyphonnic CV inputs to match the recursion count.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](x#auxilliary-clone-expander).

### CLONE knob
Selects the number of times to clone or replicate each input channel. Possible values range from 1 to 16.

### POLY input
Each channel from the polyphonic input is replicated based on the Clone count as long as the total replicated channel count does not exceed 16. Channels that cannot be replicated the full amount are ignored.

No input is treated as monophonic constant 0V.

For each channel appearing at the input, the corresponding LED above glows yellow if the channel could be successfully replicated, and red if it could not be replicated. LEDs beyond the input channel count remain off (black).

### GRP (Output grouping) button
The GRP button controls how the cloned channels will be grouped at the output
- **Input channel** ***(yellow, default)*** - All the cloned channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be cloned at the output.

### POLY output
All of the replicated channels are merged into the single polyphonic output. The order of the channels is dependent on the GRP button.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Clone is bypassed then the output is constant monophonic 0 volts.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## POLY PRUNE
![Poly Prune module image](PolyPrune.png)  
Remove selected channels and/or sort channels from a polyphonic input. Higher preserved channels are shifted down to fill gaps left by removed channels.

There are two different stages for choosing which channels to preserve that can be used individually or in series.
- Polyphonic gates CV
- Start channel and Count knobs with CV

There is also an option to sort the channels by voltage before the first stage, between the stages, or at the end.

### Stage 1

#### SELECT GATES polyphonic input
Selects which channels to preserve from the polyphonic input. A high gate on a channel will preserve that channel. A low gate will remove the channel.

Gates are controlled by Schmitt triggers that go high at 2V and go low at 0.2V.

A monophonic Select input will be replicated to match the number of channels in the main input.

A polyphonic Select input with fewer channels than the main input will assume 0V (low gate) for the missing channels. 

Extra Select channels that exceed the main channel count will be ignored.

The Select Gates input is normalled to 10V, so this stage will preserve all input channels if the port is not patched.

If no channels are preserved, then the stage 1 result is a monophonic result with 1 channel at constant 0V.

The total number of channels remaining after stage 1 is listed in the module context menu as a "Selected" count. Note this is the instantaneous value the moment the menu is opened. The value is not updated while the menu is open.

### Stage 2
Selects which channels to preserve from the stage 1 result

#### START knob and CV input
Selects which channel to start with, ranging from 1 to 16. The bipolar CV is scaled at 1 channel per 0.5V, and is additive with the knob value. The final effective start position is clamped to a value between 1 and the number of channels in the stage 1 Select result.

The effective Start value is listed in the module context menu as a "Start" value. Note this is the instantaneous value the moment the menu is opened. The value is not updated while the menu is open.

#### COUNT knob and CV input
Selects how many channels to preserve. The knob value ranges from -16 to 16, with the default noon value at 0 (All). The bipolar CV is scaled at 1 channel per 0.5V, and is additive with the knob value. The final effective count value is clamped between 1 and the number of channels in the stage 1 result.

The effective Count value is listed in the module context menu as a "Count" value. Note this is the instantaneous value the moment the menu is opened. The value is not updated while the menu is open.

A positive effective count begins with the effective Start value and counts up, wrapping back to the 1st channel after reaching the end of the stage 1 result. A negative count begins with the Start value and counts down, wrapping to the last channel after reaching the beginning of the stage 1 result. A value of 0 represents All channels, beginning with the Start value and counting up, wrapping as needed.

### SORT square switch
Controls when and how channels will be sorted
- **Off** ***(default)*** - the channel orders are preserved throughout the entire process
- **In Ascending** - the input channels are sorted by voltage ascending, prior to the stage 1 selection
- **In Descending** - the input channels are sorted by voltage descending, prior to the stage 1 selection
- **SEL Ascending** - the stage 1 selection result is sorted by voltage ascending, prior to perfoming the stage 2 selection
- **SEL Descending** - the stage 1 selection result is sorted by voltage descending, prior to perfoming the stage 2 selection
- **Out Ascending** - the final stage 2 result is sorted by voltage ascending, prior to being sent to the output
- **Out Descending** - the final stage 2 result is sorted by voltage descending, prior to being sent to the output

### IN input
The polyphonic input to be pruned.

### OUT output
The final result after performing stage 2 and any sorting.

### Usage notes
There are many ways in which the Poly Mute can be used. Here are just a few examples.

#### Manually select which channels to preserve, after they have been sorted
Use a Venom Poly Mute to create high gates for each channel you want to preserve. Send the poly input into the Poly Mute IN 1 so you can see which channels are available, and set any channels you want pruned to red, and any channels you want preserved to white. The Out 2 output will have a high gate for each channel you want preserved. Feed the Out 2 output to the Poly Prune Select Gates input.

Set the Start to 1 and the Count to 0 (All)

Patch the original poly input to the Poly Mute IN input. This configuration gives you the option to sort the input channels before the stage 1 selection is applied, so you are effectively selecting sorted channels on the Poly Mute.

#### Select two channels containing the minimum and maximum voltage.
Patch the poly input to the Poly Mute IN input

Leave the Select Gates unpatched.

Set the Start to 1.

Set the Count to -2.

Set the Sort to In Ascending or Sel Ascending.

Assuming there are at least two channels of input, the result will have two channels, the first containing the minimum, and the second containing the maximum.

#### Select all channels between 1V and 3V.
Use Venom WinComp to create high gates for the channels that meet the criteria.
- Patch the poly input into the WinComp A input with 0 Offset
- Set the B offset to 2V
- Set the TOL offset to 1V
- Patch the A=B output to the Poly Mute Select Gates input

Set the Poly Mute Start to 1 and the Count to 0 (All)

Patch the original poly input to a Venom Thru (or any other module) to create a 1 sample delay so the signal remains in sync with the WinComp gates. Patch the delayd signal to the Poly Mute IN input.

If you want the output channels to be sorted, use either the SEL or OUT sorting options.

#### Select the minimum and maximum voltages between 1V and 3V
Patch the Venom WinComp the same as the previous example and send the A=B output to the Poly Mute Select Gates input.

Patch the 1 sample delayed original input to the Poly Mute IN input.

Set the Start to 1 and the Count to -2.

Set the Sort to SEL Ascending.

The final output will have the channels with the minimum and maximum voltages found between 1 and 3 volts. If there is only 1 voltage within the range, there will only be one channel in the output. If no channels are within the range, then the output will be 1 channel at 0V. You can use the WinComp A=B output to detect that none of the voltages are within the range.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Prune is bypassed then the poly input is passed unchanged to the output.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## POLY UNISON
![Poly Unison module image](PolyUnison.PNG)  
Replicate each channel of a polyphonic input with a variable detune spread, and merge the results into a single polyphonic output.

Up to four auxilliary poly inputs may also be cloned via the [Auxilliary Clone Expander](x#auxilliary-clone-expander). Note that auxilliary outputs on the expander are not detuned.

### COUNT (Unison Count) knob
Sets the number of unison channels for each input channel, from 1 to 16.

### COUNT (Unison Count) input
Monophonic bipolar CV modulates the unison count, with each 1/3 volt representing 1 unison voice. The CV is summed with the Count knob value, and the result clamped to the range 1 to 16.

### DETUNE knob
Sets the detune spread for each source channel, measured in semitones. This parameter has no effect if the unison count is 1. The unison voices will be distributed evenly across the spread. The increment between voices = Spread / (Count - 1).

### DETUNE input
Monophonic bipolar input modulates the detune spread. The CV input is scaled so that 10V matches the DETUNE knob range, and then summed with the detune knob value to determine the effective detune spread. A module context menu option is available to use a V/Oct scale for the CV instead.

### DIR (Detune Direction) button
This color coded button specifies how the detune spread is applied to each replication set.
- Off (gray) = Center - The unison voices are divided evenly above and below the input V/oct. The range is (Input - Spread/2) to (Input + Spread/2).
- Green = Up - The unison voices start at the input V/oct and move up. The range is (Input) to (Input + Spread). 
- Red = Down - The unison voices start below the input and end at the input V/oct. The range is (Input - Spread) to (Input).

### RNG (Detune Range) button
This color coded button specifies the range of the detune knob:
- Off (gray) = 1 semitone = 1/12 Volt
- Blue = 1 octave = 12 semitones = 1 Volt
- Green = 5 octaves = 60 semitones = 5 Volts

### POLY input
Each channel from the polyphonic input is replicated based on the unison count as long as the total replicated channel count does not exceed 16. Input channels that cannot be replicated the full amount are ignored.

The absense of input is treated as monophonic constant 0V.

For each channel appearing at the input, the corresponding LED above glows yellow if the channel could be successfully replicated, and red if it could not be replicated. LEDs beyond the input channel count remain off (black).

### GRP (Output grouping) button
The GRP button controls how the replicated channels will be grouped at the output
- **Input channel** ***(yellow, default)*** - All the replicated channels for a given input channel will be grouped together at the output.
- **Input set** ***(blue)*** - The input channels will be grouped together in order as a set, and then the set will be replicated at the output.

### POLY output
All of the replicated channels are merged into the single polyphonic output. The order of the channels is dependent on the GRP button. Detune spread for each input channel goes from low to high (unless the detune CV creates a negative spread)

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Unison is bypassed then the input is passed unchanged to the output.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## RECURSE
![RECURSE module image](Recurse.PNG)  
Uses polyphony to recursively process an input via SEND and RETURN up to 16 times. Polyphonic inputs may be used, which will limit the number of recursion passes to less than 16 for each input channel. There are no limits placed on any of the input or output voltages.

### Recursion Count knob and display

Specifies how many recursion passes should be applied to each input channel. The input channel count mulitiplied by the recursion count must be less than or equal to 16, else some input channels will be dropped.

The display is yellow if all input channels can recurse the requested number of times. The display is red if one or more input channels is dropped.

### Input and Output

The Input is the signal to be recursively processed, and the result is sent to the Output. The number of output channels will match the input unless channels had to be dropped due to channel count limitations during send and return.

### Send and Return

The Send output and Return input channel count can be computed as input channels multiplied by recursion count. The maximum number of input channels is the integral division of 16 divided by recursion count. Input channels that exceed the computed maximum are dropped.

If there are 3 input channels and a recursion count of 5, then Send/Return channels 1-5 will be for input channel 1, 6-10 for input channel 2, and 11-15 for input channel 3. If the recursion count is bumped up to 6 while the input channel count remains 3, then Send/Return channels 1-6 are assigned to channel 1, 7-12 channel 2, and channel 3 is dropped because there are not enough channels remaining to complete 6 recursive passes.

The Return input is normalled to the Send output.

### Internal Modulation

Inputs can be recursively scaled and offset within the RECURSE module itself.

The SCALE input and knob value are multiplied to establish the scale factor. The scale factor is then multiplied by the input value, so RECURSE can perform ring modulation.

The OFFSET input and knob value are added to establish the offset that is added to the input.

By default, the SCALE operation occurs before the OFFSET operation. A context menu option lets you choose to peform the OFFSET before SCALE. A small light glows yellow next to the operation that is performed first.

The SCALE and OFFSET inputs support polyphony. However, only channels that correspond to what appears on the IN input will be used, extra channels will be ignored. Each SCALE or OFFSET channel will be applied to all relevant recursive steps for the corresponding IN input.

The unlabeled Modulation Mode knob determines when the SCALE and OFFSET operations take place. There are 4 values:
- **1Pre** = Once before the first Send only
- **nPre** = Before every recursive Send
- **nPost** = After every recursive Return
- **1Post** = Once after the final Return

Since the Return is normalled to the Send, it is possible to generate a polyphonic series of constant voltages using only the RECURSE module. For example, leave all inputs and the Return unpatched, set the Recursion Count to 16, the Scale to 1, the Offset to 1V, and the Mode to nPre. The SEND output will have 16 channels of integral values from 1 to 16. Change the Mode to nPost and the values will range from 0 to 15.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The Input is passed unchanged to the Output when RECURSE is bypassed. The SEND will be monophonic 0V.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## RECURSE STEREO
![Recurse Stereo module image](RecurseStereo.PNG)  
Recurse Stereo is identical to [Recurse](x#recurse) except the Input/Return inputs and Output/Send outputs are doubled to support left and right channels of a stereo pair.

The number of input polyphonic channels is strictly controlled by the Left Input. Any extra channels in the Right Input are ignored.

The Right Input is normalled to the Left Input.

In addition, The Left Return is normalled to the Left Send, and the Right Return is normalled to the Right Send.

The Recursion Count, Scale, Offset, and Modulation Timing settings are applied to both Left and Right identically.

Both left and right inputs are passed unchanged to the outputs when RECURSE STEREO is bypassed. The right input remains normalled to the left input while bypassed. Bypassed left and right send are monophonic 0V.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## SPLIT 4x2
![Split 4x2 module image](Split4x2.png)  
Split 4x2 is a compact and versatile polyphonic split utility that can separate one or two polyphonic inputs into multiple monophonic or polyphonic outputs.

There are two independent sections, each capable of splitting one input into four outputs. If the bottom input is not patched, then the two sections are merged, thus allowing the top input to be split into up to eight outputs.

By default all outputs are monophonic. Each output port has its own context menu option where you can specify any number of output channels between 1 and 16. There is also a module level context menu to reset all outputs to monophonic.

A glowing red LED next to an input port indicates there is at least one polyphonic input channel that could not be split into one of the outputs.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Split 4x2 is bypassed.

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## SPREAD MERGE
![Spread Merge module image](SpreadMerge.png)  

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


## SPREAD MERGE EXPANDER
![Spread Merge Expander module image](SpreadMergeExpander.png)  

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)

## STEREO MERGE SPLIT
![Stereo Merge Split module image](StereoMergeSplit.png)  
Compact and versatile polyphonic merge and split utility for stereo signals

[Polyphony Management top](#polyphony-management)|[Venom top](/README.md#venom)


