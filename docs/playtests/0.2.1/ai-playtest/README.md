# AI playtest of `dae6fe4` (two agents, Strata driver)

A second driven AI playtest, of `main` after the fixes from the [first one](../../0.2.0/ai-playtest/README.md)
([#30](https://github.com/Preponderous-Software/atman/pull/30)). No release had this build, so it is
filed under the next expected version, 0.2.1. Two agents played it through
[playtester](https://github.com/Preponderous-Software/playtester)'s `atman-strata` profile, headless on a
stepped clock, from screenshots alone, at 640x360. [summary.md](summary.md) merges their reports.

| | |
|---|---|
| Build | `dae6fe4` (#30), built with the profile's own command from a fresh clone |
| Date | 2026-10-24 |
| Harness | playtester `0ec1a59`, `pt` 0.2.0, `PT_CLOCK=stepped` |
| Window | 640x360 |

| Session | Seed | Budget | Goal | Outcome |
|---|---|---|---|---|
| s01 | 3 | 45 min | The rope at dusk, sleep, then V | partial: all three ropes, slept into the dream, stopped there |
| s02 | 4 | 60 min | The whole day through the dream and the deep sleep | achieved: three ropes, the dream, the after-mode's morning, both new codex pages |

The #30 fixes worked: both agents found the path, met all three snakes and saw them turn to ropes.
For the first time an agent reached the dream and woke into the after-mode.

## The major finding, and what was done

| Finding | Seen by | What was done |
|---|---|---|
| The dream has nothing to say it is a dream; the backwards clock reads as a hang or a clock bug (s01 stopped inside it) | s01, s02 | The dream now begins with "You sleep, and dream." and is seen through a faint violet haze (fix/playtest-round-2) |

Also acted on: NEXT keeps the snake count ("(1 of 3 seen)") and says to go closer with the lamp when
out of its reach; the bearing stays until a metre and a half from the place, so it no longer goes
before your door is in reach; V steps out to just behind the body, so the view changes. Not acted
on: the slow sunset wait in a driven session (the machine's frame rate), the pots appearing behind
the wheel, the codex list cutting long titles at 640x360, and the "Other schools" page naming the
rope and the snake before dusk.

s02, asked whether anything felt preachy or inaccurate: "Not preachy. The teaching stays in the
optional codex, with verse, transliteration, a cited translation and a careful gloss, and it
presents the other schools fairly."
