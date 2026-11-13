Recurse Stereo is identical to [Recurse](Recurse.md) except the Input/Return inputs and Output/Send outputs are doubled to support left and right channels of a stereo pair.

The number of input polyphonic channels is strictly controlled by the Left Input. Any extra channels in the Right Input are ignored.

The Right Input is normalled to the Left Input.

In addition, The Left Return is normalled to the Left Send, and the Right Return is normalled to the Right Send.

The Recursion Count, Scale, Offset, and Modulation Timing settings are applied to both Left and Right identically.

Both left and right inputs are passed unchanged to the outputs when RECURSE STEREO is bypassed. The right input remains normalled to the left input while bypassed. Bypassed left and right send are monophonic 0V.
