A highly configurable multi-stage polyphonic envelope generator supporting anywhere from 1 to 20 stages.

## *Basic Operation*
By default the Envelope Factory has four stages, but a module context menu "Stage count" option lets you select any count from 1 to 20. The module automatically expands or contracts to match the selected stage count. Modules to the right are automatically pushed right as needed when expanding. Cables attached to removed stages are automatically deleted.

Each stage can be configured independently to perform one of five actions:
- **Move** rises or falls from the current start voltage to a target voltage over a selected time interval.
- **Hold** holds a constant voltage for a selected time interval.
- **Sust** sustains a constant voltage for as long as the triggering gate remains high. Optionally a Sustain stage can drift toward the target of the next stage.
- **Rise** is the same as Move except the target is always 100% (normally 10V).
- **Fall** is the same as Move except the target is always 0% (0V).

Actions can be configured to have one of three modes:
- **Full** The stage always proceeds to completion before the envelope progresses to the next stage. Retrigger is not allowed.
- **RTrg** Same as Full except a retrigger can abort the stage prematurely to initiate the start of a new envelope.
- **Gate** The stage proceeds to completion as long as the triggering gate remains high. If the gate goes low then the envelope proceeds to the next non-Gate stage.

With these basic building blocks, a tremendous variety of envelope types may be constructed.

Envelopes start and end at 0%, with a typical range between 0 and 100%. If an envelope ends at a non-zero percentage, then the envelope output drops to 0 when the envelope becomes idle. The drop is instant if the Retrig From 0 option is enabled. Otherwise the drop is delayed by 2 samples so that looping envelopes don't have an unwanted downward spike to 0.

In addition to the standard envelope, Envelope Factory also produces an inverted form that typically starts at 100%, drops to 0%, then returns to 100%.

An amplitude control determines the voltage range between 0 and 100%, typically 10V. An offset control can shift the envelope higher or lower while preserving the overall shape, typically to make looping envelopes bipolar.

Without any CV modulation, stages can be as short as 1 msec, or as long as 2.78 hours. With CV modulation the stage lengths can be shortened or lengthened even more. Hold stages can actually be set to be nearly instantaneous (actually 1 sample), serving only to specify the target voltage of the previous Move stage.

Each stage has its own gate output that is high for as long as the stage is active. In addition, each stage can be selected to generate a trigger on a communal Stage Triggers output. There is also an Idle gate that is high whenever there is no active envelope. Additionally an end of cycle trigger can be generated at the Stage Triggers output.

#### Optional VCA

The Amplitude and Offset controls/inputs and Inverted output can be replaced by a VCA level control, audio input, Velocity input with velocity response control, and VCA output. When configured as a VCA, there is also an option to sync the envelope with the VCA input. In this case the triggers are delayed until the VCA input crosses 0. This is useful for preventing unwanted clicks when the envelope attack is extremely short.

#### Looping envelopes
Envelope Factory can function as an LFO by using the Idle gate output to trigger the envelope. When the envelope completes the Idle gate will go high which will retrigger the envelope.

There are two possible configurations for a looping envelope
- Endless loop: Patch the Idle output to the Gate input.
- Gated loop: Patch the Idle output to the Retrigger input. The envelope will loop as long as the Gate input is held high. This will only work as an LFO if the envelope does not have a Sustain stage.

There is a V/Oct input that applies to all timed stages equally, giving you V/Oct control over the LFO rate.

By using very short stage lengths, the looping envelope can be run at audio rates. However, the accuracy of stage timing is limited by the VCV sample rate, so the oscillator will not quite respond 1 V/Oct. Each stage timing can be off by as much as 1 sample, and this small error can become very significant at audio rates, especially as the pitch rises.

In addition, all CV inputs can be driven at audio rates.

Envelope Factory does not have any provision for oversampling, so audio rate outputs will be prone to digital aliasing.

#### Default configuration

The factory default for a newly placed Envelope Factory has four stages that are configured to function as either an ADSR or ADBDR envelope.

