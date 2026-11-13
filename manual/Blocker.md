Blocks expander chains for modules like [Venom Bypass](Bypass.md) and Stoermelder Strip.

Both Venom Bypass and Stoermelder Strip can operate on contiguous neighbor modules to the left and/or right, up until there is a gap. They locate neighbors via the VCV expander mechanism. Blocker works by hiding itself from the expander mechanism, so Bypass and Strip think there is a gap. Even when Blocker is bypassed, it still blocks expansion.

Blocker uses virtually no CPU, so it also works well as a 1hp blank.
