# Atman

A small 3D game about Advaita Vedānta: the teaching that the self (Ātman) is the absolute
(Brahman), and that liberation (mokṣa) is a recognition, not a reward. Atman runs on
[Strata](https://github.com/Preponderous-Software/strata), a small C++ engine.

You wake in a river village with a body, a name and a list of chores. At dusk a snake on the path
turns out to be a rope. The potter's pots turn out to be clay. Chapter by chapter, the things you
took to be "you" are examined the way the Upaniṣads examine them, and the game's own interface
comes apart. At the end nothing rolls credits. You are still in the village, still doing the
chores, and nothing is at stake.

**Status: the [MVP](MVP.md) is complete (milestones M0 to M7), and chapter 3, Three States, has
been added since.** Play it in a browser at
**[atman.play.danielstephenson.dev](https://atman.play.danielstephenson.dev)**, on a computer or a
phone ([docs/WEB.md](docs/WEB.md)), or download it for Windows or Linux from the
[releases page](https://github.com/Preponderous-Software/atman/releases). You can walk the valley in first person: the
village, the well, the potter's yard, the ghat and the bird tree, from late afternoon through
sunset into night, with the sun where it really is over the middle Ganges plain on the spring
equinox. The HUD shows the time, the body's needs, your standing and the day's chores.

At dusk the first teaching begins, The Rope. Three snakes lie on the path between the village and
the ghat. A fear meter rises as you come near one, and you cannot bring yourself to walk up to it.
Light the lamp (F), go as close as you dare and look at it steadily for about two seconds: it is a
rope, and it stays a rope. Nothing is scored. After the last one the fear meter leaves the HUD
and does not come back.

The second teaching, Clay and Pots, is at the potter's wheel. Throw a water jar, a bowl, a cup or
a lamp (1–4), give it a name, and it goes on the drying board and into your list of things made.
After three pots you have the clay lens (C): pots, bricks, roof tiles and the riverbank all show as
one material, and the list reads "Rice for Amma: clay". Motes of light hang over the board. They
were there before the first pot, and a pot only hides the ones inside it. Break one (B) and its
name leaves the list, while not one mote moves.

The day has its chores, as an ordinary game would: carry water from the ghat to the potter's door,
and dig clay on the riverbank north of the ghat for the wheel (E to fill, give, dig and leave).
Each earns standing in the village. Drink at the well, eat at home, and at night sleep at your door
(E): the screen goes dark and you wake at half past six the next morning, with the chores on the
list again.

The night after the rope and the clay, the third teaching, Three States, comes in sleep. You dream
the valley: the same houses and paths in an evening light, except that the sun runs backwards and
every place wears the next one's name, and nothing on the HUD finds that strange ("Nothing here
seems strange"). After a minute and a quarter the dream goes out, and deep sleep is a black screen
with only the river and a dim point that slowly breathes. Then you wake, and two pages of the codex
are open: Yājñavalkya on the dream, and the fourth, what was there through waking, dream and deep
sleep.

The morning after the deep sleep is the after-mode, and it does not end. The chores come
round again and earn nothing; Standing is gone from the HUD. Press V to step out of the body: the
avatar carries on alone, drinking, eating, carrying water and clay, sitting on the ghat steps,
going home at dusk, while you fly anywhere in the valley. V again steps back in. That morning the
game says the teachings are done and nothing more is asked of you, and the drone of a tanpura comes
in under the valley's sounds; nothing says the game is won. The days after are not all alike: the
second evening the village sets small clay lamps along the ghat steps and floats more on the river
(the morning says so), the third morning a flock of egrets crosses the valley, and after that each
comes round now and then.

Until then the HUD's NEXT line says what comes next, and the names of houses and places show over
them as you near them: your door is indigo, the potter's ochre, the shrine's saffron under a
saffron pennant. After sunset each house lights a lamp by its door.

Every sound is made by the game as it plays, with no recordings: the river, the wind, birds and a
koel by day, crickets at night, footsteps on earth and on the ghat's stone, the wheel, a heartbeat
while there is fear, a bell for each new page of the codex. M (or the page's Sound button) turns it
off.

The codex (Tab) holds each verse the game has taught, as a page with the Sanskrit in Devanagari and
IAST, an English translation, the translator, the edition and the sources. A page opens when its
teaching happens: the rope and the snake (Gauḍapāda's Māṇḍūkya Kārikā 2.17–2.18), clay and its names
(Chāndogya Upaniṣad 6.1.4), the space in the pot (Kārikā 3.3–3.4), the dream (Bṛhadāraṇyaka
Upaniṣad 4.3.9–10 and 4.3.14) and the fourth (Māṇḍūkya Upaniṣad 2 and 7). One page is open from the
start: how Advaita, Viśiṣṭādvaita and Dvaita each read *tat tvam asi*. The game pauses while the
codex is open.

| The well, at a quarter to five | The ghat steps down to the river |
|---|---|
| ![First person by the village well: lime-washed houses with tiled roofs, the potter's drying pots, the HUD with needs, the NEXT line and chores](docs/images/m1-well.jpg) | ![Stone ghat steps leading down to the river at late afternoon](docs/images/m1-ghat.jpg) |

| The village from the ghat | Sunset over the river, just after six |
|---|---|
| ![Looking west from the ghat over the village toward the low sun](docs/images/m1-village-from-ghat.jpg) | ![The river at sunset, the sky orange over dark hills](docs/images/m1-sunset.jpg) |

| Lamplight on a snake at half past seven | It was a rope |
|---|---|
| ![At night by lamplight, a dark snake coiled on the path with its head raised, lamps lit by the village doors behind; the HUD shows the fear meter high and NEXT reads "Walk back to the village along the path"](docs/images/m2-lamp-on-the-snake.jpg) | ![The same spot: a straw-coloured rope lies coiled on the path where the snake was](docs/images/m2-rope.jpg) |

| Throwing a water jar | Three pots, three names |
|---|---|
| ![At the potter's wheel, a water jar rising on the wheel; the prompt reads "Throwing a water jar"](docs/images/m3-at-the-wheel.jpg) | ![A lamp, a bowl and a water jar on the drying board; the HUD lists Ganga water, Rice for Amma, Evening lamp under MADE](docs/images/m3-named-pots.jpg) |

| Through the clay lens | The jar broken, the space unmoved |
|---|---|
| ![Through the clay lens: pots, the board's brick piers and the house plinth in one clay colour, everything else grey; the list reads "Ganga water: clay"](docs/images/m3-clay-lens.jpg) | ![Shards of the water jar on the board among motes of light; Ganga water has left the list](docs/images/m3-pot-space.jpg) |

| The codex: the rope and the snake | Other schools |
|---|---|
| ![The codex page for Māṇḍūkya Kārikā 2.17–2.18: the verse in Devanagari and IAST, an English translation, a note and the sources, with the codex menu on the left](docs/images/m4-codex-rope.jpg) | ![The other-schools page: Chāndogya 6.8.7 and the readings of Advaita, Viśiṣṭādvaita and Dvaita, with citations](docs/images/m4-other-schools.jpg) |

| The dream, at a quarter to six | Later in the dream: the sun back in the afternoon |
|---|---|
| ![The dream, through a faint violet haze: the village in dusk light from your door; the HUD's NEXT reads "Nothing here seems strange" and the well is labelled "The potter's wheel"](docs/images/m6-dream.jpg) | ![Further on in the dream, by the well in bright afternoon sun at 2:38 pm by the HUD's clock; the well is still labelled "The potter's wheel" and a house "The shrine"](docs/images/m6-dream-sun.jpg) |

| The codex: the dream | The codex: the fourth |
|---|---|
| ![The codex page for Bṛhadāraṇyaka Upaniṣad 4.3.9–10 and 4.3.14 in Devanagari, IAST and Hume's English: "There are no chariots there, no spans, no roads. But he projects from himself chariots, spans, roads."](docs/images/m6-codex-dream.jpg) | ![The codex page for Māṇḍūkya Upaniṣad 2 and 7 in Devanagari, IAST and Hume's English, ending "such they think is the fourth. He is the Self. He should be discerned."](docs/images/m6-codex-fourth.jpg) |

| The next morning | Watching the avatar go about its day |
|---|---|
| ![Waking at 6:35 on 22 March outside your house in the dawn light; the potter's house is named and the shrine's saffron pennant shows over the roofs; NEXT reads "Nothing more is asked of you", the chores are listed again "as you like", the HUD reads "Living, step back and watch (V)", and there is no Standing line](docs/images/m5-morning.jpg) | ![From behind and above, the avatar in white and brown walks toward the potter's yard; the HUD reads "Watching, step back in (V)"](docs/images/m5-watching.jpg) |

| A lamp night, the second evening | Egrets, the third morning |
|---|---|
| ![Night on the ghat steps: small clay lamps burning along four of the steps, more far out on the dark river](docs/images/m7-lamp-night.jpg) | ![Early morning from the ghat: a V of nine white egrets over the river against a pale blue sky](docs/images/m7-egrets.jpg) |

*Frames from the game's own headless tour (`atman --headless --tour --frames DIR`).*

## Build and play

Atman builds with CMake 3.24 or newer and a C++17 compiler. Strata, and the SDL3 it uses, are
fetched and built along with it, at the commit `ATMAN_STRATA_REF` in `CMakeLists.txt` pins
(Strata v0.6.0).

```sh
cmake -S . -B build
cmake --build build --config Release
./build/atman               # Windows: build\Release\atman.exe
ctest --test-dir build -C Release
```

On Windows, `tools/play-atman.bat` does all of this in one step. It installs CMake and the Visual
Studio C++ build tools with winget if they are missing, clones or updates `main` into
`%USERPROFILE%\atman-play`, builds the game and starts it. Extra arguments go to the game, for
example `play-atman.bat --tour`, or `play-atman.bat --autopilot --speed 10` to watch the autopilot
play the whole slice at ten times speed.

The game continues from its save: `%APPDATA%\Atman\default` on Windows, `~/.local/share/atman/default`
elsewhere, or `--slot DIR`. It saves every two minutes, on F5 and on quit, and keeps the save before
last. A save that fails its checksum is refused with a message and the previous one loads.
`--new` starts over (the old save is kept as the previous one). `--autopilot` never saves. A
second village (F6) is saved beside the first, in `default-2` (or `DIR-2`); the settings (text
size, captions, which village) are in the first's `settings.txt`.

On Linux, SDL3 needs the X11 development packages (`libx11-dev libxext-dev libxcursor-dev libxi-dev
libxfixes-dev libxrandr-dev libxss-dev`). `cmake -DATMAN_WITH_STRATA=OFF` builds only the
simulation and its tests, with no engine.

| Input | Action |
|---|---|
| Mouse | Look |
| W A S D | Walk |
| Shift | Hurry |
| E | Use what is here: fill the jar, give the water, dig or leave clay, drink, eat, sleep |
| F | Light the lamp, or put it out |
| 1 2 3 4 | At the wheel: throw a water jar, a bowl, a cup or a lamp, then type its name and press Enter |
| B | Break the pot you are looking at |
| C | The clay lens (after three pots) |
| Tab | The codex (Up and Down, or W and S, choose a page; Tab or Esc goes back) |
| V | In the after-mode: step out and watch the avatar (W A S D and the mouse fly), or step back in |
| M | Sound on or off |
| F1 | Show or hide the controls |
| H | Hide or show the HUD, to see the valley |
| F2 | Text size: normal, large, larger |
| F3 | Captions for the sounds that matter (a heartbeat when afraid, the wheel, the river in deep sleep); they also show while the sound is off |
| F6 | Your other village: the game keeps two, each with its own save |
| F5 / F9 | Save now / load the last save |
| Esc | Free the mouse (Esc again quits) |

## Documents

| Document | What it covers |
|---|---|
| [CONCEPT.md](CONCEPT.md) | The game: the design problem, the valley, each chapter's teaching and mechanic, teaching rules, tone |
| [MVP.md](MVP.md) | The first playable slice, what "done" means for it, and the milestones |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Code layout, the simulation core, how each mechanic maps to Strata, text, saves, tests |
| [RESEARCH.md](RESEARCH.md) | Sources: Upaniṣads, Śaṅkara and later Advaita, modern scholarship, Alan Watts and other modern teachers, related games, and a verification log |
| [docs/REVIEW.md](docs/REVIEW.md) | The review packet: every codex page's Sanskrit and English, word-by-word glosses of the working translations, questions for a reviewer, how to send feedback, and the checks that need real hardware |
| [docs/WEB.md](docs/WEB.md) | The browser build: what differs from the desktop game, touch controls, saves in the browser, building it, CI and deployment |
| [docs/RELEASING.md](docs/RELEASING.md) | Cutting a release: the version, playtest evidence and the Windows and Linux downloads |
| [docs/playtests/](docs/playtests/) | Playtest notes for each release, and what was changed for them |
| [docs/THIRD-PARTY.md](docs/THIRD-PARTY.md) | The licences of the third-party code in the game and its downloads, and of the plates' font |

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

## The codex plates

Strata draws ASCII text only, so each codex page is an image ("plate") made ahead of time from
`content/codex/*.json` and committed under `content/plates/`. After changing an entry, run
`python3 tools/plates/make_plates.py` (it needs Pillow built with libraqm, for Devanagari shaping).
`python3 tools/plates/make_plates.py --check` needs nothing beyond Python. It checks that every
citation is defined, every entry names a translator, and every plate is current. CI runs it.

## License

[MIT](LICENSE). The plates are set in Tiro Devanagari Sanskrit, under the SIL Open Font License
([tools/plates/fonts/OFL.txt](tools/plates/fonts/OFL.txt)). The game decodes them with
[stb_image](https://github.com/nothings/stb) (public domain or MIT, `third_party/stb/`), and plays
sound through [miniaudio](https://github.com/mackron/miniaudio) 0.11.25 (public domain or MIT No
Attribution, `third_party/miniaudio/`).