If the Sustain Drift is kept at 0, then the default configuration behaves like a typical Attack Decay Sustain Release (ADSR) envelope. The first Move stage is the Attack, the second Move stage the Decay, the third Sustain stage as itself, and the fourth Move stage is the final Release.

If the Drift is non-zero then the default configuration acts like an Attack Decay Break Decay2 Release (ADBDR) envelope. The Sustain stage defines both the Break point, as well as the Sustain's Decay2 rate. This is more like a physical piano where the note slowly decays while the sustain pedal is held down, and quickly decays once the sustain pedal is released.

#### Factory Presets
There are a number of factory presets that implement some common envelope types. There are two versions of each factory preset: Editable presets provide the stated functionality, but can be easily modified into something else entirely. Locked presets lock various parameter values so that the envelope cannot be transformed into a different type without first unlocking the parameters. The locked versions also rename all the stage knobs and ports to make it obvious what each stage does. Locked presets are a great way to gain familiarity with how the stage configuration works.

- Attack Decay Break Decay2 Release
- Attack Decay Sustain Release
- Attack Decay
- Attack Hold Decay Sustain Release
- Attack Sustain Release
- Decay
- Delay Attack Decay Sustain Release
- Timed step sequencer - 8 stages by default: A normal sequencer except each stage is timed rather than advanced by a clock
- Triggered step sequencer - 8 stages by default: The Gate button and input function as a Run control, and the Retrig button and input act as the sequencer clock

#### Polyphony
All input and output ports are polyphonic. The total number of output channels is set to the maximum channel count found across all inputs. Monophonic inputs are replicated to match the output channel count. Polyphonic inputs with fewer channels substitute 0V for any missing channels.

#### Randomization configuration
The standard module context menu Randomize action only applies to stage controls, excluding stage Action and Mode.

There is a Randomize configuration menu option that lets you specify which types of stage controls are randomized
- Stage times (Move, Rise, Fall, and Hold actions)
- Stage levels (Hold and Sustain actions)
- Stage shapes (Move, Rise, and Fall actions)
- Stage drifts (Sustain actions)
- Stage attenuverters (All knobs for all actions)
- Stage triggers (All actions)

#### Automatic termination of envelopes
All active envelopes will instantly be terminated and the envelope generator will return to an idle state if any of the following occur:
- Any stage is reconfigured to perform a different Action
- The number of stages is changed
- The Envelope Factory is bypassed

## *Global controls*

### Standard Mode
![Standard mode image](images/EnvelopeFactory_Std.png)

#### V/OCT (Volt per octave) input
CV at this input modulates all timed stages identically. Each positive volt doubles the rate (halves the time). Each negative volt halves the rate (doubles the time). This input is particularly useful when the envelope is configured to loop, thus creating an LFO with typical V/Oct control over the rate.

#### AMP (Amplitude) knob and CV input
Determines the magnitude of the final envelope. The input is attenuated and/or inverted by the knob to determine the effective 100% voltage. The knob attenuverter defaults to 100%, and the input is normalled to 10 volts, so if not patched, the envelope ranges between 0 and 10 volts (disregarding any offset).

#### OFF (Offset) knob and CV input
Determines any offset that is added to the final envelope after the amplitude is applied. The input is attenuated and/or inverted by the knob to determine the effective offset. The knob attenuverter defaults to 0%, so by default no offset is applied.

#### Slow (Knob time range) small button
Sets the range of all stage Time knobs
- **Fast** ***(Off, default)***: 0.001 sec - 10 sec
- **Slow** ***(Yellow)***: 0.01 sec - 100 sec
- **Crawl** ***(Orange)***: 0.1 sec - 1000 sec (16.667 min)
- **Glacial** ***(Red)***: 1 sec - 10000 sec (2.78 hr)

Stage times can be lengthened or shortened beyond the knob limits through CV modulation.

#### Gate Mode small button
Controls how the manual Gate button behaves
- **Momentary** ***(Off, default)***: The Gate button remains high for as long as the button is held
- **Toggle** ***(Yellow)***: Each press of the Gate button changes the state of the button

