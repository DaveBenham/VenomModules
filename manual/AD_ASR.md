A hybrid polyphonic AD (Attack|Decay) and ASR (Attack|Sustain|Release) envelope generator with stage gates/triggers, looping capabilities, and precise V/Oct CV control over stage lengths covering an extremely wide range.

### Summary of Features
* Separate trigger and gate inputs allow one envelope generator to support both AD and ASR behaviors simultaneously
* Wide stage length range: 0.24 msec to 48.27 min
* Stage lengths are precise with accuracy dictated by VCV sample rate
* V/Oct CV control over stage lengths with attenuverters
* Independent stage shape controls for Rise and Fall: concave up to linear to concave down
* Changing a stage shape does not alter the overall time
* Multiple modes with different retrigger options: retrigger from 0 or current level
* Configurable Rise, Fall, and Sustain stage outputs indicate different events within the envelope
* Gate outputs can be used to convert triggers into precicely timed gates or delayed gates
* Feedback from stage gates can block retrigger behavior and/or force ASR attack to rise to full value
* Loop options turn the envelope into a V/Oct LFO with CV control to start and stop the oscillation
* All inputs and outputs are polyphonic with support for audio rates

### Envelope general behavior

#### AD (Attack|Decay) envelope
* The Attack (Rise) stage always rises to a full 10V, then immediately progresses to the Decay stage
* The Decay (Fall) stage falls back to 0V

There are also options as to whether a new AD envelope can be retriggered during the Rise and/or Fall stages.

#### ASR (Attack|Sustain|Release) envelope
* The Attack (Rise) stage rises toward 10V as long as the triggering gate remains high
  * If gate goes low, then immediately jumps to the Release stage at current voltage
  * If 10V is reached then progresses to Sustain stage
* The Sustain stage maintains 10V as long as the triggering gate remains high
  * Progresses to the Release stage when gate goes low
* The Release (Fall) stage falls back to 0V

There are also options as to whether a new ASR envelope can be retriggered during the Rise and/or Fall stages.

### RISE and FALL stage times
Rise and Fall stages each have a large knob to set the base time of the stage, as well as a CV input and attenuator to dynamically modulate the time.

The total effective time for a Rise from 0 to 10 or Fall from 10 to 0 can be as short as 0.24 msec or as long as 48.27 minutes.

The small top left **SPD** (Speed) button sets the range of the time knobs:
- **Slow** ***(red)***: 0.044 to 181 seconds
- **Medium** ***(yellow, default)***: 0.0028 to 11.3 seconds
- **Fast** ***(green)***: 0.00024 to 1.0 seconds
- **Glacial** ***(purple)***: 0.707 seconds to 48.27 minutes

The knob speed can be modulated by the associated CV input with attenuator. Each positive volt of CV doubles the length of the stage. Each negative volt cuts the length in half.

The CV can modulate the effective length beyond the limits of the current time knob configuration. However, the effective length for slow, medium, and fast speeds is always clamped to between ~0.24 msec and ~3 min. The effective length for glacial speed is clamped between 31.25 msec and 48.27 minutes.

Note that floating point computations have limited precision that could cause a long envelope to stall. If using the Slow configuration and the VCV sample rate is greater than 96 kHz, then the processing is automatically under-sampled to guarantee that the envelope never stalls. If using the Glacial configuration than all sample rates are under-sampled. But if using the Medium or Fast speeds then it is possible for CV modulated long envelopes to stall if the VCV sample rate is above 96 kHz. The envelope will never stall if stage length CV is not used.

### RISE and FALL shapes
The Rise and Fall stages each have a dedicated small knob to adjust the shape or curve of the stage.
- Counter-clockwise creates a concave up J curve
- Noon creates a linear rise or fall
- Clockwise creates a convex up J curve

Changing the shape does not change the overall time for a rise from 0 to 10V, or fall from 10 to 0V.

### Envelope Triggering Events

Envelopes are always triggered on the leading edge of a trigger or gate. TRIG triggers create AD envelopes, and GATE triggers create ASR envelopes.

