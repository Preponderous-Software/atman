# AI playtest of `b4bde32` and `ccf8e99` (five sessions, Strata driver)

A driven playtest of the 0.3.0 candidate after the fix for
[#37](https://github.com/Preponderous-Software/atman/issues/37). It replaces the run of `e1b80d3`
that first stood in this directory. Five sessions played the desktop build through
[playtester](https://github.com/Preponderous-Software/playtester)'s `atman-strata` profile, headless
on a stepped clock, from screenshots alone, at 640x360. Each session wrote a report, and
[summary.md](summary.md) merges them.

| | |
|---|---|
| Builds | s01 to s04: `b4bde32` (#39, the dream cue). s05: `ccf8e99` (#42, the sunset wait and wrapped HUD lines). `main` later moved on to `bdc802d` (Strata v0.7.0 and the first run's evidence); see [the 0.3.0 README](../README.md). Each was built with the profile's own command in a clean worktree |
| Date | 2026-10-29 |
| Harness | playtester `0561475`, `PT_CLOCK=stepped` |
| Window | 640x360 (the profile's) |
| Machine | 20 cores, shared with other work; a load average between 20 and 120 during the run |

## How this run differs from the last one

The previous run had four separate agents, each new to the game. This time no further agents could
be started (the session's limit on concurrent agents was reached), so all five sessions were driven
by the one agent that coordinated the run. That agent had read the change in #39 (and, for s05, #42)
before it played. The sessions therefore test whether the fixes show and work, and look for
regressions. They are not a fresh first reading of the game. The answers to the profile's two
questions come from that one player, so s01 to s03 give the same answers. s01 and s02, and s01 and
s03, were played turn about, so their wall-clock times overlap.

`main` moved to `ccf8e99` (#42) while s01 to s04 were being played. s05 played that commit through
the same day, to check #42's two changes and to check again that #37's fix still holds.

| Session | Build | Seed | Goal | Outcome |
|---|---|---|---|---|
| s01 | `b4bde32` | 1 | A new player's first day, through sleep to the morning, then V | achieved: three pots, a broken pot, three ropes, the dream and the deep sleep, woke on 22 March, V |
| s02 | `b4bde32` | 2 | The wheel, the clay lens, a broken pot, the codex, then dusk | achieved: read all three codex pages, turned one snake to rope |
| s03 | `b4bde32` | 3 | The rope at dusk, sleep, then V in the morning | achieved: three ropes, slept through to 22 March, V |
| s04 | `b4bde32` | 4 | The whole day through the dream and the deep sleep, then the codex, V and the after-mode chores | achieved: through the deep sleep to 22 March, the waking codex pages, eating, V |
| s05 | `ccf8e99` | 5 | The newest `main`: the sunset wait, the HUD's line wrapping, then the sleep and V | achieved: the whole day to 22 March, V |

The goals of s01 to s04 are word for word those of the previous run.

## #37 is fixed

All four sessions that slept (s01, s03, s04, s05) went through the dream and the deep sleep to
the morning. None took either for a hang, and none stopped inside the dream (the previous run's s03
did).

- In the dream, the NEXT block reads "Nothing here seems strange", and under it "Nothing to do; the
  dream passes by itself". The "You sleep, and dream." caption is shown too
  ([s04](s04/shots/dream1.png), [s01](s01/shots/dream1.png)).
- V in the dream says "Nothing is asked of you in sleep; it passes by itself."
  ([s04](s04/shots/dream_v.png), [s05](s05/shots/dream_v.png)).
- The deep sleep shows the breathing point and, under it, "Deep sleep. Morning comes by itself."
  ([s04](s04/shots/deep1.png), [s03](s03/shots/deep1.png)).
- The dream took about 77 seconds of game time and the deep sleep about 12, as before.

## #42 on `ccf8e99` (s05)

- On the ghat steps the line under NEXT reads "the sun going down [######....]"
  ([s05](s05/shots/sunset_bar.png)). At dusk a message says "The sun is down; a bell rings at the
  shrine." (#40)
- The NEXT line at the snakes wraps onto a second row, so "(1 of 3 seen)" can be read at 640x360
  ([s05](s05/shots/next_wrap.png)). (#41)

Two minor findings from s03, s04 and the previous run were fixed by these changes: the count cut off
at the right edge, and no sign of when the sunset wait ends.

## No blocker and no major finding

Nothing stopped play, and no session crashed. No new issue was filed.

## Minor findings (no issue filed)

Still present on `ccf8e99`:

- After waking, V shows a long message whose end runs off the bottom of the window at 640x360
  (s03, s04, s05). #41 wraps HUD lines; this message box is separate.
- Aiming at a pot on the drying board takes several tries, since the pots are small and below eye
  level (s02, s04).
- At 640x360 the HUD's text can cover a snake on the path, so hiding the HUD (H) was the quickest way
  to find one (s03, s04). A low wall by the river can stop a straight walk to the third snake without
  a sign (s03).
- "Go home" points through the house to the door on the far side (s05, and earlier runs).
- With the lamp lit, a house wall washes out the HUD's white text (s01, and earlier runs).
- A place label can be drawn over the CHORES header (s02). Walking into a wall with the view tilted
  up looks like being blocked by nothing (s01).
- Polish: the opening hint overlaps the help block, and the codex list clips long titles.

Fixed on `ccf8e99` by #42:

- The NEXT line at the snakes ran off the right edge and hid the count (s03, s04).
- Near the ghat the bearing flipped between "behind you" and "to your left", with no sign of
  arriving or of how long the wait lasts (s04).