#### Retrig From 0 small button
Controls whether a retriggered envelope starts from 0
- **Off** ***(Off, default)***: A retriggered envelope starts from the current voltage rather than 0
- **On** ***(Yellow)***: Retriggered envelopes always start from 0

#### Retrig Mode small button
Controls how Retrigger CV input is interpreted
- **Schmitt trigger** ***(Off, default)***: Retriggers on the leading edge of a high gate. The input goes high when rising above 2V, and low when falling below 0.2V.
- **CV change start** ***(Yellow)***: Retriggers the instant a change is first detected
- **CV change end** ***(Blue)***: Retriggers when the input stops changing
- **CV change start or end** ***(Green)***: Retriggers when the input starts or stops changing

The CV change modes are generally used with quantized V/Oct input so a new envelope is retriggered every time the pitch changes. The pitch CV could have glide applied, in which case the different modes specify whether the retrigger is fired at the beginning or end of the glide, or both. If there is no glide then the three CV change modes give identical results. The glide start and end detection works best with linear glide. Non-linear glide may give unexpected results.

#### GATE button and CV input
An envelope is triggered when the Gate input rises above 2V. The gate returns to a low state when the input falls below 0.2V.

The button works by adding 10V to the CV input when the button is high.

#### RETRIGGER button and CV input
This is used to enable retriggering of an envelope while the main Gate input remains high. The label is a bit misleading in that this does not retrigger an envelope directly, but rather momentarily mutes the main Gate input for one sample. So the Retrigger can only have effect when the main Gate is high.

The button always retriggers the instant it is pressed.

The CV behavior depends on the setting of the Retrig Mode button.

When configuring an envelope with retrig, it can be useful to set the manual Gate button to Toggle mode so you can set the gate high, giving you a chance to press the Retrig button.

#### STAGE TRIGS (Stage triggers) output
This output can produce a 1 msec trigger at the start of each stage and/or at the envelope EOC (End Of Cycle). The small button above and to the left of each stage Gate output controls whether the stage generates a trigger or not. Similarly the global Idle output has a small button to determine whether the EOC trigger is generated. The stage triggers are especially useful if you want to use Envelope Factory as a timed sequencer.

By default the EOC trigger is enabled, and the stages triggers are disabled, making the Stage Trigs output a convenient EOC trigger. Note that EOC only triggers when an envelope proceeds to completion. Aborted envelopes do not generate an EOC trigger.

Note that consecutive triggers can merge into a single extended trigger when a stage is shorter than 1 msec. If the module "Fast gate triggers" context menu option is enabled, then stage triggers are only 1 sample long. This prevents triggers from merging, but not all modules recognize triggers that only last 1 sample.

#### IDLE (Idle gate), LED light, and EOC switch
The Idle output gate is always high at 10V when the generator is idle, and low at 0V when an envelope is in progress. This output can be patched to the Gate (or Retrig input with Gate toggled high) to create a looping envelope.

The LED light above the port glows yellow when the Idle gate is high. When working with polyphony the brightness of the LED is proportional to the percentage of channels that are idle.

Above and to the left of the Idle output is an EOC trigger switch. When enabled a 1 msec trigger will be generated at the Stage Trigs output whenever an envelope proceeds to completion. Aborted envelopes never generate an EOC trigger.

#### ENV (Envelope) output

The final envelope, typically with a resting voltage of 0V that typically ascends to 10V before returning to 0V.

The formula for the envelope in standard mode is ***(Envelope% x AmplitudeV) + OffsetV***

#### INV (Inverse envelope) output

An inverted form of the final envelope with a typical resting voltage of 10V that typically descends to 0V before returning to 10V. 

The formula for the inverse envelope is ***((100% - Envelope%) x AmplitudeV) + OffsetV***

### VCA Modes
![VCA mode image](images/EnvelopeFactory_VCA.png)

The module context menu has an option to enable VCA mode. The VCA mode can be standard where envelopes are triggered normally, or you can choose Synced mode where the envelope triggers are synced with the incoming audio. In sync mode the envelope trigger is delayed until the VCA input crosses zero. This is useful for preventing clicks that could otherwise appear when using extremely fast attacks.