TRIG and GATE each have a manual push button as well as a CV input. The state of manual push buttons and CV inputs are maintained independently.

By default, the manual TRIG and manual GATE buttons maintain a high state for as long as the button is pressed.

If the small top right **TOG** (Toggle) button is enabled (yellow), then the GATE button becomes a toggle switch. The first press of the GATE button switches the gate high, and the next press switches the gate low.

Triggers for TRIG and GATE CV are based on Schmitt triggers that go high above 2V and go low below 0.2V. Voltages between 0.2V and 2V maintain the current state.

The type of envelope generated (AD or ASR) depends on which trigger is received first, TRIG or GATE. TRIG (AD) triggers take precedence over GATE (ASR) triggers in the event of a tie.

All triggers are typically ignored if any of the other triggers or gates are already in a high state.

### Modes of operation

The small top middle MODE button can alter the behavior of TRIG and GATE, and controls when and how envelopes are retriggered. It can also cause envelopes to loop, effectively turning the module into an LFO.

There are four modes to choose from:

- **Mode 1** ***(light blue, default)*** AD and ASR envelopes that can retrigger from the current value while falling
- **Mode 2** ***(dark blue)*** AD and ASR envelopes that can retrigger from 0 while rising or falling
- **Mode 3** ***(yellow)*** Looping envelopes that are started and stopped via TRIG, and a high GATE causes the oscillator to sustain 10V
- **Mode 4** ***(green)*** TRIG initiated AD envelope if GATE is low, or looping AD envelope that is reset by TRIG if GATE is high

AD/ASR Rise stages that start above 0V due to a retrigger are shortened proportionally to where they start. Likewise, ASR Fall stages that start below 10V are also shortened proportionally.

Looping envelopes can behave like a V/Oct LFO if the V/Oct control voltage is patched to both the Rise and Fall CV inputs, and both attenuverters are set fully counter-clockwise to -100%. The looping frequency can go into audio rates as high as ~2 kHz, or slow LFO rates as low as ~0.0001726 Hz (96.54 minutes per cycle).

#### Light Blue Mode 1 (AD or ASR | Retrigger from current value)

- AD Envelope
  - Leading edge of a TRIG initiates an AD envelope.
  - The envelope first rises to 10V, then falls back to 0V
  - The AD envelope can actually sustain 10V if a high GATE is received after the AD has been triggered.
  - A new envelope may be retriggered from the current voltage during the Fall stage
- ASR Envelope
  - Leading edge of a GATE initiates an ASR envelope
  - The envelope rises toward 10V for as long as the gate remains high
    - If 10V is reached, then progresses to the Sustain stage
    - If the gate goes low before 10V, then immediately jumps to the Fall stage
  - The Sustain stage remains at 10V for as long as the gate remains high
    - Progresses to the Fall stage when the gate goes low
  - The Fall stage falls back to 0V
    - A new envelope may be retriggered from the current voltage during the Fall stage

#### Dark Blue Mode 2 (AD or ASR | Retrigger from 0)

- AD Envelope
  - Leading edge of a TRIG initiates an AD envelope.
  - The envelope first rises to 10V
    - A new envelope can be retriggered from 0 during the Rise stage
    - The AD envelope can actually sustain 10V if a high GATE is received after the AD has been triggered.
  - The Fall stage falls back to 0V
    - A new envelope may be retriggered from 0V during the Fall stage
- ASR Envelope
  - Leading edge of a GATE initiates an ASR envelope
  - The envelope rises toward 10V for as long as the gate remains high
    - If 10V is reached, then progresses to the Sustain stage
    - If the gate goes low before 10V, then immediately jumps to the Fall stage
  - The Sustain stage remains at 10V for as long as the gate remains high
    - Progresses to the Fall stage when the gate goes low
  - The Fall stage falls back to 0V
    - A new envelope may be retriggered from 0V during the Fall stage

#### Yellow Mode 3 (LFO)

