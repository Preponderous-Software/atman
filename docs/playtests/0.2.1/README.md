# Playtest: Atman 0.2.1 candidate

No person has played this build by hand. Its evidence is two driven AI playtests and the automated
checks on the candidate.

| | |
|---|---|
| Build | `main` at `d8f5cc7` (#31), with the version set to 0.2.1 |
| Date | 2026-10-24 |
| Players | AI agents through playtester's `atman-strata` profile, the autopilot and the smoke test; not a person |

## The AI playtests

- [The first](../0.2.0/ai-playtest/README.md), four agents on the 0.2.0 release, found that the
  bearing under NEXT led away from the dusk path: three of four never finished it, and two never met
  a snake. That and the rest of what they confirmed were fixed in
  [#30](https://github.com/Preponderous-Software/atman/pull/30).
- [The second](ai-playtest/README.md), two agents on `dae6fe4` (#30), both met all three snakes, and
  one played through the dream and the deep sleep into the after-mode. Both took the dream for a
  fault; it is now named as it begins ("You sleep, and dream.") and seen through a faint violet haze
  ([#32](https://github.com/Preponderous-Software/atman/pull/32)).

Asked what the game was about, all six agents answered, in their own words, that the many named
things are one reality under names and forms; none found it preachy, and all called the "Other
schools" page fair.

## Automated checks on the candidate

| Check | Result |
|---|---|
| `atman --headless --autopilot` | ends at 12:00 pm on day 1 on the pinned fingerprint `b0790a95717290be` |
| `atman --headless --tour` at 1280x720 and 640x360 | all 28 frames render; at 640x360 no prompt or message crosses the HUD |
| `web/smoke/smoke.cjs`, desktop and iPhone 13 profiles | both pass (on #32's tree) |
| `ctest` (12 suites, natively and as wasm) | all pass, including saves written by 0.1.2 and by 0.2.0 loading |

## Not yet checked by hand

- The dream with its new cue: whether it now reads as a dream.
- Everything on the earlier lists: the sound by ear, the night's lighting and the lamp night on a
  real GPU, F2, F3, F6 and H in the desktop game, the phone layouts.
- The translations: [#28](https://github.com/Preponderous-Software/atman/issues/28) asks for a review.
