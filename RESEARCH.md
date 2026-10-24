# Atman: Research Notes

Background research for **Atman**, a proposed educational game on the Strata engine about Advaita Vedānta: Ātman, Brahman, māyā, and mokṣa.

[CONCEPT.md](CONCEPT.md), [MVP.md](MVP.md) and [ARCHITECTURE.md](ARCHITECTURE.md) cite the IDs below, for example `[P3]` or `[M2]`. Sources are grouped as:

- **P**: primary texts
- **C**: classical Advaita authors after the Upaniṣads
- **S**: modern scholarship
- **M**: modern teachers and popularizers, Alan Watts among them
- **G**: games used as design references

Status: draft, 2026-10-22. All entries once marked (verify) were checked on 2026-10-22; see §8, Verification log. Before public release, recheck every in-game quotation against the printed translation it uses.

---

## 1. Doctrinal stance

The game follows **classical Advaita as taught by Śaṅkara (c. 8th century CE)** and the Upaniṣads he commented on. Modern popularizers, Watts included, are labelled as later readings. They shape the game's tone. They do not supply its doctrine.

The core claims the game has to get right:

| Claim | Sanskrit | Main sources |
|---|---|---|
| The self (Ātman) is identical with the absolute (Brahman) | *tat tvam asi*, *aham brahmāsmi* | P1, P2, P4, P5 |
| The world of many things is real as appearance and unreal as independent substance | *māyā*, *vivarta* | C1, C2, S2 |
| Bondage comes from ignorance (*avidyā*), which superimposes the not-self on the self | *adhyāsa* | C1 (Adhyāsa Bhāṣya) |
| Liberation (*mokṣa*) is the removal of ignorance by knowledge (*jñāna*). It is not something produced by action. | *jñāna*, *karma* | C1, S4 |
| Knowledge arises from the teaching itself (*śruti* as a *pramāṇa*, a valid means of knowing), by listening, reflection and contemplation | *śravaṇa, manana, nididhyāsana* | P2 2.4.5, S4 |
| A person can be liberated while still embodied, and the body plays out its remaining karma | *jīvanmukti*, *prārabdha* | C5 |

### Points to handle carefully

1. **Liberation is not an achievement.** For Śaṅkara, action (*karma*) cannot produce mokṣa, because mokṣa is the self's nature that is already there. It is "accomplishing the accomplished" (S4). Games are built on action and reward, so this is the central design tension. The concept doc turns it into a mechanic: the game never awards mokṣa.
2. **Experience versus knowledge.** Neo-Vedānta (Vivekananda and others) often describes realization as a special *experience* (*anubhava*). Rambachan (S4, S5) argues that Śaṅkara treats the Upaniṣadic teaching as the means of knowledge, and that this is different from mystical experience. The game must not suggest that a visual effect or a trance state *is* mokṣa. Effects illustrate an analogy. The understanding is the player's.
3. **Other Vedānta schools read the same verses differently.** Viśiṣṭādvaita (Rāmānuja, 11th–12th c.) and Dvaita (Madhva, 13th c.) are living traditions with large followings, and they reject Advaita's identity reading of *tat tvam asi*. An in-game codex page should say this plainly, so the game does not present Advaita as "what Hinduism says." (S1, S6, S12, S13)
4. **Māyā is not "the world is fake."** Advaita has three grades of reality: *pāramārthika* (absolute), *vyāvahārika* (empirical, transactional) and *prātibhāsika* (illusory, as in the rope-snake). The village, tasks and people in the game are *vyāvahārika*: real for living in. Only the snake is *prātibhāsika*. (C1, S2, S3)
5. **Attribution of texts.** The *Vivekacūḍāmaṇi* and many hymns (the *Nirvāṇa Ṣaṭkam* among them) are traditionally credited to Śaṅkara, but scholars dispute this (S3, Hacker via S3). Cite them as "attributed to Śaṅkara."
6. **No deity caricatures, no exotic set dressing.** The setting is a river village drawn from life. It is not a mystical-East backdrop. Sacred syllables (Om) and images are not used as decoration or as pickups.

---