A short trigger at either TRIG or GATE starts the looping envelope to oscillate. Both manual buttons and CV inputs work equally well.

If the TRIG trigger lasts longer than the combined Rise and Fall time, then only a single envelope is produced and the oscillator stops.

If the GATE trigger lasts longer than the Rise time, then the oscillator rises and stalls at 10V until the gate goes low, at which point oscillation begins.

A running oscillator can be temporarily stopped at 10V by a high GATE. Oscillation will resume when the GATE goes low.

A running oscillator can be fully stopped by a high TRIG gate that remains high when the oscillator falls to 0V. Once stopped, oscillations will not resume until a new trigger is received.

#### Green Mode 4 (AD with retrigger from 0 or LFO with hard sync)

Triggers are never blocked by a high gate at TRIG or GATE

If the GATE is low, then TRIG initiates a single shot AD envelope. The AD envelope can be retriggered from 0 during both the Rise and Fall stages.

If the GATE goes high, then the Rise stage immediately starts from 0, and the envelope oscillates for as long as the GATE remains high. The oscillator may be reset (hard synced) by a TRIG trigger at any time. Oscillations stop when the GATE goes low.

### Stage Outputs

Each of the stage output ports has a small button next to the label to configure what exactly is produced at that output.

#### RISE output
- **Gate** ***(dark blue, default)*** Produces a high gate (10V) whenever the envelope is rising toward 10V, else low (0V) otherwise.
- **Start trigger** ***(green)*** Produces a 1ms trigger upon entry to the Rise stage. The trigger may be shortened upon exit from the Rise stage.
- **End trigger** ***(red)*** Produces a 1ms trigger upon exit from the Rise stage. The trigger may be shortened upon rentry to the Rise stage.

Note that a Rise trigger is not fired if the envelope is retriggered during the Rise stage.

#### SUS (Sustain) output
- **Gate** ***(dark blue, default)*** Produces a high gate (10V) whenever the envelope is sustaining 10V, else low (0V) otherwise.
- **Start trigger** ***(green)*** Produces a 1ms trigger upon entry to the Sustain stage. The trigger may be shortened upon exit from the Sustain stage.
- **End trigger** ***(red)*** Produces a 1ms trigger upon exit from the Sustain stage. The trigger may be shortened upon rentry to the Sustain stage.

#### FALL output
- **Gate** ***(dark blue, default)*** Produces a high gate (10V) whenever the envelope is falling toward 0V, else low (0V) otherwise.
- **Start trigger** ***(green)*** Produces a 1ms trigger upon entry to the Fall stage. The trigger may be shortened upon exit from the Fall stage.
- **End trigger** ***(red)*** Produces a 1ms trigger upon exit from the Fall stage. The trigger may be shortened upon rentry to the Fall stage.
- **EOC (end of cycle) trigger** ***(orange)*** Produces a 1ms trigger when the Fall stage reaches 0V. The trigger is not fired if the envelope is retriggered before reaching 0V.

### ENV (Envelope) output

The envelopes are output here. The small button beside the label configures the voltage range of the envelope.

- **Unipolar** ***(green, default)*** Normal envelope from 0V to 10V
- **Inverted unipolar** ***(orange)*** Inverts the envelope from 10V to 0V
- **Bipolar** ***(red)*** Offsets the envelope by -5V for a +/-5V range

### Alternate AD and ASR behavior via feedback

Retrigger and ASR Rise behavior can be modified by patching one or more of the stage gate outputs into the TRIG and/or GATE inputs. These configurations take advantage of VCV's stackable input ports. Note that the patched stage output(s) must be configured to produce a gate for these configurations to work.

#### Non-looping Mode 1
- Feedback: None
  - AD Trigger
    - Rise To Full: Yes
    - Rise Retrigger: No
    - Sustain At Full: No*
    - Fall Retrigger: From current
  - ASR Gate
    - Rise To Full: While gate high
    - Sustain At Full: While gate high
    - Fall Retrigger: From current
