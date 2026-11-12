Adds CV inputs with attenuverters for all of the [Bernoulli Switch](BernoulliSwitch.md) parameters, all of which can be driven at audio rates.

The expander must be placed to the right of the Bernoulli Switch. The LED near the top glows yellow when a successful connection is made.

All the expander inputs are additive with their associated Bernoulli Switch parameters, with one exception. The mode input supersedes the mode parameter when patched. 
- Toggle mode <= -1V
- Swap mode >-1V and <1V
- Gate mode >= 1V

An attenuator for the Bernoulli Switch Probability CV was added rather than an attenuator for the expander mode CV.

All expander CV inputs are monophonic, with the signal applied equally to any polyphony found at the Bernoulli Switch.

None of the expander inputs are oversampled, even if the Bernoulli Switch has oversampling enabled.

All expander inputs as well as the probability CV attenuator are ignored when the expander is bypassed.

The following video demonstrates how the Bernoulli Switch coupled with the Bernoulli Switch Expander can transform a simple sine wave into a dynamic complex stereo sound.

https://www.youtube.com/watch?v=VRERXi6RuPE
