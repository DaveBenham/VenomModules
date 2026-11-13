Computes V/Oct CV for musical ratios using integral numerators and denominators. Integral ratios are particularly useful for generating harmonious tones with linear Frequency or Phase modulation.

Keeping the Denominator at 1 produces standard integral harmonics. Keeping the Numerator at 1 produces integral subharmonics. But other ratios like 3/2 can also produce pleasing tones.

This module is fully polyphonic. The number of output channels is the maximum channel count found across all inputs. Any monophonic input is replicated to match the output channel count. A polyphonic input with fewer channels uses 0V for any missing channels.

### Upper NUM (Numerator) section
The upper section controls the ratio numerator.

The medium knob at the top sets the base value, ranging from 1 to 128, with the default of 1.

The CV input is scaled at 1 per every 0.1V, and can be attenuated and/or inverted by the small attenuverter.

The small QUANT knob quantizes the CV to multiples of 1, 2, 3, 4, 5, 6, 7, or 8, with the default value at 1.

The LED numeric display shows the effective numerator after CV is applied. The maximum effective numerator is 199.

### Lower DEN (Denominator) section
The lower section controls the ratio denominator.

The medium knob at the top sets the base value, ranging from 1 to 128, with the default of 1.

The CV input is scaled at 1 per every 0.1V, and can be attenuated and/or inverted by the small attenuverter.

The small QUANT knob quantizes the CV to multiples of 1, 2, 3, 4, 5, 6, 7, or 8, with the default value at 1.

The LED numeric display shows the effective denominator after CV is applied. The maximum effective denominator is 199.

### ROOT input
The Root CV is added to the ratio value to obtain the final V/Oct value. The Root is normalled to 0V.

### OUT output
The final V/Oct CV after the Root has been added.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

The Root CV is passed through unchanged to the Output if Ratio is bypassed.
