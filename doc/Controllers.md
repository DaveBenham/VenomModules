# Controllers

- [KNOB 5](#knob-5)
- [MOUSE PAD](#mouse-pad)
- [POLY MUTE](#poly-mute)
- [POLY OFFSET](#poly-offset)
- [PUSH 5](#push-5)

[Venom top](/README.md#venom)

## KNOB 5
![Knob 5 module image](Knob5.png)  
Five independently configurable constant voltage knobs.

### Individual Knob configuration
Each knob has custom menu options to tailor the knob to your needs.

#### Knob Range
Determines the minimum and maximum voltage of the knob.
- **0-1 V**
- **0-2 V**
- **0-5 V**
- **0-10 V**
- **+/- 1 V**
- **+/- 2 V**
- **+/- 5 V**
- **+/- 10 V** (default)
- **Custom**

If **Custom** is chosen, then two menu options are added to set the custom range: **Custom min** for the full counter-clockwise level, and **Custom max** for the full clockwise level. The min and max values can range from -100 to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom levels. You are free to set the min greater than the max so the knob works in reverse.

#### Quantize
Determines how knob values are quantized.
- **Off (continuous)** (default)
- **Integers (octaves)**
- **1/12 V (semitones)**
- **Custom interval**

If **Custom interval** is chosen, then a **Custom quantize interval** menu option is added to set the custom interval. The interval can be any positive value up to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom interval. If you enter a value less than or equal to zero, then the value will be transformed into 1.

#### Display unit
Determines how knob values are displaed and entered in knob context menu and hover text. Output values are always in Volts.
- **Volts (V)** (default)
- **Cents (&cent;)**

#### Polyphony channels
Determines the number of polyphonic channels to output. All channels will be identical. The default is 1 (mono).

 
### Global Knob configuration
The module context menu includes options to configure all knobs simultaneously. The option values are the same as for individual knobs except custom values are not supported. You must configure each knob individually if you want custom values.

If all knobs currently share the same value, then the current value is displayed in the menu. If at least one knob is different, then the current value is empty.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus. However, the rename function is modified slightly. Renameing a knob will automatically rename the corresponding output port, and vice versa.

### Bypass

All outputs are constant monophonic 0 V when KNOB 5 is bypassed.

[Controllers top](#controllers)|[Venom top](/README.md#venom)

## MOUSE PAD
![Mouse Pad module image](MousePad.png)  
Use your mouse as a controller, without having to point at any specific module. Activate the Mouse Pad output by pressing and holding some combination of Shift, Ctrl, and/or Alt (Option on Mac). A 10V gate is generated while active, and there are two indpendent CV outputs that track the mouse movement. The horizontal axis controls the X output, and vertical the Y.

### Activate section

The top four buttons configure how the Mouse Pad is activated. At least one of Shift, Ctrl, and/or Alt must be enabled to use Mouse Pad. Whichever buttons are activated, that specifies what key combination is required to track movement and produce output. The Mouse Pad stops tracking motion when at least one of the keys is released.

The Tog (toggle) button changes behavior. The first combination press activates, and it remains active after release. The next combination press deactivates tracking.

### X / Y sections

Both X and Y have 4 controls to configure the output.

The Scale knobs amplify, attenuate and/or invert the mouse sensitivity. At 1 both the X and Y have a 10V range.

The Origin knobs determine what part of the VCV Rack window corresponds to 0V for X and Y. Values of 0%, 0% correspond to the lower left corner of the window. Values of 100%, 100% correspond to the the upper right corner. So a value of 50%, 50% corresponds to the center of the window.

The Absolute buttons control whether the output is absolute, or relative to the mouse position at the moment of activation. If enabled then the Origin knobs establish the origin. If not enabled then the Origin knobs are ignored.

The Return buttons control whether the outputs return to 0 upon release of the activation keys, or if the last value is held until the next activation.

### Outputs

All outputs are monophonic.

Gate is high (10V) whenever the mouse is tracking, and low (0V) when not tracking.

X output follows the horizontal motion of the mouse, and Y the vertical motion. Both values have a 10V range when Scale is at 100%

### Multiple Mouse Pads

Multiple Mouse Pads can be effectively used if each is given a different combination of activation keys. This allows you to use a single mouse to send user controlled CV to different parts of your patch on demand.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V if Mouse Pad is bypassed.

[Controllers top](#controllers)|[Venom top](/README.md#venom)


## POLY MUTE
![Poly Mute module image](PolyMute.png)  
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
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Mute is bypassed then the poly inputs are passed unchanged to the outputs. The Soft button has no effect when bypassing or un-bypassing the module.

[Controllers top](#controllers)|[Venom top](/README.md#venom)


## POLY OFFSET
![Poly Offset module image](PolyOffset.png)  
Provides an offset control for each channel of a polyphonic signal. For each polyphonic output channel, the channel's knob voltage is added to the input voltage to get the final output voltage.

### Offset knobs
There is one offset knob for each of the possible polyphonic channels. The default (initialize) value for all knobs always starts out at 0 volts. Of course the default can be overriden by the standard Venom parameter context menu option.

The module context menu has the following options to configure the behavior of the knobs:

#### Knob Range
Determines the minimum and maximum voltage of the knobs.
- **0-1 V**
- **0-2 V**
- **0-5 V**
- **0-10 V**
- **+/- 1 V**
- **+/- 2 V**
- **+/- 5 V**
- **+/- 10 V** (default)
- **Custom**

If **Custom** is chosen, then two menu options are added to set the custom range: **Custom min** for the full counter-clockwise level, and **Custom max** for the full clockwise level. The min and max values can range from -100 to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom levels. You are free to set the min greater than the max so the knobs work in reverse.

#### Quantize
Determines how values are quantized.
- **Off (continuous)** (default) - Neither the output nor the knob offset values are quantized.
- **Output to Integers (octaves)** - The sum of input voltage plus knob value is quantized to the nearest integral Volt.
- **Output to 1/12 V (semitones)** - The sum of input voltage plus knob value is quantized to the nearest 1/12 Volt.
- **Output to custom interval** - The sum of input voltage plus knob value is quantized to the nearest multiple of an interval that you specify.
- **Offset to Integers (octaves)** - The knob offset value (and display value) is quantized to the nearest integral Volt.
- **Offset to 1/12 V (semitones)** - The knob offset value (and display value) is quantized to the nearest 1/12 Volt.
- **Offset to custom interval** - The knob offset value (and display value) is quantized to the nearest multiple of an interval that you specify.

If either **custom interval** option is chosen, then a **Custom quantize interval** menu option is added to set the custom interval. The interval can be any positive value up to 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom interval. If you enter a value less than or equal to zero, then the value will be transformed into 1.


#### Display unit
Determines how knob values are displayed and entered in knob context menu and hover text. Output values are always in Volts.
- **Volts (V)** (default)
- **Cents (&cent;)**

### Output polyphonic channel count
By default the number of output channels matches the number of input channels. Knobs for channels above the output count are ignored.

There is a "Polyphony channels" option in the module context menu that lets you override the default and select a specific output channel count. Input channels and knobs above the specified channel count are ignored. Monophonic inputs are cloned to match the selected channel count. If the input channel count is poly but less than the selected channel count, then missing channel inputs are assumed to be constant 0 volts, meaning the knob alone specifies the output voltage.

### Channel count display
The number of polyphonic channels at the output is displayed in the LED panel. The display will be yellow if the number of output channels is greater than or equal to the input channel count. The display will be red if the selected channel count is less than the input channel count.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

If Poly Offset is bypassed then the input is passed unchanged to the output.

[Controllers top](#controllers)|[Venom top](/README.md#venom)


## PUSH 5
![Push 5 module image](Push5.png)  
Five independently configurable push buttons.

### Individual Button configuration
Each button has custom menu options that allow you to tailor the button to your needs

#### Button Mode
- **Trigger** - A 1 msec On value trigger is output each time the button is pressed.
- **Gate (default)** - The On value is output while the button is pressed.
- **Toggle** - The button changes state each time the button is pressed. Toggle button states are stored with patches, selection sets, and presets, and are restored when the patch, selection set, preset is loaded.

#### On value
- **10 V (default)**
- **5 V**
- **1 V**
- **0 V**
- **-1 V**
- **-5 V**
- **-10 V**
- **Custom**

If **Custom** is chosen, then a **Custom ON value** menu option is added that lets you type in any value between -100 and 100. You can enter simple mathematical expressions - all of the keyins that work for setting a knob value also work for the custom value.

#### Off value
Same values as On except the default is 0 V and the **Custom** value adds a **Custom OFF value** menu option.

#### On Color
- **Red**
- **Yellow**
- **Blue**
- **Green**
- **Purple**
- **Orange**
- **White (default)**
- **Dim Red**
- **Dim Yellow**
- **Dim Blue**
- **Dim Green**
- **Dim Purple**
- **Dim Orange**
- **Dim Gray**
- **Off**

#### Off Color
Same values as Off except the default is Dim Gray

#### Polyphony channels
Determines the number of channels to output. All channels will be identical. The default is 1 (mono).

### Global Button configuration
The module context menu includes options that configure all buttons simultaneously. The options and values are the same as for individual buttons, except Custom values are not supported. You must configure each button individually if you want custom values.

If all buttons currently share the same value, then the current value is displayed in the menu. If at least one button is different then the current value is empty.

### Standard Venom Context Menus
[Venom Themes](/README.md#themes), [Custom Names](/README.md#custom-names), and [Parameter Locks and Custom Defaults](/README.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus. However, the rename function is modified slightly. Renameing a button will automatically rename the corresponding output port, and vice versa.

### Bypass

All outputs are constant monophonic 0 V when PUSH 5 is bypassed.

[Controllers top](#controllers)|[Venom top](/README.md#venom)


