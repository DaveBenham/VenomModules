Extend context menus to support parameter/port renaming and parameter custom defaults for non-Venom modules.

Custom names and defaults are stored with the patch and restored upon patch load as long as the Widget Menu Extender remains with the patch.

Factory names and defaults are restored whenever the Widget Menu Extender is removed from the patch.

Note that custom names and defaults are built into all of the Venom plugin modules (except for Rhythm Explorer). Widget Menu Extender brings that functionality to non-Venom modules (as well as Rhythm Explorer). The Venom module custom names and defaults are mainained independently from Widget Menu Extender.

### ENABLE button
Controls whether extended context menus are enabled or not. The button is bright blue when extended menus are On.

Extended context menu options will only be available if the Enable button is On.

Any existing custom names and defaults remain in effect when the extended menus are Off.

### Custom Names
When enabled, the context menu for every foreign input port, output port, and parameter control is extended with an option to rename the parameter or port with a custom name. Once set, the custom name only appears in hover text and context menus - it does not update the module faceplate.

If a parameter or port has been given a custom name, then an additional menu option is added to restore the factory name.

If a module dynamically updates the parameter or port name, then that overrides any custom name from Widget Menu Extender.

Do not include "input" or "output" in your custom port name - VCV will automatically append input or output to the name you provide.

### Custom Defaults
When enabled, the context menu for every foreign parameter control is extended with an option to set the default value to the current value. The parameter is set to the default value whenever it is initialized, whether by double click, or menu option, etc.

If a custom default value has been established, then an additional menu option is added to restore the factory default value.

If a module dynamically updates the parameter default value, then that overrides any custom default value from Widget Menu Extender.

### Multiple Instances
Only one instance of Widget Menu Extender can be active per patch. If another instance is inserted, then it will be permanently disabled and the Enable button will be bright red. Since permanently disabled Widget Menu Extenders serve no purpose, they probably should be deleted from the patch.

If using VCV Pro within a DAW, each instance of the VCV plugin is regarded as a separate patch, and can have its own active Widget Menu Extender.

When importing a selection set containing Widget Menu Extender, custom names and defaults are preserved as long as the patch does not already have an active Widget Menu Extender. But if an active Widget Menu Extender already exists, then the selection set Widget Menu Extender will be permanently disabled, and selection set custom names and defaults will be lost.

### Bypass
When bypassed, Widget Menu Extender behaves the same as if the Enable button is off - the extended context menu options will not be available, but existing custom names and defaults are preserved.
