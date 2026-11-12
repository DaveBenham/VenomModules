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
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus. However, the rename function is modified slightly. Renameing a button will automatically rename the corresponding output port, and vice versa.

### Bypass

All outputs are constant monophonic 0 V when PUSH 5 is bypassed.
