Compact and versatile polyphonic merge and split utility for stereo signals.

Stereo Merge Split is a simplified version of [Merge Split](MergeSplit.md) that is optimized for ease of use. It enables polyphonic modules without built in stereo support to be used with stereo signals.

### Top Merge section
The top section merges monophonic or polyphonic Left and Right inputs into a single Stereo polyphonic output. The left and right inputs can each have up to 8 channels of polyphony, yielding up to 16 channels in the output.

The Right input is normalled to the Left input. If either input has fewer channels than the other, than it is padded with 0V channels to match. The Left input channels are then copied to the Stereo output, followed by the Right channels. Only the first eight channels of each input can be copied. If either input has more than 8 channels, then the extra channels are ignored, and the LED below and to the right of the output glows red to indicate an error.

The Stereo output is typically patched to the polyphonic input of your module that does not have built in stereo support.

### Bottom Split section
The bottom section splits a polyphonic Stereo input into monophonic or polyphonic Left and Right outputs.

The Stereo input channels are split in half, with the first half going to the Left output, and the second half going to the Right output. If the Stereo input has an odd number of channels, then the extra channel goes to the Left output, and the Right output is padded with an extra 0V channel so the left and right channel counts match.

The Stereo input usually comes from a polyphonic output of your module that does not have built in stereo support.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass
All outputs are monophonic 0V if Stereo Merge Split is bypassed.
