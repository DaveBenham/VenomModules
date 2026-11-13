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
Use a Venom [Poly Mute](PolyMute.md) to create high gates for each channel you want to preserve. Send the poly input into the Poly Mute IN 1 so you can see which channels are available, and set any channels you want pruned to red, and any channels you want preserved to white. The Out 2 output will have a high gate for each channel you want preserved. Feed the Out 2 output to the Poly Prune Select Gates input.

Set the Start to 1 and the Count to 0 (All)

Patch the original poly input to the Poly Mute IN input. This configuration gives you the option to sort the input channels before the stage 1 selection is applied, so you are effectively selecting sorted channels on the Poly Mute.

#### Select two channels containing the minimum and maximum voltage.
Patch the poly input to the Poly Prune IN input

Leave the Select Gates unpatched.

Set the Start to 1.

Set the Count to -2.

Set the Sort to In Ascending or Sel Ascending.

Assuming there are at least two channels of input, the result will have two channels, the first containing the minimum, and the second containing the maximum.

#### Select all channels between 1V and 3V.
Use Venom [WinComp](WinComp.md) to create high gates for the channels that meet the criteria.
- Patch the poly input into the WinComp A input with 0 Offset
- Set the B offset to 2V
- Set the TOL offset to 1V
- Patch the A=B output to the Poly Prune Select Gates input

Set the Poly Prune Start to 1 and the Count to 0 (All)

Patch the original poly input to a Venom [Thru](Thru.md) (or any other module) to create a 1 sample delay so the signal remains in sync with the WinComp gates. Patch the delayd signal to the Poly Mute IN input.

If you want the output channels to be sorted, use either the SEL or OUT sorting options.

#### Select the minimum and maximum voltages between 1V and 3V
Patch the Venom WinComp the same as the previous example and send the A=B output to the Poly Prune Select Gates input.

Patch the 1 sample delayed original input to the Poly Prune IN input.

Set the Start to 1 and the Count to -2.

Set the Sort to SEL Ascending.

The final output will have the channels with the minimum and maximum voltages found between 1 and 3 volts. If there is only 1 voltage within the range, there will only be one channel in the output. If no channels are within the range, then the output will be 1 channel at 0V. You can use the WinComp A=B output to detect that none of the voltages are within the range.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Prune is bypassed then the poly input is passed unchanged to the output.