When using a VCA mode, the Amplitude and Offset controls and inputs, as well as the Inverse output are replaced by the following:

#### VCA level knob and audio input
The knob specifies the attenuation level of the VCA with 100% being full volume.  
The input is the audio to be attenuated by the envelope.

#### VEL (Velocity) input and response control knob
The unipolar Velocity input is a second opportunity for attenuation that is multiplicative with the VCA Level. An input of 0V represents no output, and 10V representing 100%. The knob specifies the response curve for the velocity. The default noon value is a linear response. Counter-clockwise rotation creates a concave up response, similar to exponential. Clockwise rotation creates a concave down response, similar to logarithmic.

The Velocity level is sampled the instant the envelope is triggered and held constant throughout the duration of the envelope.

#### ENV (Envelope) output
The envelope is still output at the same port as standard mode. However, the formula for the envelope is changed.

***Final Envelope = Envelope% x Level% x Velocity%<sub>ResponseAdjusted</sub> x 10V***

#### VCA output
This is the audio input after it has been attenuated by the envelope and VCA.

## *Stage controls*
The module context menu has a Stages option where you specify the number of stages you want for your envelope.

The two square buttons at the top and the Gate output at the bottom have identical functionality for each stage.

#### ACTION square button
Controls what action the stage performs. The configuration and labeling of the other stage controls are automatically adjusted whenever the Action is changed.

- **Move** - The envelope rises or falls from the starting level to the target level over a fixed period of time. The starting level is the current level at the start of the stage. The target level is specified by the following Hold or Sustain stage Level. If not followed by a Hold or Sustain, then the target alternates between 100% and 0%, as indicated by the yellow LEDS above and below the Action button. An LED glowing yellow above the button indicates a 100% target, and an LED glowing yellow below indicates 0%.
- **Hold** - The envelope holds the specified level constant for a fixed amount of time.
- **Sustain** - The envelope holds the specified level constant for as long as the triggering Gate remains high. If a non-zero Drift is specified then the sustain will actually drift toward the target of the next ungated stage rather than maintaining a constant level.
- **Rise** - Same as Move, except the envelope always rises to a target of 100%, and the upper LED will glow yellow.
- **Fall** - Same as Move, except the envelope always falls to a target of 0%, and the lower LED will glow yellow.

All forms of Move normally start at the current level when the stage is initiated and then progress to the target value. However, if the first stage action is Fall then the envelope starts at 100% and falls to 0. This special case enables definition of a simple Decay envelope using a single stage.

#### MODE square button
Controls whether the stage length is impacted by the main Gate, and whether the envelope can be retriggered during the stage.

- **Full** - The stage always runs to completion and then the envelope advances to the next stage. Retrigger is not allowed.
- **RTrg (Retriggerable Full)** - The stage runs to completion and then the envelope advances to the next stage. However, the stage can be terminated prematurely by retriggering a new envelope. A retrigger happens when the Gate goes low before or during the stage, and then goes high during the RTrg stage.
- **Gate (Gated)** - The stage is gated, meaning the stage runs to completion unless the main Gate goes low. When the main Gate goes low the envelope immediately proceeds to the next stage that is not Gated, or else terminates if none exists.

#### GATE output, trigger button, and LED light
The stage gate output is high at 10V for as long as the stage is active, otherwise it is 0V.

The small unlabeled button above and to the left of the port controls whether the onset of the stage produces a 1 msec trigger at the Gate Trigs output.

The LED light above and to the right of the port glows yellow when the stage is active. When working with polyphony the brightness of the LED is proportional to the percentage of channels that are in that stage.

### *Variable knobs and CV inputs*
The functions of the remaining stage knobs and inputs change depending on the chosen stage Action.

### Move, Rise, and Fall action controls
![Move stage image](images/EnvelopeFactory_Move.png)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![Rise stage image](images/EnvelopeFactory_Rise.png)&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![Fall stage image](images/EnvelopeFactory_Fall.png)  
Rise and Fall are speciall cases of Move, so all three use the same controls.

#### Time knob, CV input and attenuverter
Specifies the time it takes to complete the Move action. Each positive volt of attenuverted CV doubles the time, and each negative volt halves the time.

