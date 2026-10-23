# Atman

A small 3D game about Advaita Vedānta: the teaching that the self (Ātman) is the absolute
(Brahman), and that liberation (mokṣa) is a recognition, not a reward. Atman runs on
[Strata](https://github.com/Preponderous-Software/strata), a small C++ engine.

You wake in a river village with a body, a name and a list of chores. At dusk a snake on the path
turns out to be a rope. The potter's pots turn out to be clay. Chapter by chapter, the things you
took to be "you" are examined the way the Upaniṣads examine them, and the game's own interface
comes apart. At the end nothing rolls credits. You are still in the village, still doing the
chores, and nothing is at stake.

**Status: milestone M2 of the [MVP](MVP.md).** You can walk the valley in first person: the
village, the well, the potter's yard, the ghat and the bird tree, from late afternoon through
sunset into night, with the sun where it really is over the middle Ganges plain on the spring
equinox. The HUD shows the time, the body's needs, your standing and the quest log.

At dusk the first teaching begins, The Rope. Three snakes lie on the path between the village and
the ghat. A fear meter rises as you come near one, and you cannot bring yourself to walk up to it.
Light the lamp (F), go as close as you dare and look at it steadily for about two seconds: it is a
rope, and it stays a rope. Nothing is scored. After the last one the fear meter leaves the HUD
and does not come back. The chores in the quest log cannot be done yet.

| The well, at a quarter to five | The ghat steps down to the river |
|---|---|
| ![First person by the village well: lime-washed houses with tiled roofs, the potter's drying pots, the HUD with needs and quests](docs/images/m1-well.jpg) | ![Stone ghat steps leading down to the river at late afternoon](docs/images/m1-ghat.jpg) |

| The village from the ghat | Sunset over the river, just after six |
|---|---|
| ![Looking west from the ghat over the village toward the low sun](docs/images/m1-village-from-ghat.jpg) | ![The river at sunset, the sky orange over dark hills](docs/images/m1-sunset.jpg) |

| Lamplight on a snake at half past seven | It was a rope |
|---|---|
| ![At night by lamplight, a dark snake coiled on the path with its head raised; the HUD shows the fear meter high](docs/images/m2-lamp-on-the-snake.jpg) | ![The same spot: a straw-coloured rope lies coiled on the path where the snake was](docs/images/m2-rope.jpg) |

*Frames from the game's own headless tour (`atman --headless --tour --frames DIR`).*

## Build and play

Atman builds with CMake 3.24 or newer and a C++17 compiler. Strata, and the SDL3 it uses, are
fetched and built along with it, at the commit `ATMAN_STRATA_REF` in `CMakeLists.txt` pins
(Strata v0.4.0).

```sh
cmake -S . -B build
cmake --build build --config Release
./build/atman               # Windows: build\Release\atman.exe
ctest --test-dir build -C Release
```

On Linux, SDL3 needs the X11 development packages (`libx11-dev libxext-dev libxcursor-dev libxi-dev
libxfixes-dev libxrandr-dev libxss-dev`). `cmake -DATMAN_WITH_STRATA=OFF` builds only the
simulation and its tests, with no engine.

| Input | Action |
|---|---|
| Mouse | Look |
| W A S D | Walk |
| Shift | Hurry |
| F | Light the lamp, or put it out |
| F1 | Show or hide the controls |
| Esc | Free the mouse (Esc again quits) |

## Documents

| Document | What it covers |
|---|---|
| [CONCEPT.md](CONCEPT.md) | The game: the design problem, the valley, each chapter's teaching and mechanic, teaching rules, tone |
| [MVP.md](MVP.md) | The first playable slice, what "done" means for it, and the milestones |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Code layout, the simulation core, how each mechanic maps to Strata, text, saves, tests |
| [RESEARCH.md](RESEARCH.md) | Sources: Upaniṣads, Śaṅkara and later Advaita, modern scholarship, Alan Watts and other modern teachers, related games, and a verification log |

## Approach to the teaching

- The doctrine follows classical Advaita as taught by Śaṅkara and the Upaniṣads he commented on.
- Modern voices such as Alan Watts, Swami Vivekananda and Ramana Maharshi shape the tone, and the
  game labels them as later readings.
- Other Vedānta schools (Viśiṣṭādvaita, Dvaita) read the same verses differently, and the game says
  so.
- Every verse shown in the game is cited with its source, verse number and translator.
- The game never awards a score or achievement for liberation.
- An academic and a teacher from a living Advaita lineage review the game before a build is
  published.

Corrections from people who know this tradition are welcome as issues.

## Checking the docs

`python3 tools/check_docs.py` checks that every relative link and heading anchor resolves, and that
every citation ID (such as [P1]) is defined in RESEARCH.md. CI runs it on every pull request.

## License

[MIT](LICENSE).
