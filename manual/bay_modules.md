[Bay Input](BayInput.md), [Bay Norm](BayNorm.md), and [Bay Output](BayOutput.md) are polyphonic transmitter and receiver modules that communicate with each other without using patch cables. The modules provide for user defined labels to make clean, self documenting patch bays. They are a great companion for the MindMeld PatchMaster user interface modules.

None of the Bay modules are particularly useful on their own - Each Bay Input should be paired with at least one Bay Output and/or Bay Norm. Each input on a Bay Input is transmitted to the corresponding output on the Bay Output or Bay Norm. Each Bay Norm output has a corresponding Normal input that is used when the source Bay Input is not patched.

Signal Transmission is instantaneous - there is no sample delay introduced between a Bay Input and the linked Bay Output/Norm.

### Module Name / Label
Each Bay module has a context menu option to specify a unique name for the module instance that appears as a label at the top of the module.

### Output Source
Bay Output and Bay Norm each have a context menu option to specify the Bay Input source for that instance. A Bay Output or Bay Norm can only have a single source. But a Bay Input can be the source for multiple Bay Outputs/Norms

The Bay Input source is identified by the numeric VCV module instance ID, shown within parentheses in the context menu. The user defined Bay Input name is displayed before the numeric ID, and makes it easier to keep track of which Input is linked to which Output/Norm. Changing the name of a Bay Input does not break the link.

### Port Names / Labels
Each port on a Bay Input/Output/Norm can be given a user defined name via the [standard Venom port context menu](plugin.md#custom-names). The port name is displayed as a label above the port, except the Bay Norm input name does not appear as a label - it only appears as hover text.

#### Bay Input default port names
The factory default input port name is always "Port " followed by the port number.

#### Bay Output, Bay Norm default output port names
The factory default output port name depends on whether the module has been linked to a source:
- Linked default: Inherits the current name from the source Bay Input port
- Unlinked default: "Port " followed by the port number

#### Bay Norm default normal input port names
The factory default is always the current output port name with "normal" appended. The name only appears as hover text.

### 0 Channel outputs
Bay Output and Bay Norm have an "Enable 0 Channel output" context menu option. If this option is enabled, then a Bay Output output will have 0 channels if the source Bay Input is not patched or there is no link. A Bay Norm output will have 0 channels if both the source input and the normal input are not patched or if there is no link.

If the option is not enabled then the output would be constant monophonic 0 volt instead.

Cables with 0 channels act as though there is no patch cable at all, so normalled inputs at the destination input are preserved.

### VCV Rack Pro within a DAW
The Bay modules work fine when used with Rack Pro running as a plugin within a DAW. However, Bay Output and Bay Normal can only link to Bay Input sources within the same plugin instance. They cannot link across multiple plugin instances, whether they be in the same or different tracks.

### Recommended configuration
#### Patch Bay Input
Place the Bay Input in your patch bay with a name of "Input n" where n is a sequential number.

Give each of the Bay Input ports a descriptive name designating the use for the port.

Place the destination Bay Norm in the "operational guts" of your patch, and give it a name of "Target n".

Map the Target n source to the corresponding "Input n" in your patch bay.

Leave all Bay Norm ports with factory default names so the labels are automatically inherited from the source.

Patch the output ports to the correct inputs in your operational patch. Patch the default values to the normal inputs.

#### Patch Bay Output
Place a Bay Input near the operational guts of your patch and give it a name of "Source n", where n is a sequential number.

Give each of the "Source n" ports a descriptive name designating the use of the patch bay output.

Patch each signal source in your operational patch to the appropriate input port in your "Source n"

Place a Bay Output in your patch bay and give it a name of "Output n" where n matches the number of the source.

Map the output to the corresponding "Source n" in your operational patch.

Leave all patch bay output ports with their factory default names so they inherit the name from the source.

### Selection dulicate/paste/import behavior

Custom module and port names are always preserved when duplicating, pasting or importing a selection set.

If duplicating, pasting, or importing a linked pair of modules, then the output modules are linked to the copied sources.

If duplicating, pasting, or importing linked output modules without the sources, then an attempt is made to link the outputs to the original uncopied sources.

### Bypass Behavior

No values are teleported from a Bay Input to a Bay Output/Norm if either module is bypassed.

All Bay Output ports will be constant monophonic 0 volts if the Bay Output is bypassed or the source Bay Input is bypassed.

The Bay Norm output will be the normal input if the source Bay Input is bypassed. The Bay Norm output will be monophonic constant 0 volts if the Bay Norm is bypassed.
