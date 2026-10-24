# MVP

The MVP is the smallest slice that feels like Atman: **the prologue and the first two teachings in
the waking valley, ending in a short after-mode where the game keeps running and asks nothing of
you.** If seeing the rope as a rope and the pots as clay feels like understanding something,
rather than winning something, the rest of the game is more of it.

Bracketed IDs refer to sources in [RESEARCH.md](RESEARCH.md). Chapters are described in
[CONCEPT.md](CONCEPT.md).

## In scope

| Area | The MVP has |
|---|---|
| Place | One valley: a few village houses, the potter's yard, the path to the ghat, the bird tree |
| Time | One day, from late afternoon into dusk and night, then the following morning |
| Player | First-person movement, a carried lamp, look and use |
| Prologue | Chores (carry water, fetch clay for the potter), a full HUD with needs, standing and a quest log |
| Chapter 1, The Rope | Snakes at dusk, a fear meter, looking closer with the lamp, the fear meter removed for good [C1] |
| Chapter 2, Clay and Pots | The wheel, naming pots, the clay lens, breaking a pot with the space unchanged [P1] [C2] |
| After (short) | The next morning: the autopilot carries on the chores, the player can watch or step back in, the quest log has no timers, nothing ends |
| Codex | Every verse used so far, with Devanagari, IAST, translation and citation, plus the "other schools" page [S3] |
| Saves | Save and continue at any point; a damaged save is refused and the previous one is kept |
| Platform | Windows and Linux desktop, and the browser on the arcade (danielstephenson.dev/play) |

## Out of scope

Each of these is deferred, not rejected:

- Chapters 3 and 5 (sheaths, salt). Three States was added after the MVP: the dream and the deep
  sleep, with the fourth's codex page; the witness view is the after-mode's watching (V).
- The full after-mode with the HUD dissolved.
- Narration, music and Sanskrit pronunciation audio.
- Scholar and teacher sign-off. This is required before a build is published, not before the MVP is
  playable.

## What "done" means

Each check is something a playtester can do in the build without reading code.

| Step | Done when |
|---|---|
| Prologue | A new player can carry water to the potter's house and fetch clay without instructions beyond the quest log, and the HUD shows needs, standing and quests. |
| Dusk | The sun sets on the real solar path for the valley's date and latitude, and the path to the ghat is dark enough that the lamp matters. |
| The Rope | A snake on the path raises the fear meter as the player gets closer. Holding the lamp near it and looking at it for about two seconds turns it into a rope. The rope stays where it was. After the last snake, the fear meter leaves the HUD and does not come back. |
| No reward | No points, sound cue or achievement fire when a snake becomes a rope. The only change is the rope and a codex entry. |
| Clay | The player throws at least three pots and names them. The clay lens shows pots, bowls and bricks as one material. |
| Pot-space | Breaking a pot leaves the floating motes of light inside it exactly where they were. |
| Codex | Each verse shown has Devanagari, IAST, an English translation, the translator, and a citation ID that exists in RESEARCH.md. |
| After | The next morning the avatar walks its chores alone. The player can detach, watch, step back in, or quit. Nothing on screen says the game is over or won. |
| Saves | Quitting and reloading returns to the same place and time. A save with one flipped byte is refused with a message, and the previous save loads. |
| Autopilot | `--headless --autopilot` plays the whole MVP and matches the pinned fingerprint. |
| Browser | The arcade serves the game. A player can walk, light the lamp, throw and name pots, and read the codex in a desktop browser, and walk and look on a phone. |

## Milestones

Each milestone is one pull request, merged when CI is green.

| Milestone | Scope |
|---|---|
| M0 | Docs (this set) and CI that checks the docs' links and citation IDs |
| M1 Valley (done) | Terrain, village meshes, walking, sun from the solar position, the HUD |
| M2 Rope (done) | Dusk, the lamp, looking closer, snakes and ropes, the fear meter |
| M3 Clay (done) | Wheel, naming, clay lens, pot-space |
| M4 Codex (done) | Verse plates, the codex screen, the other-schools page |
| M5 After (done) | Autopilot, the after-mode, the pinned playthrough |
| M6 Browser (done) | A WebGL 2 build on Strata's wasm32 target, deployed to the arcade from CI, with a smoke test in desktop and phone browsers |
| M7 Saves and Windows (done) | Checksummed saves (on the desktop, and in browser storage on the arcade), Windows CI and a play script |