- Feedback: Rise->Gate
  - AD Trigger
    - Rise To Full: Yes
    - Rise Retrigger: No
    - Sustain At Full: No*
    - Fall Retrigger: From current
  - ASR Gate
    - Rise To Full: Yes
    - Sustain At Full: While gate high
    - Fall Retrigger: From current
- Feedback: Rise->Trig, Fall->Trig
  - AD Trigger
    - Rise To Full: Yes
    - Rise Retrigger: No
    - Sustain At Full: No*
    - Fall Retrigger: No
  - ASR Gate
    - Rise To Full: While gate high
    - Sustain At Full: While gate high
    - Fall Retrigger: No
- Feedback: Rise->Gate, Sus->Trig, Fall->Trig
  - AD Trigger
    - Rise To Full: Yes
    - Rise Retrigger: No
    - Sustain At Full: No*
    - Fall Retrigger: No
  - ASR Gate
    - Rise To Full: Yes
    - Sustain At Full: While gate high
    - Fall Retrigger: No

\* An AD envelope will sustain 10V if a high GATE is received after the AD is triggered by TRIG  

#### Non-looping Mode 2
|Mode|Feedback|AD Trig<br />Rise<br />To Full|AD Trig<br />Rise<br />Retrigger|AD Trig<br />Sustain<br />At Full|AD Trig<br />Fall<br />Retrigger|ASR Gate<br />Rise<br />To Full|ASR Gate<br />Sustain<br />At Full|ASR Gate<br />Fall<br />Retrigger|
|:---:|:---|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
|2|None|Yes|From 0|No|From 0|While<br />gate<br />high|While<br />gate<br />high|From 0|
|2|RISE->TRIG|Yes|No|No|From 0|While<br />gate<br />high|While<br />gate<br />high|From 0|
|2|RISE->GATE|Yes|No|No|From 0|Yes|While<br />gate<br />high|From 0|
|2|RISE->TRIG<br />SUS->TRIG<br />FALL->TRIG|Yes|No|No|No|While<br />gate<br />high|While<br />gate<br />high|No|
|2|RISE->GATE<br />SUS->TRIG<br />FALL->TRIG|Yes|No|No|No|Yes|While<br />gate<br />high|No|

### Timed gate generator

The Rise gate output can be used to convert triggers into precicely timed gates.

Set the Mode to dark blue mode 2, set the Rise stage output to gate mode, and set the Rise time to the desired gate length. A trigger or gate at the Trig input will generate your gate at the Rise stage output. The Fall time is irrelevant because a trigger received during the fall stage will initiate a new envelope from 0, so the generated gate will always have the correct length.

If a second trigger is received during the rise stage, then the old gate is aborted and a new gate is begun immediately without the gate output going low.

You can prevent triggers from initiating overlapping gates by patching the Rise output to the Trig input.

### Delayed gate generator

The Fall gate output can be used to convert triggers into precicely timed delayed gates.

Set the Mode to dark blue mode 2, set the Fall stage output to gate mode, set the Rise time to your desired delay time, and the Fall time to your desired gate length. A trigger or gate at the Trig input will start the delay period, and then after the delay your desired gate will be sent through the Fall stage output.

If a subsequent trigger is received during the delay (rise) or gate (fall) period, then the current delayed gate will be aborted and a new delayed gate will be initiated.

You can prevent subsequent triggers from being received during the delay by setting the Rise stage to gate mode and patching the rise gate to the Trig input.

You can prevent all triggers from being received during the delay or gate by setting the Rise stage to gate mode and patching both the rise gate and fall gate to the Trig input. This guarantees that once a trigger is received, the full gate output will be initiated after the full delay period.

### Polyphony

All inputs and outputs are fully polyphonic with support for both low frequency and audio rates.

The total number of output channels is determined by the maximum number of channels received across all CV inputs.

Any monophonic CV input is replicated to match the output channel count.

Polyphonic CV input with fewer channels are assigned constant 0V for the missing channels.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are constant monophonic 0V when the AD/ASR Envelope Generator is bypassed.
