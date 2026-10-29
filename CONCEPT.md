# Concept

Atman is a small 3D game about the oldest question in Advaita Vedānta: *who is playing?* You wake
in a river village with a body, a name and a list of chores. By the end you are still there, still
doing the chores, and nothing is at stake.

Bracketed IDs such as [P1] or [S4] refer to sources in [RESEARCH.md](RESEARCH.md).

## Pitch

Atman starts as an ordinary cozy game. You have a health bar, a reputation, a potter's errands and
a fear of snakes on the path at dusk. Over five chapters each of those things is examined the way
the Upaniṣads examine them. The snake turns out to be a rope. The pots turn out to be clay. The
body, the breath and the mind are each offered as "you" and each is set aside. The game's own
interface comes apart along the way.

The teaching follows classical Advaita as Śaṅkara taught it [C1], with the Upaniṣadic verses
quoted on screen in Devanagari, transliteration and translation. Alan Watts sets the tone. He
treated the whole thing as a cosmic game of hide-and-seek in which the seeker forgot, on purpose,
that it was also the one hiding [M2]. The tradition has its own word for that, *līlā*, play [P10].

| | |
|---|---|
| Audience | Curious adults; no prior knowledge assumed |
| Length | About three hours, then open-ended |
| Engine | [Strata](https://github.com/Preponderous-Software/strata) |
| Targets | Windows and Linux desktop, and the browser (WebGL 2) on the arcade |

## The design problem

**Games reward action. Advaita says no action produces liberation.**

For Śaṅkara, *mokṣa* is the self's own nature, already there. It is uncovered by knowledge and is
never manufactured by deeds [C1]. Rambachan calls this "accomplishing the accomplished" [S4]. A
quest marker, an XP bar or an "Enlightened" trophy would teach the opposite of the subject.

So the game uses its conventions as the material it examines. Every system a player expects
(goals, fear, inventory, identity, an ending) gets looked at closely, and that close look is the
progress. Knowledge is the only thing that advances, as in *Outer Wilds* [G2]. No achievement is
ever awarded for mokṣa.

## Structure: one valley, four states

The whole game takes place in one valley: a village, a potter's yard, a well and a river ghat.
That keeps the scope to one hand-built scene. The Māṇḍūkya Upaniṣad supplies the spine [P3]. The
same valley is played in three states, and the question is what stays present through all of them.

| State | Sanskrit | In the game |
|---|---|---|
| Waking | *jāgrat* | The shared, lit world: sun, shadows, chores |
| Dream | *svapna* | The same valley, private and unstable: geometry drifts, rules bend, nothing seems strange |
| Deep sleep | *suṣupti* | No objects at all: a black screen with the river audible |
| The fourth | *turīya* | Not a fourth state. It is what is aware of all three. In the game it is the witness camera. |

## Chapters

The order matters. Each chapter takes away one thing the player was holding, and the next chapter
assumes it is gone.

### Prologue: The Seeker

- **Teaching:** You forgot on purpose. Brahman hides as each of us and then goes looking [M2]. In
  the tradition's own terms, creation is *līlā*, done for its own sake [P10].
- **Mechanic:** A deliberately ordinary cozy game: walk, carry water, help the potter, earn
  standing in the village. There is a full HUD (needs, reputation, a quest log). Two birds sit on
  a tree by the ghat, one eating and one watching. The player won't understand them yet [P6].

### 1. The Rope (*adhyāsa*, अध्यास)

- **Teaching:** Superimposition: in dim light we lay "snake" over a rope. The fear is real. The
  snake never was [C1]. Advaita uses this for the self mistaken for the body and mind.
- **Mechanic:** At dusk, snakes block the paths and a fear meter rises. Carrying a lamp and looking
  closer turns each snake back into rope. The fear meter is the first HUD element to go for good.
  The rope stays on the path and is still a rope: the village is real for living in, and only the
  snake was an error [S1].

### 2. Clay and Pots (*ghaṭākāśa*, घटाकाश)

- **Teaching:** "By one lump of clay all that is made of clay is known; the modification is only a
  name" [P1]. Gauḍapāda adds the space in a pot, which seems separate and was never divided [C2].
- **Mechanic:** The player throws pots at the wheel and names them, and the names go into the
  inventory. A clay lens shows every pot, bowl and brick as the same material. Breaking a pot makes
  no change to the space it held. The inventory starts listing names only.

### 3. Five Sheaths (*neti neti*, नेति नेति)

- **Teaching:** The Taittirīya's five sheaths of food, breath, mind, intellect and bliss [P5],
  examined by *neti neti*, "not this, not this" [P2].
- **Mechanic:** Five chambers in the old well. Each one offers itself as you: the body, the breath,
  the thoughts, the clever reasoning about all of this. In each chamber the player notices that it
  can be *seen*, so it is not the seer. The fifth chamber, bliss, is pleasant on purpose, and the
  game lets players stay there as long as they like. Leaving it is the hard part.

### 4. Three States (*turīya*, तुरीय)

- **Teaching:** Waking, dream and deep sleep come and go. What is present to all three [P3]? "By
  what would one know the knower?" [P2]. Ramana Maharshi's question applies too: *who is asking?*
  [M6]
- **Mechanic:** The player sleeps. The valley is replayed as dream (the potter is also your mother,
  and nothing seems strange) [C2] [M4], then as deep sleep, where no input does anything. On waking,
  the camera can leave the avatar for the first time. This is the witness view (*sākṣī*), and the
  second bird finally makes sense [C6].
- **As built (after the MVP):** the first night after chapters 1 and 2. In the dream the sun runs
  backwards and each place wears the next one's name, and the HUD says "Nothing here seems
  strange", with "Nothing to do; the dream passes by itself" under it; deep sleep is black, with
  the river, a dim, slowly breathing point and the faint line "Deep sleep. Morning comes by
  itself." (#37); on waking the
  codex opens the dream (Bṛhadāraṇyaka 4.3.9–10, 4.3.14) [P2] and the fourth (Māṇḍūkya 2, 7) [P3],
  and the after-mode begins. The sheaths (chapter 3 here) are not built yet.

### 5. Salt in Water (*tat tvam asi*, तत् त्वम् असि)

- **Teaching:** Uddālaka has his son Śvetaketu dissolve salt in water overnight, then taste it from
  the top, the middle and the bottom. It is everywhere and cannot be seen. "That is the self. That
  you are" [P1]. The grammar of that refrain is debated [S11], and the codex says so.
- **Mechanic:** The teacher's dialogue is quoted from the Chāndogya and paced by the player. The
  remaining HUD dissolves piece by piece into the river, and nothing replaces it. The
  *mahāvākyas* are recorded in the codex [P2] [P3] [P4].

### After: Living Free (*jīvanmukti*, जीवन्मुक्ति)

- **Teaching:** Liberation while living. The body goes on playing out the karma already in motion
  (*prārabdha*) [C5]. One acts without claim on the fruits [P9].
- **Mechanic:** No credits. The avatar keeps living on its own, carrying water and helping the
  potter, while the player watches from anywhere or steps back in. The quest log still lists
  chores, with no timers or failure. Dusk still comes. Ropes are ropes. The player can quit
  whenever they like, and quitting is not a loss.

> स य एषोऽणिमैतदात्म्यमिदं सर्वं तत्सत्यं स आत्मा तत्त्वमसि श्वेतकेतो
>
> *sa ya eṣo 'ṇimaitad ātmyam idaṃ sarvaṃ tat satyaṃ sa ātmā tat tvam asi śvetaketo*
>
> "That which is the finest essence, this whole world has it as its self. That is the true. That
> is the self. That you are, Śvetaketu."
>
> Chāndogya Upaniṣad 6.8.7 [P1]. Translation adapted from Hume (1921). Brereton notes that the
> final *tat* is neuter, so "That you are" is already an interpretation [S11].

## Teaching integrity

These rules outrank any gameplay idea.

- **Classical first, modern voices labelled.** Doctrine comes from the Upaniṣads and Śaṅkara.
  Watts, Vivekananda and Ramana appear in the codex as later readings [M1] [M5].
- **No effect claims to be mokṣa.** Visuals illustrate analogies. Following Rambachan,
  understanding comes from the teaching itself and is not a special experience the game can
  trigger [S4].
- **The world stays real enough to live in.** Māyā is handled with Advaita's three grades of
  reality. The village, the people and the chores are empirically real. Only the snake was an error
  [S1] [S5].
- **Other schools get their say.** A codex page explains that Viśiṣṭādvaita and Dvaita read *tat
  tvam asi* differently, so Advaita is not presented as "what Hinduism says" [S3].
- **Every verse is cited.** Devanagari, IAST and translation, with the source and verse number on
  screen. Disputed attributions are marked "attributed to Śaṅkara" [C4].
- **No exotic set dressing.** No deities as props, and no Om as a collectible. The valley is a
  working village drawn from life [S10].
- **Reviewed before release.** One academic and one teacher from a living Advaita lineage sign off
  before a build is published.

## Voice: the Watts register

Watts gives the game permission to be light. The prologue's narration is playful and a little
conspiratorial: *you hid yourself very well this time.* That paraphrases the hide-and-seek passage
of *The Book* [M2]. The after-mode borrows the present-tense ease of *The Wisdom of Insecurity*
[M3].

Watts's recordings are not used. *Everything* licensed them from the Alan Watts Organization
[G1]. Atman paraphrases and cites instead, and keeps the doctrine itself classical.

## Open questions

- Which translation goes in-game: Hume (public domain) or a licensed Olivelle? [P1]
- Who reviews it? Candidates are an Indologist and a teacher from the Ramakrishna Order or Arsha
  Vidya [M8] [M9].
- Does devotional (*bhakti*) Advaita belong anywhere beyond the codex?
