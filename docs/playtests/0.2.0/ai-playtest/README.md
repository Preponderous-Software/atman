# AI playtest of 0.2.0 (four agents, Strata driver)

Four AI agents played the released 0.2.0 build through
[playtester](https://github.com/Preponderous-Software/playtester)'s `atman-strata` profile: headless,
driven through Strata's drive channel on a stepped clock (the world waits while an agent thinks),
from screenshots alone, at 640x360. Each wrote a report; [summary.md](summary.md) merges them.

| | |
|---|---|
| Build | `1100130` (the 0.2.0 release), built with the profile's own command from a fresh clone |
| Date | 2026-10-24 |
| Harness | playtester `0ec1a59`, `pt` 0.2.0, `PT_CLOCK=stepped` |
| Window | 640x360 (the profile's) |
| Machine | 20 cores, a load average near 8 at the start; the game ran at about 6–7 frames a second per session |

| Session | Seed | Budget | Goal | Outcome |
|---|---|---|---|---|
| s01 | 1 | 45 min | A new player's first day, through sleep, then V | partial: never met a snake |
| s02 | 2 | 40 min | The wheel, the clay lens, a broken pot, the codex, then dusk | achieved, and saw one snake turn to rope |
| s03 | 3 | 45 min | The rope at dusk, sleep, then V | partial: two of three snakes, no after-mode |
| s04 | 4 | 50 min | The day through to the dream and the deep sleep | partial: stuck on the dusk path |

No agent reached the dream. All four took the clay and the pots to be about one reality under many
names and forms, and the two who met a snake read it as fear put on a rope that never changed.
None found the game preachy, and all called the "Other schools" page fair.

## The major findings, and what was done

| Finding | Seen by | What was done |
|---|---|---|
| "Walk back to the village along the path" never completes; the snake and rope are never met | s01, s02, s04 | The cause was #25's bearing, which pointed at your door from dusk on. It now points at the nearest snake still on the path, and NEXT says what to do at one (fix/playtest-round-1) |
| The next morning NEXT says to wait for the sunset again | s01, s03 | Correct (a snake was left), but unexplained: it now says "A snake is still on the path; it comes back at dusk" |
| The HUD covers the pots, the wheel and the lens | s02 | H hides the HUD |
| At 640x360 the HUD, help, greeting and prompt overlap | s01, s03, s04 | Text is three quarters size under 600 pixels tall; the prompt and messages keep clear of the HUD |
| A black wall on the dusk path stopped walking | s04 | Not reproduced; the screenshot is consistent with walking into a house wall in the dark while following the old bearing, which no longer points there |
| V does nothing the morning after the rope, with no word why | s03 | V before the after-mode now says when it comes |

The minor findings that were acted on: B and C now say why they did nothing, Tab opens on the page
that has just opened, and the codex footers say where RESEARCH.md is. Not acted on: the "Other
schools" page's Advaita note names the rope and the snake before the dusk path (it is open from the
start); the slow sunset wait in a driven session (a matter of the machine's frame rate); and the
"awaiting review" credit, which stays until the review asked for in
[#28](https://github.com/Preponderous-Software/atman/issues/28).
