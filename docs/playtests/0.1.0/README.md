# Playtest: Atman 0.1.0 candidate

The owner played the MVP once it was complete and left four notes. Each was answered by the usability pass in
[#13](https://github.com/Preponderous-Software/atman/pull/13) before the release.

| | |
|---|---|
| Build | `main` after M7 (`2a445b8`; `b940190` after it changed only docs); no release existed yet, so this is filed under the next expected version |
| Date | 2026-10-23 |
| Player | The owner, by hand |

## Notes and what was changed

| Note | What was changed |
|---|---|
| "Didn't know which houses were which" | Each house is named over its roof when it is in sight within 30 m, and the well, the wheel, the clay bank and the ghat within 16 m. Your door is indigo, the potter's ochre, and the shrine's saffron, with a saffron pennant over the roofs. A new game says which house is yours. |
| "It was dark and hard to see and the lamp didn't help much" | The lamp is held at the shoulder, reaches 16 m instead of 10, and is about twice as bright. After sunset every house lights a lamp by its door, and a faint fill from the sky keeps the shapes of things. After dark, the prompt says "F  light your lamp" while it is out. |
| "Wasn't sure when the game was over" | A NEXT line on the HUD says what comes next, from the first pot to "Go home and sleep" and, in the after-mode, "Nothing more is asked of you". A new codex page is announced. The after-mode's first morning says "The teachings are done. Nothing more is asked of you." and a tanpura drone comes in. |
| "No sound" | Every sound is synthesized as the game plays: the river, the wind, birds and a koel, crickets, footsteps, the wheel, a heartbeat with fear, the drone, and a cue for each thing that happens. M (or the browser's Sound button) turns it off. |

The same pass also made the browser's words name its buttons on a touch screen, kept the names
of places from under the phone's HUD panel, and moved phone messages below the crosshair.

## Not yet checked by hand

- The sound on real hardware (the Windows desktop and a phone). It was checked offline: every
  sound was rendered and its levels measured, and test_audio checks that it is bounded and
  deterministic.
- The night's new lighting on a real GPU, rather than Mesa llvmpipe.
