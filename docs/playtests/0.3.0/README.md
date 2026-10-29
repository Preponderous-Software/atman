# Playtest: Atman 0.3.0 candidate

No person has played this build by hand. Its evidence is a driven AI playtest of four agents, the
web smoke test against the live arcade build, and the automated checks on the candidate.

| | |
|---|---|
| Build | `main` at `e1b80d3` (#35), which still says 0.2.1; filed under the next expected version, 0.3.0 |
| Date | 2026-10-29 |
| Players | AI agents through playtester's `atman-strata` profile, the autopilot and the smoke test; not a person |

Since 0.2.1 the game changed in one place: #35 loads the browser game behind Strata's loading shell.
The desktop game's code is the same as 0.2.1's, so the desktop playtest is a check for regressions,
and the smoke test covers the change itself. `02386ca` (#36), on `main` after it, changes only
README.md and docs/WEB.md.

## The AI playtest

[Four agents](ai-playtest/README.md) played the desktop build headless. Three finished their goals
and one stopped partway through. All four turned the snakes on the dusk path to rope. Three slept
through the dream and the deep sleep into the after-mode. No blocker was found, and no crash.

One finding was major and confirmed:
[#37](https://github.com/Preponderous-Software/atman/issues/37). The three agents who slept took
the dream, and one of them the deep sleep, for a hang. Nothing says that either ends by itself.
One agent stopped inside the dream.

Asked what the game was about, all four said in their own words that the named pots are one clay
and the snake was only ever the rope: many names and forms over one reality, and fear that comes
from not seeing clearly. None found it preachy, and all called the "Other schools" page fair.

## The browser build (the change in this release)

`web/smoke/smoke.cjs` from `e1b80d3` was run against the live arcade
(`https://atman.play.danielstephenson.dev/`, deployed from `02386ca`, whose game files are the same
as `e1b80d3`'s). The desktop and iPhone 13 profiles both passed. In both, the loading shell showed
and then cleared. Without WebGL 2 the shell shows the trailer and says why, and none of the engine
downloads. Walking, looking, the wheel (desktop), saving and reloading, and the second village all
worked.

| Desktop, after loading | Phone, without WebGL 2 |
|---|---|
| ![The game in the browser after the loading shell](web-desktop.jpg) | ![The shell's fallback without WebGL 2](web-phone-no-webgl2.jpg) |

## Automated checks on the candidate

| Check | Result |
|---|---|
| `atman --headless --autopilot` (local, llvmpipe) | ends at 12:00 pm on day 1 on the pinned fingerprint `b0790a95717290be` |
| `atman --headless --tour` at 1280x720 and 640x360 (local) | all 28 frames render at both sizes |
| playtester `pt smoke --profile atman-strata` | PASS |
| CI on `e1b80d3`: Docs, Rules and tests (Linux), Game (Linux, headless tour), Game (Windows, MSVC), Web build and smoke test (headless Chromium) | all passed |

## Not yet checked by hand

- The loading shell on a real phone and a real desktop GPU.
- Everything on the earlier lists: the dream and deep sleep as a person sees them, the sound by ear,
  the night's lighting and the lamp night on a real GPU, F2, F3, F6 and H in the desktop game.
- The translations: [#28](https://github.com/Preponderous-Software/atman/issues/28) asks for a review.
