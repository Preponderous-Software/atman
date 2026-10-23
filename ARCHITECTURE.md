# Architecture

Atman is a C++17 game on [Strata](https://github.com/Preponderous-Software/strata). Implemented
(milestones M1 and M2): the valley's ground, river and village layout with walking and collision,
the game clock, the body's needs and chapter 1, The Rope (`src/sim/`); the terrain grid, village,
snake and rope meshes (`src/bridge/geometry.*`) and their upload to Strata (`src/bridge/scene.*`);
the sun, the lamp, the camera and the HUD (`src/play/view.*`); and the desktop app with its headless
tour (`app/main.cpp`). Everything else here is still the plan. The layout follows
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
    end
    subgraph Engine["Strata (separate repo)"]
        Render["Renderer, window, input, picking"]
    end
    Content --> Sim
    Play --> Sim
    Sim --> Bridge --> Render
    Sim --> Save
```

**The engine boundary:** game rules never go into Strata. Strata is a shared engine and takes only
generic features a real consumer needs. Atman may propose engine additions (listed
[below](#engine-additions)), but "a snake becomes a rope after two seconds of lamplight" stays here.

## Source layout

| Path | Contents |
|---|---|
| `src/sim/` | Engine-free, deterministic rules: the valley, chores, the fear meter, snakes and ropes, pots and clay, chapter state, the clock, the autopilot |
| `src/bridge/` | The valley as a terrain grid and meshes (`geometry.*`) and their upload to Strata (`scene.*`), which swaps each coil's snake mesh for its rope mesh when the player sees it |
| `src/play/` | The sun, camera and HUD (`view.*`). Later a `Session` that drives the game for any front end: input, codex screen, autosave |
| `src/save/` | Save format: magic, version, SHA-256, previous save kept |
| `app/main.cpp` | Desktop entry point |
| `content/` | Codex entries and dialogue as data files |
| `tools/plates/` | Offline renderer for verse plates (see [Text](#text)) |
| `tests/` | Rules tests, a determinism fingerprint, a full autopilot playthrough |

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
  that keeps living (*prārabdha*, [C5]). The same code serves both.

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
| Clay lens | Textured meshes re-tinted to one clay texture | 1.7 |
| Space in the pots | Soft points (`STRATA_POINT_SOFT`) with alpha blending, unaffected when the pot mesh is removed | 1.5 |
| HUD and its retirement | Overlay text and lines with alpha, faded out by the play layer | 1.5 |
| Mouse look | `strata_set_relative_mouse_mode` | 1.8 |
| Witness camera (after the MVP) | Free camera with picking, as in steading's fly-cam | 1.8 |
| Dream drift (after the MVP) | `strata_set_mesh_transform` per frame | 1.7 |
| Browser (after the MVP) | WebGL 2 / wasm32 release asset | v0.4.0 |

Strata is fetched with CMake `FetchContent` at a pinned full commit SHA (`ATMAN_STRATA_REF`), and a
local checkout can be used with `-DFETCHCONTENT_SOURCE_DIR_STRATA=<path>`.

## Text

Strata's text overlay uses an 8×8 bitmap font that covers printable ASCII only. Devanagari needs
shaping (conjuncts, vowel signs), and even IAST letters such as ā and ṣ fall outside that range.

The MVP draws verses as **plates**:

1. `tools/plates/` renders each codex entry offline to an RGBA PNG, using a shaping text stack
   (Pillow with libraqm, or HarfBuzz) and an OFL-licensed Devanagari face such as Tiro Devanagari
   Sanskrit.
2. The PNGs are committed under `content/plates/` with the source text beside them, and CI checks
   that the plates are current.
3. At run time the bridge uploads each plate with `strata_create_texture_rgba8` and shows it on an
   unlit quad (`STRATA_MESH_UNLIT`).

The HUD stays in English ASCII. A UTF-8 glyph-atlas text path in Strata is a possible later engine
addition.

## Codex data

Each codex entry is a data file with: an id, the source text and verse (for example "Chāndogya
Upaniṣad 6.8.7"), Devanagari, IAST, an English translation, the translator and edition, and one or
more citation IDs from [RESEARCH.md](RESEARCH.md). CI fails if an entry cites an ID that
RESEARCH.md does not define, or if a translation has no named translator.

## Saves

The MVP uses Kindling's approach: a small binary format with a magic number, a version and a
SHA-256 over the body. It is written to a temporary file and renamed into place, and the previous
save is kept. A save that fails its checksum is refused with a message, and the previous one loads.

## Testing

| Test | What it proves |
|---|---|
| Rules tests | Each chapter's facts change only the way the rules say |
| Determinism | A fixed input sequence produces the pinned state fingerprint on every compiler |
| Playthrough | The autopilot finishes the MVP and lands on the pinned fingerprint |
| No-reward check | Seeing through a snake changes the coil and the codex record and nothing else: standing, quests and needs are untouched (`tests/test_rope.cpp`) |
| Codex | Every entry's citation IDs exist in RESEARCH.md, and its plate is up to date |
| Saves | Round trip; bit flips are rejected; the previous save survives |
| Headless tour | The desktop app renders fixed camera shots headless, as in Kindling's CI |

## Engine additions

These are proposed only if a milestone needs them, and each one goes to Strata as its own pull
request first:

- **UTF-8 text with a glyph atlas**, so IAST and Devanagari can be drawn live instead of as plates.
- **Transparent meshes**, if the salt and dissolve effects in chapter 5 need more than alpha-blended
  points and lines.
- **Audio**, for the river in deep sleep. Strata has no audio today; the game may handle sound
  outside the engine.
