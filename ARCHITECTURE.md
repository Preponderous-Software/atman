# Architecture

Atman is a C++17 game on [Strata](https://github.com/Preponderous-Software/strata). Implemented
(milestones M1 to M7): the valley's ground, river and village layout with walking and collision,
the game clock, the body's needs, chapter 1, The Rope, chapter 2, Clay and Pots, the prologue's
chores, sleep and the after-mode, and the autopilot with its pinned playthrough (`src/sim/`);
the terrain grid and the village, snake, rope, pot and shard meshes, each paintable through the
clay lens (`src/bridge/geometry.*`), and their upload to Strata (`src/bridge/scene.*`); the sun,
the lamp, the motes, the camera and the HUD (`src/play/view.*`); the codex, its entries and plates
(`content/`, `tools/plates/`, `src/codex/`) and its screen (`src/play/codex_screen.*`); the
saves (`src/save/`); the sound (`src/audio/`); the desktop app with its headless tour (`app/main.cpp`), and the browser game (`app/web_main.cpp`,
`web/`; see [docs/WEB.md](docs/WEB.md)), which share one game session (`src/play/session.*`). Everything else here is still the plan. The layout follows
[Kindling](https://github.com/Preponderous-Software/kindling), the first game built this way, so
the parts that worked there are reused here.

## Layers

```mermaid
flowchart TB
    subgraph Content["Content (data, not code)"]
        Verses["Codex: verses, citations, glossary"]
        Valley["Valley layout, chores, dialogue"]
    end
    subgraph Game["Atman (this repo)"]
        Sim["Simulation core<br/>chapters, fear, clay, chores, clock"]
        Play["Play layer<br/>input, HUD, codex screen, session"]
        Bridge["Render bridge<br/>game state to Strata meshes, lights, overlays"]
        Save["Saves<br/>checksummed, previous kept"]
        Sound["Sound<br/>cues from the world, synthesized"]
    end
    subgraph Engine["Strata (separate repo)"]
        Render["Renderer, window, input, picking"]
    end
    Content --> Sim
    Play --> Sim
    Sim --> Bridge --> Render
    Sim --> Save
    Sim --> Sound
```

**The engine boundary:** game rules never go into Strata. Strata is a shared engine and takes only
generic features a real consumer needs. Atman may propose engine additions (listed
[below](#engine-additions)), but "a snake becomes a rope after two seconds of lamplight" stays here.

## Source layout

| Path | Contents |
|---|---|
| `src/sim/` | Engine-free, deterministic rules: the valley, the fear meter, snakes and ropes, pots and clay (`clay.*`), the chores, sleep and the after-mode (`chores.*`), the clock, and the autopilot with its route planner (`autopilot.*`) |
| `src/bridge/` | The valley as a terrain grid and meshes (`geometry.*`) and their upload to Strata (`scene.*`), which swaps each coil's snake mesh for its rope mesh when the player sees it, a pot for its shards when it breaks, and every clay colour when the lens turns on |
| `src/codex/` | The codex entries and their plates, built in at compile time by `cmake/embed_codex.cmake`; which entries are open; the PNG decoder (stb_image). No engine. |
| `src/play/` | The sun, camera and HUD (`view.*`), the codex screen (`codex_screen.*`), and the `Session` that drives a game for either front end: input, the codex, watching the avatar (`session.*`) |
| `src/save/` | Save format: magic, version, SHA-256, previous save kept |
| `src/audio/` | Sound: what the valley sounds like, worked out from the world (`cues.*`: the steady sounds of the place and hour, and a cue for each thing that happens); the synth that makes every sound from nothing (`synth.*`); and the sound card through miniaudio (`device.*`), silent when there is none. Only `device.*` touches a device. |
| `app/main.cpp` | Desktop entry point, the headless tour and the headless playthrough |
| `app/web_main.cpp`, `web/` | Browser entry point, the page shell with touch controls, and its smoke test |
| `content/` | Codex entries (`codex/*.json`) and their plates (`plates/*.png`); later dialogue |
| `tools/plates/` | Offline renderer and checker for verse plates, and the font (see [Text](#text)) |
| `tools/check_docs.py`, `tools/play-atman.bat` | The docs check CI runs, and the Windows play script that installs the tools, builds and starts the game |
| `third_party/` | stb_image and miniaudio, the game's own copies ([docs/THIRD-PARTY.md](docs/THIRD-PARTY.md)) |
| `tests/` | Rules tests, a determinism fingerprint, a full autopilot playthrough, saves, codex, geometry and sound |

## Simulation core

- **Fixed tick and no engine types.** The sim runs on a fixed tick and never includes a Strata
  header, so it builds and tests without a GPU.
- **Determinism.** As in Kindling, trig goes through the sim's own deterministic functions (no
  libm `sin`/`cos`), so the fingerprint in `tests/test_determinism.cpp` matches on GCC, Clang and
  MSVC. Any rule change updates the fingerprint in the same pull request.
- **Chapters as state, not scripts.** Each chapter is a set of facts the sim tracks (snakes seen
  through, pots named, pots broken, HUD elements retired). A chapter is complete when its facts
  hold. Nothing is scored.
- **The autopilot is a feature.** It plays the MVP in CI, and in the after-mode it is the avatar
  that keeps living (*prārabdha*, [C5]). The same code serves both. It chooses what to do from the
  World alone and walks there on a visibility graph around the village's obstacles, so the same
  World always gets the same input. Watching is not World state: the free camera lives in the play
  layer, and stepping out or back in changes nothing in the rules.

## Mapping the game to Strata

Everything here exists in Strata 0.4.0 (ABI 1.8) unless marked.

| Game need | Strata feature | Since |
|---|---|---|
| The valley | Lit terrain tile, `strata_set_terrain` | ABI 1.3 |
| Village, pots, the well | Meshes, textured meshes, cube shadow maps | 1.4 / 1.7 |
| Dusk and morning | `strata_solar_position` (NOAA) and `strata_set_sun_direction` | 1.6 |
| The lamp | Point lights (up to 32) | 1.1 |
| Looking closer at a snake | The sim's own line-of-sight test against each coil, so the rule stays deterministic and testable without a GPU (`strata_raycast_meshes`, ABI 1.8, is kept for picking that needs real triangles) | — |
| Snake becomes rope | The bridge removes the coil's snake mesh and adds its rope mesh, built on the same spiral | 1.4 |
| Pots on the wheel | The pot mesh scaled by `strata_set_mesh_transform` as it rises | 1.7 |
| Clay lens | The terrain, village and pots rebuilt with clay in one colour and the rest grey, and uploaded again | 1.3 / 1.4 |
| Space in the pots | Soft points (`STRATA_POINT_SOFT`) with alpha blending at fixed positions from the sim, untouched when a pot mesh is swapped for its shards | 1.5 |
| Codex pages | Each plate uploaded once with `strata_create_texture_rgba8` and drawn on an unlit quad (`STRATA_MESH_UNLIT`) in front of a camera of its own, far from the valley, cleared to the plate's dark colour; the menu is overlay text | 1.7 |
| HUD and its retirement | Overlay text, the bars included; a retired element's line is no longer drawn (`hudLines` in `src/play/view.cpp`) | 1.5 |
| Mouse look | `strata_set_relative_mouse_mode` | 1.8 |
| Watching the avatar (after-mode) | A free camera in the play layer, and the avatar mesh moved by `strata_set_mesh_transform` | 1.7 |
| Waking | A screen-sized black point in front of the eye, fading out | 1.5 |
| Witness camera (after the MVP) | Free camera with picking, as in steading's fly-cam | 1.8 |
| Dream drift (after the MVP) | `strata_set_mesh_transform` per frame | 1.7 |
| Browser (MVP milestone M6) | Strata's WebGL 2 backend under Emscripten, built from source at the pinned commit | v0.4.0 |

Strata is fetched with CMake `FetchContent` at a pinned full commit SHA (`ATMAN_STRATA_REF`), and a
local checkout can be used with `-DFETCHCONTENT_SOURCE_DIR_STRATA=<path>`.

## Text

Strata's text overlay uses an 8×8 bitmap font that covers printable ASCII only. Devanagari needs
shaping (conjuncts, vowel signs), and even IAST letters such as ā and ṣ fall outside that range.

The game draws verses as **plates**:

1. `tools/plates/make_plates.py` renders each codex entry offline to a 1600 × 900 PNG, using
   Pillow with libraqm for shaping and Tiro Devanagari Sanskrit (OFL), which also covers IAST. The
   left column is left dark for the game's menu.
2. The PNGs are committed under `content/plates/` beside their entries. Each records a SHA-256 of
   its entry and of the renderer in a PNG text chunk, so `make_plates.py --check` can tell a stale
   plate with plain Python, no Pillow or fonts. CI runs the check.
3. CMake builds the entries and plates into the game (`cmake/embed_codex.cmake`), so it needs no
   files beside it. At run time the codex screen decodes a plate with stb_image the first time it
   is shown and uploads it with `strata_create_texture_rgba8`.

The HUD stays in English ASCII. A UTF-8 glyph-atlas text path in Strata is a possible later engine
addition.

## Codex data

Each codex entry is a JSON file, `content/codex/<order>-<id>.json`, with: an id, its menu line
(ASCII), what opens it (a `CodexId` record, or `always`), a title, the source and verse (for
example "Chāndogya Upaniṣad 6.8.7"), Devanagari and IAST line by line, an English translation, the
translator, the Sanskrit edition, one or more citation IDs from [RESEARCH.md](RESEARCH.md), and
optional notes and readings (the other-schools page uses readings). CI fails if an entry cites an
ID that RESEARCH.md does not define, if a translation has no named translator, if a record opens
no entry or more than one, or if a plate is stale. Where no public-domain translation was checked,
the translator line says the translation is a working one made for Atman and awaiting review.

## Saves

Saves use Kindling's format (`src/save/`): the magic `ATMANSAV`, a version, the payload length, the
World field by field in little-endian, and a SHA-256 over the payload. The checksum is checked
before a byte of the payload is read, every read is bounds-checked, and a decoded World must be
plausible (finite numbers, a possible clock, pots that match the board, names in plain text)
before the rules run on it. A save is written to a temporary file and renamed into place, and the
save it replaces is kept as the previous save. A damaged current save is refused with a message
and the previous one loads; saving over a damaged save never replaces a good previous one.

The session (`src/play/session.*`) loads the slot on start, saves every two minutes, on F5 and on
quit, and loads on F9. What is only on screen is not saved: the codex page, the free camera and a
half-typed name. The desktop slot is `%APPDATA%\Atman\default` or `~/.local/share/atman/default`; the
browser's is `/atman-save/default` on IndexedDB (see [docs/WEB.md](docs/WEB.md)).

Two kinds of run never touch the slot: `--autopilot`, and a run a test harness drives through
Strata's drive channel (`STRATA_DRIVE`, Strata `docs/DRIVE.md`; playtester's `atman-strata`
profile). A driven run has no slot, so it neither loads nor saves, and it is silent, because its
clock may be stepped by the harness (`engine.frameSeconds()`) and synthesised sound would drift
against it.

## Testing

| Test | What it proves |
|---|---|
| Rules tests | Each chapter's facts change only the way the rules say (`tests/test_movement.cpp`, `test_clock.cpp`, `test_rope.cpp`, `test_clay.cpp`, `test_chores.cpp`) |
| Determinism | A fixed input sequence produces the pinned state fingerprint on every compiler |
| Playthrough | The autopilot plays the MVP from a new game (chores, pots, lens, a broken pot, the snakes, sleep) into the after-mode's first noon, with standing earned only by the first day's chores, and lands on the pinned fingerprint on every compiler (`tests/test_autopilot.cpp`; `atman --headless --autopilot` in CI) |
| No-reward check | Seeing through a snake, or breaking a pot, changes the thing itself and the codex record and nothing else: standing, quests and needs are untouched (`tests/test_rope.cpp`, `tests/test_clay.cpp`) |
| Geometry | The valley's meshes and terrain are valid for Strata and face outward; each form's pot matches the size the rules give it; the clay lens keeps the village's shape and only the clay's colour; each coil, as snake or rope, lies where the rules put it; and the terrain is the ground the rules walk on (`tests/test_geometry.cpp`) |
| Codex | Every entry's citation IDs exist in RESEARCH.md, and its plate is up to date (`make_plates.py --check`); each record opens exactly one entry, and every built-in plate decodes (`tests/test_codex.cpp`) |
| Saves | Round trips at several points of a playthrough (one with a pot waiting for its name); every single-byte flip and a truncation are refused; an impossible World is refused even with a good checksum; the previous save is kept and loads when the current one is damaged; a playthrough saved and loaded in the after-mode still ends on the pinned fingerprint (`tests/test_saves.cpp`); and in the browser a save survives a reload (the smoke test) |
| Audio | Each change in the world brings its cue and nothing else does; the steady sounds follow the place and the hour; every synthesized sound is finite, audible and unclipped, and a busy minute renders the same twice (`tests/test_audio.cpp`) |
| Headless tour | The desktop app renders fixed camera shots headless, as in Kindling's CI |

## Engine additions

These are proposed only if a milestone needs them, and each one goes to Strata as its own pull
request first:

- **UTF-8 text with a glyph atlas**, so IAST and Devanagari can be drawn live instead of as plates.
- **Transparent meshes**, if the salt and dissolve effects in chapter 5 need more than alpha-blended
  points and lines.
- **Audio**, for the river in deep sleep. Strata has no audio today, so the game plays its own sound
  outside the engine, through miniaudio (`src/audio/`); an engine API would replace only `device.*`.
