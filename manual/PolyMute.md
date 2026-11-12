Mute individual channels of two polyphonic inputs via buttons or CV.

### Polyphony channel count
The module context menu has a "Polyphony Channel count" option that determines the number of polyphony channels at each output.

The default value of "Auto" computes the output channel count as the maximum channel count found across the IN 1 and IN 2 polyphonic inputs. If neither input is patched than the output channel count is 16.

A value between 1 and 16 sets the output channel count directly, regardless how many channels are at the inputs.

### Channel buttons 1 - 16
There is a button for each potential channel of a polyphonic cable. Active channel buttons are brightly lit, inactive channel buttons (button number higher than the output channel count) are dimly lit.

Red buttons indicate muted channels. White buttons indicate Pass (un-muted) channels.

The behavior of the buttons change depending on whether the Gates CV input is patched and also whether the Gates mode is "Exclusive pass".

Assuming the mode is anything other than "Exclusive pass", then:
- Unpatched Gates CV input - Each button press toggles the state of that channel
- Patched Gates CV input - Each button press temporarily inverts the state of the channel while the button remains pressed

If the Gates mode is "Exclusive pass" then these buttons function as radio buttons. Whichever button is pressed last passes that channel, and all others are muted.

### GATES polyphonic input
Controls the state of each channel if and only if the port is patched. The behavior of a gate is dependent on the current Mode setting.

Gates are Schmitt triggers that go high at 2V and go low at 0.2V

Missing channels are assumed to be 0V (constant low gate).

### MODE (Gate mode) button
Defines the behavior of polyphonic Gate inputs.
- **Mute** ***(orange, default)*** - High gate mutes the channel. Low gate passes the channel.
- **Pass** ***(green)*** - High gate passes the channel. Low gate mutes the channel.
- **Toggle** ***(yellow)*** - Leading edge of a high gate toggles the state of the channel.
- **Exclusive pass** ***(blue)*** - The Gates input is ignored. Buttons have exclusive control over which channel is passed.

### SOFT button
Controls whether mute actions are instantaneous or slewed.

- **Off** ***(dark gray, default)*** - mute actions are instantaneous, which is generally good for CV
- **On** ***(yellow)*** - mute actions are slewed, taking 100 msec to fade in or out. Good for audio to prevent pops when muting or unmuting.

### IN 1 and IN 2 polyphonic inputs
The polyphonic inputs to be selectively muted.

A monophonic input with only one channel is automatically replicated to match the output channel count.

Missing channels in a polyphonic input are assumed to be constant 0V. Extra channels are ignored.

The inputs are normalled to 10V so Poly Mute can be used to conveniently generate control gates for Poly Prune without any need for an input.

The two inputs can be used for stereo processing. Or if only one input is patched, the corresponding output will have the muted results, and the 2nd output will have high gates for the preserved channels.

### OUT 1 and OUT 2 polyphonic outputs
For each un-muted channel the input value is copied to the output. For muted channels the output is set to 0V.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Mute is bypassed then the poly inputs are passed unchanged to the outputs. The Soft button has no effect when bypassing or un-bypassing the module.
