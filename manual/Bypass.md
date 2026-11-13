Bypass (disable) one or more modules at the end of patched cables via CV control or a manual button press.

The Bypass module has input/output pairs that simply pass through all channels appearing at the input. However, the Bypass module also has the ability to virtually transmit a message through each cable to bypass the module (or group of modules) at the end of the cable.

The Bypass module maintains its own state, indicating whether it thinks remote modules are bypassed or not. The actual bypass state of the remote module may or may not match the Bypass state because users can directly change the remote module bypass state without using the Bypass module. The Bypass module only changes the remote module bypass state when the Bypass module state changes.

### TRIG (Trigger) button
This button glows red when the Bypass state is active (meaning the controlled modules are disabled).

By default, each time the button is pressed, it toggles the current Bypass state.

If the Momentary button is active, then the button temporarily inverts the current Bypass state while it is held, and restores to the previous state when released.

### M (Button Momentary mode) button
This color coded button controls whether the trigger button acts as a toggle trigger, or a momentary inverting gate.
- **Off (gray)** (default) - The button toggles the Bypass state each time it is pressed
- **On (yellow)** - The button momentarily inverts the current Bypass state while it is pressed, and reverts to the prior state when released

### G (CV Gate mode) button
This color coded button controls whether trigger CV input acts as a gate or a toggle trigger.
- **Off (gray)** (default) - Input is a toggle trigger
- **On (green)** - Input is a gate
- **Invert (red)** - Input is an inverted gate

### TRIG (Trigger) CV input

If the Gate mode is Off, then the leading edge of a trigger at the CV input will toggle the bypass state on or off.

If the Gate mode is On, then a high gate at the input turns the bypass on, and a low gate turns the bypass off. The gate functions as a Schmitt trigger, going high at 2V, and low at 0.1V. Regardless whether the CV gate is high or low, the Trig button will always toggle the Bypass state.

If the Gate mode is Invert, then a high gate at the input turns the bypass off, and a low gate turns the bypass on. The gate functions as a Schmitt trigger, going high at 2V, and low at 0.1V. Regardless whether the CV gate is high or low, the Trig button will always toggle the Bypass state.

### Pass Thru input/output pairs

There are three input/output pairs that simply pass any input values through to the output, with full support for polyphonic cables.

Each output port is a bit unusual in that any patched cable will have 0 channels if the corresponding Bypass input port is unpatched. This allows for Bypass to control the bypass state of remote modules, without disturbing any normal value for the remote module input port. The remote module will act as though the cable is not there, yet Bypass will still be able to bypass the remote module.

### Port Bypass mode buttons
Each input port has a Bypass mode button above it, and each output port has a Bypass mode button below it. The Bypass mode dictates whether the Bypass module controls the bypass state of the module(s) at the end of patched cables at either port. The Bypass mode buttons are color coded. 

Each input Bypass mode button has the following available options:
- **Dark gray** (default) - Off  (no modules are bypassed)
- **Purple** - Source (Only the input source modules are bypassed)
- **Blue** - Source and left neighbors (The input source modules and all contiguous neighbors to the left are bypassed)
- **Yellow** - Source and right neighbors (The input source modules and all contiguous neighbors to the right are bypassed)
- **Green** - Source and all neighbors (The input source modules and all contiguous neighbors on both sides are bypassed)

Each output Bypass mode button has the following options:
- **Dark gray** - Off  (no modules are bypassed)
- **Purple** (default) - Target (Only the output target modules are bypassed)
- **Blue** - Target and left neighbors (The output target modules and all contiguous neighbors to the left are bypassed)
- **Yellow** - Target and right neighbors (The output target modules and all contiguous neighbors to the right are bypassed)
- **Green** - Target and all neighbors (The output target modules and all contiguous neighbors on both sides are bypassed)

Every time the Bypass changes state, the relevant remote modules at the end of each cable are set appropriately.

### Limiting neighbor chains
A neighbor chain always terminates at any one of the following
- A gap in the chain
- A Bypass module. The Bypass is not part of the chain.
- A Venom [Blocker](Blocker.md). The Blocker is not part of the chain.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.
