Spread each monophonic gate/CV pair across multiple polyphonic channels and merge all into one polyphonic output for the gates and another polyphonic output for the CV. The goal is to give time for envelope release stages to complete before the envelope gets retriggered by the next gate.

There is an "Add CV expander" context menu option to add one or more [Spread Merge Expander](SpreadMergeExpander.md)s to the right. There is no limit to the number of expanders that can be added.

### SPREAD COUNT square button
Specifies the number of poly channels to allocate for each input. The value can range from 2 to 16. The Spread Count times the number of patched Gate inputs must not exceed 16, otherwise the LED below the button glows red indicating an error.

### MODE square button
Specifies how the CV inputs are sampled.
- **S&H (Sample & Hold)** - The CV input is sampled when the corresponding gate transitions to a high state, and then is held constant until the next gate is received.
- **T&H (Track & Hold)** - The CV input is continuously updated while the corresponding gate remains high. Once the gate goes low the last CV value is held constant until the next high gate.

### GATE and CV inputs
There are 8 pairs of monophonic Gate and CV inputs. The Gate/CV pairs are ordered starting from the top. Patched CV is ignored if it does not have a corresponding Gate input patched.

### GATE and CV outputs
If the Spread Count is n, then the first n output channels are reserved for the first input pair. The second n output channels are for the second input pair, etc.

Say input ports 1 and 2 are patched and the Spread Count is 3. The first gate received for port 1 is assigned to Gate output channel 1, and the sampled CV is assigned to CV output channel 1. The second port 1 gate received and associated CV are assigned to output channels 2. The third pair are assigned to channels 3. After that the channels wrap so the fourth pair are assigned to channel 1 again. The gate and CV values for input ports 2 are distributed across output channels 4 through 6.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are monophonic 0V if Spread Merge is bypassed.
