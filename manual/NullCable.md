Nullify a cable by setting it to 0 channels via manual or CV control. A cable with 0 channels functions as if the cable is not even there. This is different than a mute where the output is set to constant 0 volts.

The module has three identical sections that function independently, each with a Gate button and associated monophonic Gate CV input at the top, a polyphonic input in the middle, and polyphonic output at the bottom.

All channels at the polyphonic input are copied to the output whenever the gate button glows white. The output is nullified (set to 0 channels) whenever the gate button glows red.

The behavior of the Gate button and Gate CV input depends on the value of the Gate Mode in the module context menu:

|Gate Mode|High CV gate|Low CV gate|Button without CV input|Button with CV input|
|---|---|---|---|---|
|Nullify (default)|Nulls output|Passes output|Toggles state|Inverts state while held|
|Pass|Passes output|Nulls output|Toggles state|Inverts state while held|
|Toggle|Toggles state|No change|Toggles state|Toggles state|

CV gates are Schmitt triggers that go high at 2V and go low at 0.2V.

Note that the output will always be a null cable (0 channels) if there is no input.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

When Null Cable is bypassed the outputs continue to function as they did before the module was bypassed.
