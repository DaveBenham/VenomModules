There are 7 expander modules that extend the functionality of the four Mix modules: [Mix 4](Mix4.md), [Mix 4 Stereo](Mix4Stereo.md), [VCA Mix 4](VCAMix4.md), and [VCA Mix 4 Stereo](VCAMix4Stereo.md).
- [Mix Offset Expander](MixOffset.md)
- [Mix Mute Expander](MixMute.md)
- [Mix Solo Expander](MixSolo.md)
- [Mix Fade Expander](MixFade.md)
- [Mix Fade 2 Expander](MixFade2.md)
- [Mix Pan Expander](MixPan.md)
- [Mix Aux Send Expander](MixSend.md)

Mix expanders must be placed to the right of the main mix module. Multiple expanders can be used for one mix module as long as they form a contiguous chain to the right. Each expander has an LED in the upper left that glows yellow if successfully connected to a mix module.

A main mix module may have up to 16 expanders. However, any combination of Mix Solo, Mix Mute, Mix Fade, and Mix Fade 2 counts as only one expander toward the 16 expander limit. Also any Mix Offset does not count toward the 16 expander limit.

Bypassing an expander disables that expander without disrupting expanders to the right.

All expanders except Offset are processed after the main Mix input channel Levels are applied, and before the main final Mix level is applied. When used with VCA Mix 4 or VCA Mix 4 Stereo, the expanders are processed after each channel has already been sent to the VCA channel output. Again, the Offset expander is an exception.

The order of operations generally proceeds from left to right. The order typically does not affect the end result unless Aux Send modules are used. However, some expanders have restrictions on where they are placed, and/or how many can be used for a single mix module. Refer to each expander for details.

CV inputs and outputs are generally all monophonic, with the effect applied equally to all polyphonic channels in the main mix module. The two exceptions are the Aux Send and Mix Pan expander modules, both of which fully support polyphonic cables. Extra poly channels beyond what is seen in the parent module are ignored.

Venom expanders are written so that communication between the parent module and the expander does not add any sample delays.

Some of the expanders have hidden options that are only available in the main mix module context menu. These expander related menu options only appear when the relevant expander is connected to the mix module.

#### Soft mute/solo: (Mute and/or Solo and/or Send expander)
 * If enabled (default), then mute/solo transitions are slewed to take 25 msec (or more if fades are applied).
 * If disabled, then mute/solo transitions are immediate (unless fades are applied)

#### Mute/solo CV toggles on/off: (Mute expander)
 * If disabled (default), then Mute CV and Solo CV function as a gate
 * If enabled, then Mute CV and Solo CV function as a toggle

#### Mono input pan law: (Pan expander)
Controls how left and right channels are attenuated/amplified as a mono input is panned. This affects the perceived loudness as a signal is panned left and right versus center.
 * **0 dB (linear: center overpowered)**  
   While panning left, the left gain is held constant while the right is attenuated. While panning right, the right gain is held constant while the left is attenuated. Mono signals sound softer when panned left or right compared to when panned center.
 * **+1.5 dB side (compromise: center overpowered)**  
   When panning left, the left channel is amplified slightly to +1.5 dB when panned full left. Likewise when panning right the right channel is amplified slightly. Mono signals still sound softer when panned left or right, but to a lesser degree.
 * **+3 dB side (default - equal power)**  
   While panning left, the left channel is amplified until it reaches +3 dB when panned full left. The right channel is similarly amplified when panning right. Mono signal loudness is perceived to be constant regardless whether panned left, center, or right.
 * **+4.5 dB side (compromise: side overpowered)**  
   When panning left, the left channel is amplified until it reaches +4.5 dB when panned hard left. The right channel is similarly amplified when panning right. Mono signals sound slightly louder when panned left or right compared to center.
 * **+6 dB side (linear: side overpowered)**  
   When panning left or right, one side is amplifed and the other attenuated in equal amounts, such that the net gain is always 1. Mono signals sound louder when panned left or right compared to center.
 * **-1.5 dB center (compromise: center overpowered)**  
   Same as +1.5 dB side except the center is attenuated rather than amplify the side.
 * **-3 dB center (equeal power)**  
   Same as +3 dB side except the center is attenuated rather than amplify the side.
 * **-4.5 dB center (compromise: side overpowered)**  
   Same as +4.5 dB side except the center is attenuated rather than amplify the side.
 * **-6 dB center (linear: side overpowered)**  
   Same as +6 dB side except the center is attenuated rather than amplify the side.

#### Stereo input pan law: (Pan expander)
Controls how left and right channels are attenuated/amplified as a stereo input is panned. All of the mono options are available, plus the following
 * **True panning (transfer content)**  
   Right channel input is mixed in with the left channel while panning left, and vice versa while panning right.
 * **Follow mono law (default)**  
   The mono pan law setting is used for stereo inputs as well