Note that the time is for a normal full Move stage. Retriggered envelopes starting with Move can result in a shortened Move, and ungated Move after a Gate release can result in shortened or extended Moves. In such circumstances, the time will be lengthened or shortened proportionally depending on the actual Level at the time the Move starts.

If the starting voltage is already beyond the target voltage, then the envelope instantly jumps to the target voltage and advances to the next stage.

#### Shape knob, CV input and attenuverter
The knob specifies the shape of the stage, with the noon value of 0 representing linear. Counter-clockwise rotation specifies concave up curvature, with -1 being the most curvature. Clockwise rotation specifies concave down curvature, with 1 being the most curvature.

The shape CV is scaled at 0.2 per Volt such that a 10V peak to peak input can cover the entire shape range from -1 to 1.

Unlike some other envelope generators, changing the shape of a stage does not alter the length of the stage.

If the envelope is looped and used as an audio source, then modulation of the Move Shapes can add interesting evolving timbres without changing the fundamental pitch.

### Hold action controls
![Hold stage image](images/EnvelopeFactory_Hold.png)

#### Level knob, CV input and attenuverter
Specifies the Level of the Hold action, with 0% representing 0V, and 100% representing 10V.

The attenuverted CV is summed directly with the knob value, and the result is clamped to a value between 0 and 10 volts inclusive.

Typically a Hold stage is preceded by a Move stage, in which case the envelope will already be at the Hold Level at the start of the stage. If not preceded by a Move, then the envelope instantly jumps to the Hold Level at the start of the stage.

If the first envelope stage is a Hold at Level 0, and the envelope is retriggered, and Retrig From 0 is off, then Hold preserves the voltage at the time of retrigger instead of jumping to 0.

#### Time knob, CV input and attenuverter
Specifies how long the Hold level is held constant before progressing to the next stage.

The knob has an exponential scale, which normally would not allow for a length of 0. However, the minimum (fully counter-clockwise) value is interpreted as 0.

Each positive volt of attenuverted CV doubles the time, and each negative volt halves the time. Any effective length <= 0.001 sec is treated as 0.

Note that a length of 0 is not truly zero - instead the stage will last exactly one sample.

### Sustain action controls
![Sustain stage image](images/EnvelopeFactory_Sust.png)

#### Level knob, CV input and attenuverter
Specifies the Level of the Sustain action, with 0% representing 0V, and 100% representing 10V.

The attenuverted CV is summed directly with the knob value, and the result is clamped to a value between 0 and 10 volts inclusive.

Typically a Sustain stage is preceded by a Move stage, in which case the envelope will already be at the Sustain Level at the start of the stage. If not preceded by a Move, then the envelope instantly jumps to the Sustain Level at the start of the stage.

If the first envelope stage is a Sustain at Level 0, and the envelope is retriggered, and Retrig From 0 is off, then Sustain preserves the voltage at the time of retrigger instead of jumping to 0.

#### Drift knob, CV and attenuverter
Typically this knob is kept at zero (fully counter-clockwise) to preserve normal Sustain behavior.

If the value is non-zero then it specifies the rate at which the Sustain output will drift toward the target voltage of the next ungated stage. This feature was added to enable the stage to behave more like a sustain pedal on a piano, slowly fading until it reaches zero, or quickly dying if the pedal is released. However, the feature can be used more creatively, and can even drift to a higher level.

The Drift knob ranges from 0 to 1000 %/sec. The drift is always linear. If the target voltage is reached, then it will hold the target value throughout the rest of the Sustain stage.

The attenuverted CV is scaled at 10% per volt and summed with the knob value to establish the effective drift value. If the effective drift is 0 then the Sustain will hold the sustain level indefinitely.

### Standard Venom Context Menus
[Venom Themes](plugin.md#themes), [Custom Names](plugin.md#custom-names), and [Parameter Locks and Custom Defaults](plugin.md#parameter-locks-and-custom-defaults) are available via standard Venom context menus.

### Bypass

All outputs are monophonic 0V while Envelope Factory is bypassed.
