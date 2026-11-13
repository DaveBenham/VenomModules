Merges multiple sets of monophonic gate/CV pairs into a single monophonic stream of gates with associated CV. This can be useful for extracting a single melodic line from multiple sources.

The GATE output is the logical OR of all Gate inputs. If the CLOCK input is not patched, then each Gate input is a simple Schmitt trigger that goes high above 2V and low below 0.2V.

If the CLOCK input is patched, then the Gate transitions are restricted to when the Clock transitions to high. In other words, the Gates function as Sample & Hold inputs. Each time the Clock transitions to high (another Schmitt trigger) the current state of the Gate is sampled and held until the next Clock high transition. This is useful for obtaining output Gates that are synced to a timing grid.

The top square button controls how frequently the CV output is updated.
- **S&H (Sample & Hold)** - The output CV is sampled when the output Gate transitions to high, and then held until the next output Gate high transition.
- **T&H (Track & Hold)** - The output CV is continuously updated while the output Gate remains high, and then maintains its last value while the output Gate is low.

The bottom square button controls how the output CV value is selected from among all the CV inputs. Each CV input is associated with the Gate at the same vertical level. A given CV input is only eligible for selection if the corresponding Gate input is in a high state.
- **First** - The first eligible CV is selected, ordered from top to bottom.
- **Last** - The last eligible CV is selected, ordered from top to bottom.
- **Min (Minimum)** - The minimum eligible CV value is selected.
- **Max (Maximum)** - The maximum eligible CV value is selected.
- **Avg (Average)** - The output CV is the average of all eligible CV inputs.
- **Sum** - The output CV is the sum of all eligible CV inputs.

Additional CV channels may be added by placing one or more [Linear Merge Expander](LinearMergeExpander.md) modules to the right of Linear Merge. There is an "Add CV expander" context menu option available. There is no limit to the number of expanders that can be added.

[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

If Linear Merge is bypassed, then all outputs are constant monophonic 0V.