## 2. Primary texts (śruti and smṛti)

| ID | Text | Passages used | Use in the game |
|---|---|---|---|
| P1 | **Chāndogya Upaniṣad** | 6.1.4 (clay and its modifications, "only a name"); 6.8.7 and refrain through 6.16 (*tat tvam asi*); 6.13 (salt dissolved in water); 6.12 (banyan seed) | Ch. 2 Clay; Ch. 5 Salt; Uddālaka and Śvetaketu as the teaching dialogue |
| P2 | **Bṛhadāraṇyaka Upaniṣad** | 1.4.10 (*aham brahmāsmi*); 2.3.6 (*neti neti*); 2.4.5 (self to be seen, heard, reflected on, contemplated); 2.4.14 / 4.5.15 ("by what would one know the knower?"); 4.3 (Yājñavalkya on dream and the self as its own light) | Ch. 3 Sheaths (*neti neti*); Ch. 4 States (dream); the witness |
| P3 | **Māṇḍūkya Upaniṣad** | 2 (*ayam ātmā brahma*); 3–7 (waking, dream, deep sleep, *turīya*) | Ch. 4 Three States, the game's structural spine |
| P4 | **Aitareya Upaniṣad** | 3.1.3 (*prajñānaṃ brahma*, "consciousness is Brahman"): chapter 3, section 1, verse 3. Some popular sources shorten it to "3.3". | Codex: the four *mahāvākyas* |
| P5 | **Taittirīya Upaniṣad** | 2.1–2.5 (five sheaths: *annamaya, prāṇamaya, manomaya, vijñānamaya, ānandamaya*); 3.1–3.6 (Bhṛgu's inquiry) | Ch. 3 Five Sheaths |
| P6 | **Muṇḍaka Upaniṣad** | 3.1.1–2 (two birds on one tree: one eats, one watches) | Witness camera; opening image |
| P7 | **Kaṭha Upaniṣad** | 1.3.3–4 (self as chariot-owner, body as chariot); 1.2 (Naciketas and Death) | Prologue framing |
| P8 | **Īśa Upaniṣad** | 6–7 (one who sees all beings in the self) | Ending text |
| P9 | **Bhagavad Gītā** | 2.20 (the self is not born and does not die); 2.47 (act without claim on the fruits); ch. 13 (field and knower of the field) | Post-ending: acting without stake |
| P10 | **Brahma Sūtra** | 2.1.33 *lokavat tu līlākaivalyam* (creation as *līlā*, play, "as in the world") | Grounds the "game" metaphor in the tradition itself, not only in Watts |

**Translations to choose from**

- Patrick Olivelle, *The Early Upaniṣads: Annotated Text and Translation* (Oxford University Press, 1998). Scholarly, with Sanskrit on facing pages. A licence would be needed to quote it in-game.
- Patrick Olivelle, *Upaniṣads* (Oxford World's Classics, 1996). Readable, but under copyright.
- Robert Ernest Hume, *The Thirteen Principal Upanishads* (Oxford, 1921). **Public domain in the US**, which makes it the safest source for in-game quotations. Some of its English is dated.
- F. Max Müller, *The Upanishads*, Sacred Books of the East vols. 1 (1879) and 15 (1884). Public domain. Older still.
- George Thibaut, *The Vedānta-Sūtras with the Commentary by Śaṅkarācārya*, SBE vols. 34 (1890) and 38 (1896). Public domain. Includes the Adhyāsa Bhāṣya and the BS 2.1.33 commentary.

Recommendation: quote **Hume (1921)** in-game, lightly modernized and marked as such. Check every quotation against Olivelle (1998). Show the Sanskrit in Devanagari with IAST beside it.

---

## 3. Classical Advaita authors

| ID | Author / text | Relevance |
|---|---|---|
| C1 | **Śaṅkara**, *Brahmasūtra Bhāṣya*, especially the introduction (*Adhyāsa Bhāṣya*) on superimposition, and 2.1.33 on *līlā* | Rope-snake, superimposition, three grades of reality |
| C2 | **Gauḍapāda**, *Māṇḍūkya Kārikā* (*Āgama Śāstra*), c. 6th c. Ch. 2 (dream analogy; 2.32 on no origination and no dissolution, *ajātivāda*); ch. 3 vv. 3–9 (pot-space, *ghaṭākāśa*: 3.3 pot made, 3.4 pot broken, 3.5–3.7 many pot-spaces and one space, 3.8 apparent impurity, 3.9 point of the simile) | Ch. 2 pot-space; Ch. 4 dream |
| C3 | **Śaṅkara**, *Upadeśasāhasrī* | Undisputed teaching manual. Good for teacher dialogue patterns. |
| C4 | *Vivekacūḍāmaṇi* (attributed to Śaṅkara, authorship disputed) | Four qualifications (*sādhana-catuṣṭaya*): discrimination, dispassion, six virtues, longing for freedom. Rope-snake verses. |
| C5 | **Vidyāraṇya**, *Jīvanmuktiviveka* and *Pañcadaśī* (14th c.) | Liberation while living; the post-ending design |
| C6 | *Dṛg-Dṛśya-Viveka* (attribution varies) | Seer and seen. Basis for the witness camera. |
| C7 | **Sadānanda**, *Vedāntasāra* (15th c.) | Concise standard summary. Useful for checking the codex. |
| C8 | *Aṣṭāvakra Gītā* and *Yoga Vāsiṣṭha* | Popular nondual texts outside Śaṅkara's school. Codex mention only. |

---

## 4. Modern scholarship

| ID | Work | Why it matters |
|---|---|---|
| S1 | Eliot Deutsch, *Advaita Vedānta: A Philosophical Reconstruction* (East-West Center Press, 1969) | Clear, short philosophical overview. Primary reference for the codex text. |
| S2 | Eliot Deutsch and J. A. B. van Buitenen, *A Source Book of Advaita Vedānta* (Honolulu: University Press of Hawaii, 1971; ix + 335 pp.) | Translated excerpts across the tradition |
| S3 | Karl H. Potter (ed.), *Encyclopedia of Indian Philosophies, Vol. III: Advaita Vedānta up to Śaṃkara and His Pupils* (Princeton / Motilal Banarsidass, 1981) | Authorship questions, dating, summaries of every text |
| S4 | Anantanand Rambachan, *Accomplishing the Accomplished: The Vedas as a Source of Valid Knowledge in Śaṅkara* (University of Hawaii Press, 1991; SACP Monograph) | Liberation as knowledge from the teaching, not as an experience. The anti-achievement design rests on this. |
| S5 | Anantanand Rambachan, *The Advaita Worldview: God, World, and Humanity* (SUNY Press, 2006) | Accessible. Engages social ethics, so it supports the "village is still real" stance. |
| S6 | Arvind Sharma, *Advaita Vedānta: An Introduction* (Delhi: Motilal Banarsidass, 2004; 2nd ed. 2007; ISBN 978-81-208-2027-2) | Introductory. Candidate first book for players. |
| S7 | Michael Comans, *The Method of Early Advaita Vedānta* (Motilal Banarsidass, 2000) | Gauḍapāda through Śaṅkara and Sureśvara. Teaching method. |
| S8 | Natalia Isayeva, *Shankara and Indian Philosophy* (SUNY Press, 1993) | Historical context for Śaṅkara |
| S9 | Paul Deussen, *The System of the Vedânta* (trans. Charles Johnston, 1912) | Public domain. Historically influential on Western readings, Watts's among them. |
| S10 | Wilhelm Halbfass, *India and Europe: An Essay in Understanding* (SUNY Press, 1988) | How the West received Vedānta. Helps the game avoid repeating colonial or Orientalist framings. |
| S11 | Joel P. Brereton, "Tat Tvam Asi in Context," *Zeitschrift der Deutschen Morgenländischen Gesellschaft* 136:1 (1986), 98–109 | Grammar of the refrain: the final *tat* is neuter, so the standard "That thou art" is an interpretation. The codex should mention this and avoid overstating the identity reading as plain grammar. |
| S12 | Shyam Ranganathan, "Rāmānuja (c. 1017 – c. 1137)," *Internet Encyclopedia of Philosophy* ([iep.utm.edu/ramanuja](https://iep.utm.edu/ramanuja/)) | Viśiṣṭādvaita's reading of *tat tvam asi*: co-ordinate predication (*sāmānādhikaraṇya*), and selves as modes of Brahman's body. The codex's other-schools page. |
| S13 | Valerie Stoker, "Madhva (1238—1317)," *Internet Encyclopedia of Philosophy* ([iep.utm.edu/madhva](https://iep.utm.edu/madhva/)) | Dvaita: Brahman alone independent, selves dependent; Madhva's *atat tvam asi*, "you are not that". The codex's other-schools page. |

---

## 5. Modern teachers and popularizers

These shape tone, pacing and the player's first contact with the ideas. Each is labelled in-game as a modern voice.

| ID | Source | What the game takes from it |
|---|---|---|
| M1 | **Alan Watts**, *The Supreme Identity: An Essay on Oriental Metaphysic and the Christian Religion* (Faber and Faber, 1950; Noonday, 1957) | Watts's most direct Vedānta book. Brings Vedānta into Western philosophical language. |
| M2 | **Alan Watts**, *The Book: On the Taboo Against Knowing Who You Are* (Pantheon, 1966) | The central influence on tone: the cosmic game of **hide-and-seek** (Brahman playing at being each of us and forgetting on purpose), and the "skin-encapsulated ego." It gives the game its playful, unsolemn voice. |
| M3 | **Alan Watts**, *The Wisdom of Insecurity* (Pantheon, 1951) | Present-moment framing for the post-ending "nothing at stake" mode |
| M4 | **Alan Watts**, lectures, notably "The Dream of Life" (dream-a-world thought experiment) | Ch. 4 dream state. Paraphrase only. |
| M5 | **Swami Vivekananda**, *Jnana-Yoga* (lectures, published 1899; Complete Works vol. 2) | Origin of much modern English Vedānta vocabulary. Labelled as Neo-Vedānta (see S4). |
| M6 | **Ramana Maharshi**, *Who Am I?* (*Nān Yār?*, 1901 conversations, published 1923 onward) | Self-inquiry (*ātma-vicāra*): the "Who is asking?" prompt in Ch. 4 |
| M7 | **Nisargadatta Maharaj**, *I Am That* (ed. Maurice Frydman, Chetana, 1973) | Plain conversational voice. Reference for teacher dialogue. |
| M8 | **Swami Sarvapriyananda** (Vedanta Society of New York), Māṇḍūkya lecture series (public video) | Contemporary monastic teacher. Best modern walkthrough of the three states. Candidate reviewer contact. |
| M9 | **Swami Dayananda Saraswati** (Arsha Vidya Gurukulam) | Traditional *śravaṇa*-based teaching lineage that agrees with S4 |
| M10 | Aldous Huxley, *The Perennial Philosophy* (Harper, 1945) | Shaped Watts's generation. The game should not adopt its "all religions say the same thing" claim. |

### Watts: how far to go

- **Use:** the hide-and-seek and *līlā* framing (anchored in Brahma Sūtra 2.1.33, P10, so it is not Watts alone), his humour, and his refusal to be solemn. The prologue's "you forgot on purpose" line paraphrases M2.
- **Don't use:** his audio. The lectures are controlled by the Alan Watts Organization. *Everything* (G1) licensed them, and the organization sells that game's lecture set as an album. A licence is possible later but is out of scope.
- **Label:** Watts mixes Vedānta with Zen, Taoism and Christian mysticism. Where the game borrows his framing, the codex notes it is "Watts's reading, in the spirit of Advaita."

---

## 6. Games as design references

| ID | Game | Lesson for Atman |
|---|---|---|
| G1 | ***Everything*** (David OReilly, 2017) | Watts narration over play in which the player can become anything. Shows the theme works in a game. Atman differs by teaching a specific tradition with citations. |
| G2 | ***Outer Wilds*** (Mobius Digital, 2019) | Progress through knowledge only: no upgrades, and the player is the only thing that changes. The closest existing match to *jñāna* as the means. |
| G3 | ***The Witness*** (Jonathan Blow, 2016) | Insight as gameplay, and recorded lectures on enlightenment hidden in the world. Shows how to place philosophical audio and text without lecturing. Its title shares our witness-consciousness theme. |
| G4 | ***Journey*** (thatgamecompany, 2012) | Wordless arc, anonymous companions, and an ending that loops to the beginning |
| G5 | ***Proteus*** (Ed Key and David Kanaga, 2013) | A world whose only verb is noticing. Seasons as state shifts. |
| G6 | ***The Stanley Parable*** (Galactic Cafe, 2013) | Plays with goals and the urge to complete. Useful for the ending that withholds an ending. |

---

## 7. Open research questions

1. Which translation goes in-game: Hume (public domain) or a licensed Olivelle?
2. Who reviews the game? Ideally one academic Indologist and one teacher from a living Advaita lineage (Ramakrishna Order, Arsha Vidya, or Śṛṅgeri). Get their sign-off before any public release.
3. Should devotional (*bhakti*) Advaita have a place, given Śaṅkara's hymns? Currently it is codex-only.
4. Should the codex cover the Advaita debate about whether *avidyā* resides in the individual or in Brahman (Bhāmatī and Vivaraṇa sub-schools)? Probably too deep for this audience.
5. How should deep sleep (*suṣupti*) be presented so it does not read as "the game froze"? As
   built: twelve seconds of black with the river and one dim point that slowly brightens and fades,
   like breathing. Whether that reads right needs a playtest.
6. Pronunciation audio for Sanskrit terms needs a qualified speaker.

---

## 8. Verification log (2026-10-22)

| Entry | Was | Finding | Result |
|---|---|---|---|
| P4 Aitareya | "3.3 or 3.1.3" | The verse is chapter 3, section 1, verse 3, in the Sanskrit text with a verse-3 marker ([gita.pub 3.1.3](https://gita.pub/upanishad/aitareya/verse/3-1-3/), [Wikipedia](https://en.wikipedia.org/wiki/Aitareya_Upanishad)). "3.3" is a popular shorthand. | Cite as **3.1.3** |
| P7 Kaṭha chariot | 1.3.3–4, numbering unsure | Part 1, canto (*vallī*) 3, verses 3–4. Śaṅkara's commentary uses 1.3.3 ([shlokam](https://shlokam.org/?p=109079), [wisdomlib](https://www.wisdomlib.org/hinduism/book/katha-upanishad-shankara-bhashya/d/doc145206.html), [Ratha Kalpana](https://en.wikipedia.org/wiki/Ratha_Kalpana)). The Sanskrit has *rathinam* or *rathitam* depending on edition. | **Confirmed** |
| C2 pot-space | ch. 3 vv. 3–10 | 3.3 introduces the simile; 3.3–3.9 develop it, and 3.9 discusses its value ([wisdomlib 3.3](https://www.wisdomlib.org/hinduism/book/mandukya-upanishad-karika-bhashya/d/doc143673.html), [chinfo outline](https://chinfo.org/wp-content/uploads/2024/05/41_03_Mandukya_Karika_-_III-36f62731-eb51-43df-b0d0-bb3ee39b8891.pdf), [wisdomlib essay](https://www.wisdomlib.org/hinduism/essay/consciousness-in-mandukya-karika/d/doc1145117.html)). | Corrected to **3.3–3.9** |
| C2 *ajātivāda* verse | 2.32 | "na nirodho na cotpattiḥ…" is Kārikā 2.32. It also appears in several minor Upaniṣads and in *Vivekacūḍāmaṇi* 574 ([advaita-l, 2024-03](https://lists.advaita-vedanta.org/archives/advaita-l/2024-March/061612.html)). | **Confirmed** |
| S2 Source Book | University of Hawaii Press, 1971 | Published as "University Press of Hawaii" (the press's earlier name), Honolulu 1971, ix + 335 pp.; reviewed in JAS 1972 ([Open Library](https://openlibrary.org/books/OL5221303M), [JAS review](https://resolve.cambridge.org/core/journals/journal-of-asian-studies/article/source-book-of-advaita-vedanta-by-eliot-deutsch-and-j-a-b-van-buitenen-honolulu-the-university-press-of-hawaii-1971-ix-335-pp-bibliography-index-1500/02F8B7BC486186A2B5D7EFBB36D5B4C0)). | Publisher name **corrected** |
| S6 Sharma | Motilal Banarsidass, 2004 | First edition 2004 (vii + 125 pp.), 2nd ed. 2007 ([MLBD](https://mlbd.in/products/advaita-vedanta-an-introduction-arvind-sharma-9788120820272-8120820274), [Wikipedia](https://en.wikipedia.org/wiki/Arvind_Sharma)). | **Confirmed** |
| P1 Sanskrit of 6.8.7 (concept page) | written from memory | Matches the standard text *sa ya eṣo 'ṇimaitad ātmyam idaṃ sarvaṃ tat satyaṃ sa ātmā tat tvam asi śvetaketo* ([shlokam 6.8.7](https://shlokam.org/texts/Chandogya-6-8-7), [Columbia class note](https://columbia.edu/itc/religion/nonduality/classnotes/sep10/samaveda_mahavakya.html)). Brereton argues the compound should be read *etadātmyam* and the final *tat* is neuter. | **Confirmed**, and S11 added |

Earlier checks in this session: Watts, *The Supreme Identity* (Faber and Faber, 1950); Rambachan, *Accomplishing the Accomplished* (University of Hawaii Press, 1991); *Everything* uses Watts lectures, with an album sold by the Alan Watts Organization; Brahma Sūtra 2.1.33 *lokavat tu līlākaivalyam*.

### Codex texts (2026-10-22, M4)

| Entry | Checked against | Result |
|---|---|---|
| Hume's English for Chāndogya 6.1.4 and 6.8.7 | The 1921 printing, scanned on the Internet Archive ([thirteenprincipa028442mbp](https://archive.org/details/thirteenprincipa028442mbp)) | Quoted word for word in the codex, except Hume's gloss "(*satya*)" in 6.8.7, which is left out and said so |
| Sanskrit of Chāndogya 6.1.4 and 6.8.7 | GRETIL's Chāndogya Upaniṣad (Sansknet project input) | IAST as GRETIL gives it; Devanagari transliterated from it, with final consonants joined to the next word |
| Sanskrit of Māṇḍūkya Kārikā 2.17–2.18 and 3.3–3.4 | GRETIL's Āgamaśāstra, input by Peter Schreiner from Āpaṭe's Ānandāśrama edition (Pune, 1921) | Matches; the rope-snake verses are 2.17–2.18 and the pot-space verses open at 3.3–3.4, as C2 says |
| English for the Kārikā verses | No public-domain translation was checked | The codex uses a working translation made for Atman from the Sanskrit and says so on the plate. It needs a reviewer's check (§7, question 2). |
| Rāmānuja on *tat tvam asi* | S12 | "That" and "thou" signify an underlying unity with different qualitative content; Brahman stands to selves as soul to body |
| Madhva's *atat tvam asi* | S13 | Carrying the "a" over from the preceding word gives *atat tvam asi*, "you are not that" |
| Hume's English for Bṛhadāraṇyaka 4.3.9–10, 4.3.14 and Māṇḍūkya 2 and 7 (chapter 3, after the MVP) | The same 1921 scan ([thirteenprincipa028442mbp](https://archive.org/details/thirteenprincipa028442mbp)) | Quoted word for word, except Hume's Sanskrit glosses in parentheses, which are left out and said so; 4.3.10 is cut with an ellipsis, shown on the plate |
| Sanskrit of those verses | GRETIL's Bṛhadāraṇyaka Upaniṣad (Kāṇva) and Māṇḍūkya Upaniṣad with Śaṅkara's commentaries (Sansknet project input) | IAST as GRETIL gives it; Devanagari transliterated from it with indic-transliteration, consonant-final words joined to the next |

Not yet checked against a physical copy: Olivelle (1998) for any passage, and Hume's page numbers.
