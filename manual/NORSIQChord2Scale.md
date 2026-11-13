Converts up to 14 channels of a polyphonic "chord" input into a set of CV outputs that define a scale for the [Non-Octave-Repeating Scale Intervallic Quantizer](NORS_IQ.md).

### General Operation
Polyphonic V/Oct channels at the CHORD input are sorted, and the lowest note defines the scale root, the number of channels determines the scale length, and the intervals between the sorted notes define the intervals for the NORSIQ. The computation of the defining scale CV may be continuous, or it may be held constant until a trigger is received.

#### NORSIQ configuration
The NORSIQ module must be configured properly for the CV from the NORSIQ Chord To Scale module to define the correct scale:
- The EQUAL DIVS button must be off so that intervals can be defined directly.
- All 14 Interval knobs must be fully counter-clockwise (0V, 0 cents, or 1:1 ratio)
- The SCALE LENGTH knob must be fully counter-clockwise (1)
- The SCALE ROOT should be at the default noon position (C4), unless you want to transpose the scale to a different key.
  
The PSEUD-OCT INTERVAL and EDPO knobs and inputs are ignored. The INTERVAL UNIT, ROOT UNIT, ROUND, and EQUI LIKELY contols may configured as you see fit.

The NORSIQ has a "NORSIQ Chord To Scale configuration" factory preset that quickly sets the appropriate configuration with some parameters locked to make sure it works properly.

### OCTAVE FOLD button
Controls where the scale repeats
- **Off (gray - default)**: The last (highest) note of the sorted chord input defines the last note of the scale, which becomes the root of the next scale.
- **On (white)**: An extra note that is an octave multiple above the scale root is added to define the last note of the scale. The lowest root octave that is above the highest chord note is used. If the highest chord note is already an octave of the root, then it becomes the last note of the scale, and no note is added.

### TRIG input
If patched, then the outputs will not change until a monophonic trigger is received. The trigger is detected by a Schmitt trigger with a high threshold of 2V and low threshold of 0.1V.

If left unpatched, then the outputs are continuosly updated according to the current inputs.

### CHORD input
The polyphonic V/Oct input used to define the scale. If the OCTAVE FOLD is off, then only the first 14 channels are used. If the OCTAVE FOLD is on, then only the first 13 channels are used.

### TRIG output
The result of the TRIG input Schmitt trigger is forwarded to the TRIG output so that the trigger remains in sync with any scale changes.

### ROOT output
This monophonic output is typically patched to the NORSIQ SCALE ROOT input to define the root of the scale according to the lowest sorted note from the CHORD input. The ROOT output can be ignored if you want to transpose the scale to a different key.

### LENGTH output
This monophonic output must be patched to the NORSIQ SCALE LENGTH input to define the length of the scale.

### INTVLS output
This polyphonic output must be patched to the NORSIQ POLY INTERVALS input to define the interval between each of the notes in the scale.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V if NORSIQ Chord To Scale is bypassed.
