# AI playtest of `e1b80d3` (four agents, Strata driver)

A driven AI playtest of `main` at `e1b80d3` (#35), the 0.3.0 candidate. Four agents played it
through [playtester](https://github.com/Preponderous-Software/playtester)'s `atman-strata` profile,
headless on a stepped clock, from screenshots alone, at 640x360. Each wrote a report, and
[summary.md](summary.md) merges them.

| | |
|---|---|
| Build | `e1b80d3`, built with the profile's own command in a fresh worktree |
| Date | 2026-10-29 |
| Harness | playtester `0561475`, `pt` 0.2.0, `PT_CLOCK=stepped` |
| Window | 640x360 (the profile's) |
| Machine | 20 cores, shared with other playtests; a load average near 40 at the start, and about 5 to 8 frames a second per session |

| Session | Seed | Budget | Goal | Outcome |
|---|---|---|---|---|
| s01 | 1 | 35 min | A new player's first day, through sleep to the morning, then V | achieved: three pots, three ropes, the dream, woke on 22 March, V |
| s02 | 2 | 35 min | The wheel, the clay lens, a broken pot, the codex, then dusk | achieved, and turned one snake to rope |
| s03 | 3 | 35 min | The rope at dusk, sleep, then V in the morning | partial: all three ropes, slept, stopped inside the dream |
| s04 | 4 | 40 min | The whole day through the dream and the deep sleep, then the codex, V and the after-mode chores | achieved: through the deep sleep to 22 March, the waking codex pages, V |

The first three goals are the profile's own, with shorter budgets so that the run finished before
10:00. The fourth repeats the dream goal of the 0.2.1 playtest. s01 and s04 went over their budgets
by a few minutes, while the dream played out.

## The major finding

| Finding | Seen by | Issue |
|---|---|---|
| The dream and the deep sleep give no sign that they end by themselves. The clock runs backwards for over a minute under "Nothing here seems strange", and the deep sleep is a black screen, so both were taken for a hang | s01, s03, s04 | [#37](https://github.com/Preponderous-Software/atman/issues/37) |

The "You sleep, and dream." caption from #32 was seen and understood. It names the dream, but it
does not say that the dream passes on its own. s04 rated the deep sleep a second major finding. It
is folded into #37, since its cause is the same.

## Minor findings (no issue filed)

- At 640x360 the NEXT line at the snakes runs off the right edge and hides the "(1 of 3)" count
  (s01, s03, s04). The opening hint still overlaps the help block (all four).
- The ghat's sunset wait has no cue for how long it lasts (s01, s02, s03).
- Thrown pots appear on the drying board behind the wheel, with no word of where they went (s01,
  s02). The wheel looks like a flat dark disc (s02).
- After waking, the after-mode chores have no bearing, and E at the well did nothing for s04.
- Lamp-lit walls wash out the HUD text, and "Go home" points through the house to the door on the
  far side (s01, s03).
- Polish: the codex list clips long titles, the codex text is small at 640x360, and V's first view
  can start inside a house wall.

Most of the minor findings were reported in the 0.2.0 and 0.2.1 playtests too. None is new with
#35, which changed only the browser build.
