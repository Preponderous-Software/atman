# Atman

A small 3D game about Advaita Vedānta: the teaching that the self (Ātman) is the absolute
(Brahman), and that liberation (mokṣa) is a recognition, not a reward. Atman is planned to run on
[Strata](https://github.com/Preponderous-Software/strata), a small C++ engine.

You wake in a river village with a body, a name and a list of chores. At dusk a snake on the path
turns out to be a rope. The potter's pots turn out to be clay. Chapter by chapter, the things you
took to be "you" are examined the way the Upaniṣads examine them, and the game's own interface
comes apart. At the end nothing rolls credits. You are still in the village, still doing the
chores, and nothing is at stake.

**Status: design only.** Nothing is built yet. The documents below describe the game, the first
playable slice, and how it will be built.

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

## License

[MIT](LICENSE).
