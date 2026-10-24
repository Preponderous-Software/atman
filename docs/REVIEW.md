# Review packet: the codex texts

Atman is a small game that teaches classical Advaita Vedānta. Before a build is published, the
Sanskrit and English it shows are to be checked by people who know the tradition
([RESEARCH.md §7, question 2](../RESEARCH.md#7-open-research-questions)). This packet puts
everything a reviewer needs in one place, so the request can be handed to someone who has never
seen the game.

## Who this is for, and what is asked

Two reviewers are sought:

- **An academic Indologist** who reads Sanskrit, to check the working translations word by word.
- **A teacher from a living Advaita lineage** (for example the Ramakrishna Order, Arsha Vidya, or
  Śṛṅgeri), to check that the pages say what the tradition teaches and do not overstate it.

What is asked:

1. Read the two **working translations** made for Atman (Gauḍapāda's Māṇḍūkya Kārikā 2.17–2.18 and
   3.3–3.4) against the Sanskrit, with the word-by-word glosses below. These are the main thing to
   review. Say whether each English sentence is a fair translation, and suggest wording where it
   is not.
2. Glance at the four passages quoted from **Hume's 1921 translation**. They are a published,
   public-domain translation, so only the fit is in question: is the excerpt cut in a sensible
   place, and does it say what the page needs?
3. Read the **notes** each page shows, and the **"Other schools"** page, for doctrinal fairness.
4. Answer whichever of the [questions](#questions-for-the-reviewer) you have a view on, and send
   the feedback as described in [How to send feedback](#how-to-send-feedback).

How long it should take: about an hour for the two working translations with the glosses, and
about forty-five minutes for the Hume excerpts, the notes and the questions. No knowledge of the game is
needed, and nothing needs to be installed; each page is also available as the image the game shows.

The translations themselves have not been changed while this packet was written. Doubts found
while glossing are listed as questions instead.

## What the reviewer is looking at

The game's codex holds one page per teaching. Each page is a JSON file in
[content/codex/](../content/codex/) and is drawn as an image (a "plate") by
`tools/plates/make_plates.py`, because the game's text overlay cannot draw Devanagari. A plate
shows, top to bottom: the title, the source, the Devanagari, the IAST, the English, any readings,
the notes, then the translator, the edition and the source IDs (defined in
[RESEARCH.md](../RESEARCH.md)).

| Page | Source | English | Plate |
|---|---|---|---|
| The rope and the snake | Gauḍapāda, Māṇḍūkya Kārikā 2.17–2.18 | **Working translation for Atman** | [rope-and-snake.png](../content/plates/rope-and-snake.png) |
| Clay and its names | Chāndogya Upaniṣad 6.1.4 | Hume (1921), public domain | [clay-and-pots.png](../content/plates/clay-and-pots.png) |
| The space in the pot | Gauḍapāda, Māṇḍūkya Kārikā 3.3–3.4 | **Working translation for Atman** | [pot-space.png](../content/plates/pot-space.png) |
| The dream | Bṛhadāraṇyaka Upaniṣad 4.3.9–4.3.10, 4.3.14 | Hume (1921), public domain | [the-dream.png](../content/plates/the-dream.png) |
| The fourth | Māṇḍūkya Upaniṣad 2 and 7 | Hume (1921), public domain | [the-fourth.png](../content/plates/the-fourth.png) |
| Other schools: "that you are" | Chāndogya Upaniṣad 6.8.7 | Hume (1921), public domain | [other-schools.png](../content/plates/other-schools.png) |

The Sanskrit of every page was checked against GRETIL on 2026-10-22; see the
[Codex texts table](../RESEARCH.md#codex-texts-2026-10-22-m4) in RESEARCH.md. Below, the
Devanagari, IAST, English and notes are copied exactly from the JSON files.

## The pages

### 1. The rope and the snake (working translation)

**Source:** Gauḍapāda, Māṇḍūkya Kārikā 2.17–2.18 [C2]

**Shown in the game:** when the player, at dusk with the lamp lit, has looked steadily at a snake on the path long enough to see that it is a rope.

> अनिश्चिता यथा रज्जुरन्धकारे विकल्पिता ।\
> सर्पधारादिभिर्भावैस्तद्वदात्मा विकल्पितः ॥ २.१७ ॥\
> निश्चितायां यथा रज्ज्वां विकल्पो विनिवर्तते ।\
> रज्जुरेवेति चाद्वैतं तद्वदात्मविनिश्चयः ॥ २.१८ ॥

> *aniścitā yathā rajjur andhakāre vikalpitā*\
> *sarpadhārādibhir bhāvais tadvad ātmā vikalpitaḥ*\
> *niścitāyāṃ yathā rajjvāṃ vikalpo vinivartate*\
> *rajjur eveti cādvaitaṃ tadvad ātmaviniścayaḥ*

English, as shown (**working translation, to be reviewed**):

> As a rope not clearly seen in the dark is imagined to be a snake, a trickle of water and other
> things, so the Self is imagined to be many things.
>
> When the rope is seen clearly, the imagining stops, and there is only the rope, with nothing
> second to it. So it is when the Self is known for certain.

- **Translator, as shown:** Working translation for Atman from the Sanskrit, awaiting review
- **Edition, as shown:** Sanskrit: Āgamaśāstra, ed. V. G. Āpaṭe, Ānandāśrama Sanskrit Series 10
  (Pune, 1921), as input on GRETIL
- **Note, as shown:** The snake was never there, and the rope never changed. Advaita calls an
  error like this prātibhāsika: an appearance that lasts only until it is seen through. The
  village, its chores and its people are not that kind of error; they are real for living in.
  [C1] [S1]

### 2. Clay and its names (Hume 1921)

**Source:** Chāndogya Upaniṣad 6.1.4 [P1]

**Shown in the game:** when the player first looks through the clay lens, after making three pots.

> यथा सोम्यैकेन मृत्पिण्डेन सर्वं मृन्मयं विज्ञातं स्यात् ।\
> वाचारम्भणं विकारो नामधेयं मृत्तिकेत्येव सत्यम् ॥ ६.१.४ ॥

> *yathā somyaikena mṛtpiṇḍena sarvaṃ mṛnmayaṃ vijñātaṃ syāt*\
> *vācārambhaṇaṃ vikāro nāmadheyaṃ mṛttikety eva satyam*

English, as shown (**published translation, public domain; check the fit of the excerpt only**):

> “Just as, my dear, by one piece of clay everything made of clay may be known—the modification is merely a verbal distinction, a name; the reality is just ‘clay’—”

- **Translator, as shown:** Robert Ernest Hume, The Thirteen Principal Upanishads (Oxford
  University Press, 1921)
- **Edition, as shown:** Sanskrit: Chāndogya Upaniṣad, Sansknet project input, as on GRETIL
- **Note, as shown:** Uddālaka is teaching his son Śvetaketu, who has come home proud of his
  learning. A water jar, a bowl and a lamp are different names and shapes, and each is useful as
  what it is. What they are made of is one thing: clay.

### 3. The space in the pot (working translation)

**Source:** Gauḍapāda, Māṇḍūkya Kārikā 3.3–3.4 [C2]

**Shown in the game:** when the player first breaks a pot on the drying board.

> आत्मा ह्याकाशवज्जीवैर्घटाकाशैरिवोदितः ।\
> घटादिवच्च संघातैर्जातावेतन्निदर्शनम् ॥ ३.३ ॥\
> घटादिषु प्रलीनेषु घटाकाशादयो यथा ।\
> आकाशे संप्रलीयन्ते तद्वज्जीवा इहात्मनि ॥ ३.४ ॥

> *ātmā hy ākāśavaj jīvair ghaṭākāśair ivoditaḥ*\
> *ghaṭādivac ca saṃghātair jātāv etan nidarśanam*\
> *ghaṭādiṣu pralīneṣu ghaṭākāśādayo yathā*\
> *ākāśe saṃpralīyante tadvaj jīvā ihātmani*

English, as shown (**working translation, to be reviewed**):

> The Self is like space. It is spoken of as many individual selves, as space is spoken of as the
> spaces inside pots, and bodies are like the pots. This is the example for being born.
>
> When the pots are broken, the space that was in them is gone into space. In the same way,
> individual selves are gone into the Self.

- **Translator, as shown:** Working translation for Atman from the Sanskrit, awaiting review
- **Edition, as shown:** Sanskrit: Āgamaśāstra, ed. V. G. Āpaṭe, Ānandāśrama Sanskrit Series 10
  (Pune, 1921), as input on GRETIL
- **Note, as shown:** Nothing went anywhere. The lights over the drying board were the same before
  the pot was made and after it broke; only the pot changed. Gauḍapāda carries the example on
  through 3.9.

### 4. The dream (Hume 1921)

**Source:** Bṛhadāraṇyaka Upaniṣad 4.3.9–4.3.10, 4.3.14 [P2]

**Shown in the game:** on waking from the deep sleep, the first night after the rope and the clay (chapter 3, added after this packet was first written).

> स यत्र प्रस्वपित्यस्य लोकस्य सर्वावतो मात्रामपादाय स्वयं विहत्य स्वयं निर्माय\
> स्वेन भासा स्वेन ज्योतिषा प्रस्वपिति । अत्रायं पुरुषः स्वयंज्योतिर्भवति ॥ ४.३.९ ॥\
> न तत्र रथा न रथयोगा न पन्थानो भवन्ति । अथ रथान्रथयोगान्पथः सृजते । … स हि कर्ता ॥ ४.३.१० ॥\
> आराममस्य पश्यन्ति न तं पश्यति कश्चन ॥ ४.३.१४ ॥

> *sa yatra prasvapity asya lokasya sarvāvato mātrām apādāya svayaṃ vihatya svayaṃ nirmāya*\
> *svena bhāsā svena jyotiṣā prasvapiti | atrāyaṃ puruṣaḥ svayaṃjyotir bhavati*\
> *na tatra rathā na rathayogā na panthāno bhavanti | atha rathān rathayogān pathaḥ sṛjate | … sa hi kartā*\
> *ārāmam asya paśyanti na taṃ paśyati kaś cana*

English, as shown (**published translation, public domain; check the fit of the excerpt only**):

> “When one goes to sleep, he takes along the material of this all-containing world, himself tears it apart, himself builds it up, and dreams by his own brightness, by his own light. Then this person becomes self-illuminated.

> “There are no chariots there, no spans, no roads. But he projects from himself chariots, spans, roads. … For he is a creator.”

> “People see his pleasure-ground; Him no one sees at all.”

- **Translator, as shown:** Robert Ernest Hume, The Thirteen Principal Upanishads (Oxford University Press, 1921); his Sanskrit glosses left out
- **Edition, as shown:** Sanskrit: Bṛhadāraṇyaka Upaniṣad, Kāṇva recension, Sansknet project input, as on GRETIL
- **Note, as shown:** Yājñavalkya is answering King Janaka: what is a person's light? In a dream there is no sun and no lamp, yet roads, a river and people are seen. The dreamer makes all of it and is the light it is seen by, and is never one of the things in it. [P2] [C1]
- **Note, as shown:** Gauḍapāda goes further and likens the waking world itself to a dream. Śaṅkara still holds the waking village real for living in (vyāvahārika); the dream is the clearer case of a world that depends on its seer. [C2] [C1]

Hume's Sanskrit glosses in parentheses are left out, and 4.3.10 is cut with an ellipsis, as the page shows; see the [Codex texts table](../RESEARCH.md#codex-texts-2026-10-22-m4).

### 5. The fourth (Hume 1921)

**Source:** Māṇḍūkya Upaniṣad 2 and 7 [P3]

**Shown in the game:** with the dream's page, on the same waking.

> सर्वं ह्येतद्ब्रह्मायमात्मा ब्रह्म सोऽयमात्मा चतुष्पात् ॥ २ ॥\
> नान्तःप्रज्ञं न बहिःप्रज्ञं नोभयतःप्रज्ञं न प्रज्ञानघनं न प्रज्ञं नाप्रज्ञम् ।\
> अदृष्टमव्यवहार्यमग्राह्यमलक्षणमचिन्त्यमव्यपदेश्यम्\
> एकात्मप्रत्ययसारं प्रपञ्चोपशमं शान्तं शिवमद्वैतं चतुर्थं मन्यन्ते । स आत्मा स विज्ञेयः ॥ ७ ॥

> *sarvaṃ hy etad brahmāyam ātmā brahma so 'yam ātmā catuṣpāt*\
> *nāntaḥprajñaṃ na bahiḥprajñaṃ nobhayataḥprajñaṃ na prajñānaghanaṃ na prajñaṃ nāprajñam*\
> *adṛṣṭam avyavahāryam agrāhyam alakṣaṇam acintyam avyapadeśyam*\
> *ekātmapratyayasāraṃ prapañcopaśamaṃ śāntaṃ śivam advaitaṃ caturthaṃ manyante | sa ātmā sa vijñeyaḥ*

English, as shown (**published translation, public domain; check the fit of the excerpt only**):

> “For truly, everything here is Brahma; this self is Brahma. This same self has four fourths.”

> “Not inwardly cognitive, not outwardly cognitive, not both-wise cognitive, not a cognition-mass, not cognitive, not non-cognitive, unseen, with which there can be no dealing, ungraspable, having no distinctive mark, non-thinkable, that cannot be designated, the essence of the assurance of which is the state of being one with the Self, the cessation of development, tranquil, benign, without a second—[such] they think is the fourth. He is the Self. He should be discerned.”

- **Translator, as shown:** Robert Ernest Hume, The Thirteen Principal Upanishads (Oxford University Press, 1921); his Sanskrit glosses left out
- **Edition, as shown:** Sanskrit: Māṇḍūkya Upaniṣad, Sansknet project input, as on GRETIL
- **Note, as shown:** Verses 3 to 5 name the first three fourths: the self waking, dreaming, and in deep sleep, where there is no desire and no dream. The fourth is not a fourth state that comes after them. It is what is there through all three, so the verse can only say what it is not. [P3] [C1]
- **Note, as shown:** Stepping out of the body to watch the day lived (V) is a picture of the witness, the second bird on the tree that eats nothing and watches. A picture is not the knowing; the verses are the teaching. [P6]

Hume's Sanskrit glosses in parentheses are left out, as the page says.

### 6. Other schools: "that you are" (Hume 1921)

**Source:** Chāndogya Upaniṣad 6.8.7 [P1]

**Shown in the game:** open from the start.

> स य एषोऽणिमैतदात्म्यमिदं सर्वम् । तत्सत्यम् । स आत्मा ।\
> तत्त्वमसि श्वेतकेतो ॥ ६.८.७ ॥

> *sa ya eṣo 'ṇimaitad ātmyam idaṃ sarvam / tat satyam / sa ātmā /*\
> *tat tvam asi śvetaketo*

English, as shown (**published translation, public domain; check the fit of the excerpt only**):

> “That which is the finest essence—this whole world has that as its soul. That is Reality. That is Ātman (Soul). That art thou, Śvetaketu.”

The page then gives three readings of the verse, as shown:

| School | Teacher | Text on the page | Sources |
|---|---|---|---|
| Advaita | Śaṅkara | The game follows this reading. You, the self, are Brahman, with no second thing beside it. The difference between them is put there by ignorance, as the snake is put on the rope. | [C1] [S1] |
| Viśiṣṭādvaita | Rāmānuja | “That” and “you” name one reality, Brahman, with different content. Individual selves are real modes of Brahman, which stands to them as a soul stands to its body. There is unity, and the differences within it are real. | [S12] |
| Dvaita | Madhva | The self and Brahman are always distinct: Brahman alone is independent, and selves depend on it. Madhva divides the words differently, carrying the “a” over from the word before, and reads atat tvam asi: “you are not that.” | [S13] |

- **Translator, as shown:** Robert Ernest Hume, The Thirteen Principal Upanishads (Oxford
  University Press, 1921); his Sanskrit gloss left out
- **Edition, as shown:** Sanskrit: Chāndogya Upaniṣad, Sansknet project input, as on GRETIL
- **Note, as shown:** The grammar alone does not settle it: the final “that” is neuter, and “that you are” is already an interpretation. [S11] Atman teaches Advaita. It is one school's reading, not what Hinduism says.

Hume's Sanskrit gloss "(*satya*)" is left out of the page, and the page says so;
see the [Codex texts table](../RESEARCH.md#codex-texts-2026-10-22-m4).

## Word-by-word glosses of the working translations

Every dictionary sense below was checked on 2026-10-23 against Monier-Williams, *A
Sanskrit-English Dictionary* (1899), in the Cologne digital edition
([sanskrit-lexicon.uni-koeln.de](https://www.sanskrit-lexicon.uni-koeln.de/)). The Cologne site
limited the number of lookups, so the entries were read from the same edition's source text, which
the site displays ([csl-orig, `v02/mw/mw.txt`](https://github.com/sanskrit-lexicon/csl-orig)).
Each sense is cited with its headword and the printed page and column ("MW 955,1" is page 955,
column 1); any headword can be looked up on the Cologne site to see the same entry. Only the senses
relevant to the verse are quoted, not the whole entry.

The grammatical parse in the first column is the packet's own and is part of what is to be
reviewed. Where a word is not a headword in MW, that is said, and the parts are glossed instead.

### Kārikā 2.17

| Word | Parse | MW sense (headword, page) | How the translation renders it |
|---|---|---|---|
| *aniścitā* | nom. sg. f. of *a-niścita*, with *rajjuḥ* | "unascertained, not certain" (*a-niścita*, MW 30,2) | "not clearly seen" |
| *yathā* | indeclinable | "in which manner or way, according as, as, like" (*yathā*, MW 841,2) | "As" |
| *rajjuḥ* | nom. sg. | "a rope, cord, string, line" (*rajju*, MW 861,1) | "a rope" |
| *andhakāre* | loc. sg. | "darkness" (*andha-kāra*, MW 44,3) | "in the dark" |
| *vikalpitā* | nom. sg. f., p.p. of caus. *vi-√kḷp* | caus. "to prepare, arrange, contrive, form, fashion (in various ways)"; "to suppose, conjecture, imagine, presume" (*vi-√kḷp*, MW 955,1–2). The headword *vi-kalpita* itself gives "prepared, arranged"; "divided, manifold"; "doubtful, undecided" (MW 955,2), not "imagined". | "is imagined to be" |
| *sarpa-* | first member of the compound | "a snake, serpent" (*sarpa*, MW 1184,1) | "a snake" |
| *-dhārā-* | second member | 1. "stream or current of water"; "flood, gush, jet, drop (of any liquid)"; "continuous line or series" (*dhārā* 1, MW 515,3). A second homonym means "margin, sharp edge, rim, blade" (*dhārā* 2, MW 515,3). | "a trickle of water" |
| *-ādibhiḥ* | instr. pl., end of the compound | ifc. "beginning with, et cætera, and so on" (*ādi*, MW 136,3) | "and other things" |
| *bhāvaiḥ* | instr. pl. | "becoming, being, existing, occurring, appearance"; "that which is or exists, thing or substance, being or living creature" (*bhāva*, MW 754,1–2) | "things" (folded into "other things" and "many things") |
| *tadvat* | indeclinable | ind. "like that, thus, so" (correlative of *yathā*); "in like manner, likewise" (*tad-vat*, MW 434,2–3) | "so" |
| *ātmā* | nom. sg. | "the soul, principle of life and sensation"; "the individual soul, self"; "the highest personal principle of life, Brahma" (*ātman*, MW 135,1) | "the Self" |
| *vikalpitaḥ* | nom. sg. m., as *vikalpitā* above | as *vikalpitā* (*vi-√kḷp*, MW 955,1–2) | "is imagined to be many things" |

### Kārikā 2.18

| Word | Parse | MW sense (headword, page) | How the translation renders it |
|---|---|---|---|
| *niścitāyām* | loc. sg. f., with *rajjvām* (locative absolute) | "ascertained, determined, settled, decided" (*niś-cita*, MW 561,2) | "When ... is seen clearly" |
| *yathā* | indeclinable | as in 2.17 (*yathā*, MW 841,2) | not rendered separately ("When ... So it is") |
| *rajjvām* | loc. sg. of *rajju* | "a rope" (*rajju*, MW 861,1) | "the rope" |
| *vikalpaḥ* | nom. sg. | "false notion, fancy, imagination"; also "difference of perception, distinction"; "variation, ... diversity, manifoldness" (*vi-kalpa* 2, MW 955,2) | "the imagining" |
| *vinivartate* | 3 sg. pres. ātm. of *vi-ni-√vṛt* | "to turn back, return"; "to cease, end, disappear" (*vi-ni-√vṛt*, MW 971,2) | "stops" |
| *rajjuḥ* | nom. sg. | "a rope" (*rajju*, MW 861,1) | "the rope" |
| *eva* | particle | "just, exactly, very, same, only, even, alone, merely" (*eva* 1, MW 232,2) | "only" |
| *iti* | particle | "in this manner, thus"; marks the preceding words as what is said or thought (*iti* 2, MW 165,1) | not rendered as a word; "there is only the rope" stands for the thought "only the rope" |
| *ca* | particle | "and, both, also, moreover" (*ca* 2, MW 380,1) | "and" |
| *advaitam* | nom. sg. n. | n. "non-duality"; mfn. "destitute of duality, having no duplicate"; "sole, unique" (*a-dvaita*, MW 19,3) | "with nothing second to it" |
| *tadvat* | indeclinable | "like that, thus, so" (*tad-vat*, MW 434,2) | "So it is" |
| *ātma-* | first member of the compound | "self" (*ātman*, MW 135,1) | "the Self" |
| *-viniścayaḥ* | nom. sg. | "deciding, settling, ascertainment, settled opinion, decision" (*vi-niścaya*, MW 971,2) | "is known for certain" |

### Kārikā 3.3

| Word | Parse | MW sense (headword, page) | How the translation renders it |
|---|---|---|---|
| *ātmā* | nom. sg. | "self"; "the highest personal principle of life, Brahma" (*ātman*, MW 135,1) | "The Self" |
| *hi* | particle | "for, because"; "indeed, assuredly, surely" (*hi* 2, MW 1297,3) | not rendered |
| *ākāśa-* | stem, with the affix *-vat* | "a free or open space, vacuity"; "the ether, sky or atmosphere" (*ā-kāśa*, MW 126,3–127,1) | "space" |
| *-vat* | affix | "an affix ... added to words to imply likeness or resemblance, and generally translatable by 'as', 'like'" (*vat* 1, MW 915,2). The headword *ākāśa-vat* in MW means "spacious, extensive" (MW 127,1), which is a different formation and does not fit here. | "is like" |
| *jīvaiḥ* | instr. pl. | "the principle of life, vital breath, the living or personal soul (as distinguished from the universal soul)" (*jīva*, MW 422,2) | "many individual selves" |
| *ghaṭākāśaiḥ* | instr. pl. | Not a headword in MW. Its parts: *ghaṭa* "a jar, pitcher, jug, large earthen water-jar" (MW 375,1) and *ā-kāśa* "a free or open space" (MW 126,3) | "the spaces inside pots" |
| *iva* | particle | "like, in the same manner as"; "as it were, as if" (*iva*, MW 168,3) | "as" |
| *uditaḥ* | nom. sg. m., p.p. | Two homonyms: 1. from *ud-√i*, "risen, ascended"; "born, produced"; "apparent, visible" (*ud-ita* 1, MW 186,1); 2. p.p. of *√vad*, "said, spoken"; "communicated, proclaimed, declared" (*udita* 2, MW 186,2) | "is spoken of as" (the second homonym; see the questions) |
| *ghaṭādi-* | compound, with *-vat* | *ghaṭa* "a jar" (MW 375,1) + *ādi* "and so on" (MW 136,3) | "the pots" |
| *-vat* | affix | "as, like" (*vat* 1, MW 915,2) | "are like" |
| *ca* | particle | "and" (*ca* 2, MW 380,1) | "and" |
| *saṃghātaiḥ* | instr. pl. | "close union or combination, collection, cluster, heap, mass"; "any aggregate of matter, body" (*saṃ-ghāta*, MW 1130,1) | "bodies" |
| *jātau* | loc. sg. of *jāti* | "birth, production"; "re-birth" (*jāti*, MW 418,1) | "for being born" |
| *etat* | nom. sg. n. | "this, this here" (*etad*, MW 231,2) | "This" |
| *nidarśanam* | nom. sg. n. | "instance, example, illustration"; "pointing to, showing, indicating" (*ni-darśana*, MW 548,2) | "the example" |

### Kārikā 3.4

| Word | Parse | MW sense (headword, page) | How the translation renders it |
|---|---|---|---|
| *ghaṭādiṣu* | loc. pl., with *pralīneṣu* (locative absolute) | *ghaṭa* "a jar" (MW 375,1) + *ādi* "and so on" (MW 136,3) | "the pots" |
| *pralīneṣu* | loc. pl. of *pralīna* | "dissolved, reabsorbed into (loc.), disappeared, lost, died" (*pralīna*, MW 690,1) | "are broken" |
| *ghaṭākāśādayaḥ* | nom. pl. | *ghaṭa* + *ā-kāśa* + *ādi*, as above (MW 375,1; 126,3; 136,3) | "the space that was in them" |
| *yathā* | indeclinable | "as" (*yathā*, MW 841,2) | "When ... In the same way" (the correlation) |
| *ākāśe* | loc. sg. | "a free or open space"; "the ether, sky" (*ā-kāśa*, MW 126,3–127,1) | "into space" |
| *saṃpralīyante* | 3 pl. pres. ātm. of *sam-pra-√lī* | *sam-pra-√lī* is not a headword in MW. Its parts: *pra-√lī* "to become dissolved or reabsorbed into (loc.), disappear, perish, die" (MW 689,3); *sam* "with, together, altogether", expressing "conjunction", "union", "thoroughness", "completeness" (*sam* 2, MW 1152,1) | "is gone into" / "are gone into" |
| *tadvat* | indeclinable | "like that, thus, so" (*tad-vat*, MW 434,2) | "In the same way" |
| *jīvāḥ* | nom. pl. | "the living or personal soul" (*jīva*, MW 422,2) | "individual selves" |
| *iha* | indeclinable | "in this place, here"; "in this world"; "in this case" (*iha*, MW 169,3) | not rendered |
| *ātmani* | loc. sg. | "self"; "the highest personal principle of life, Brahma" (*ātman*, MW 135,1) | "into the Self" |

### Senses that could not be verified

No sense in the tables above is given from memory. Two forms are not MW headwords, so they are
glossed by their parts and the compound or compound verb as a whole is **not verified** in MW:
*ghaṭākāśa* ("pot-space") and *sam-pra-√lī* ("to merge completely into"). The grammatical parses
are the packet's own and are not dictionary senses.

## Questions for the reviewer

Each question names the page it is about. A short answer is as useful as a long one.

**On the working translations**

1. *uditaḥ* (3.3): the translation reads it as "spoken of" (p.p. of *√vad*). MW also gives *ud-ita*
   from *ud-√i*, "born, produced; apparent, visible", and the verse ends *jātāv etan nidarśanam*,
   "this is the example with regard to birth". Which reading should the game follow, and how does
   Śaṅkara's commentary on this verse take it?
2. *pralīneṣu* (3.4): the translation says "When the pots are broken". MW gives "dissolved,
   reabsorbed into, disappeared, lost", not "broken". In the game the player breaks a pot, so
   "broken" fits the scene. Is "broken" an acceptable rendering, or should the English say
   something like "dissolved" or "gone"?
3. *saṃpralīyante* (3.4): rendered "is gone into" / "are gone into". Is "merge into" or "are
   absorbed into" closer to *sam-pra-√lī*?
4. *iha* (3.4) and *hi* (3.3) are not rendered. Should either be?
5. *dhārā* (2.17): rendered "a trickle of water". MW's first sense is "stream or current of water";
   it also has "continuous line or series", and a second homonym "sharp edge". Which sense do the
   commentators take here, and is "trickle" too small a word?
6. *aniścitā* / *niścitāyām* (2.17–2.18): rendered "not clearly seen" / "seen clearly". MW gives
   "unascertained, not certain" and "ascertained, determined". Is the shift from knowing to seeing
   acceptable, given that the game shows the rope being looked at by lamplight?
7. *vikalpitā* / *vikalpitaḥ* (2.17): rendered "imagined". The sense "imagine" is in MW under the
   causative of *vi-√kḷp*, but MW's entry for *vi-kalpita* itself has "prepared, arranged; divided,
   manifold; doubtful". Is "imagined" right for Gauḍapāda's technical use, or would "conceived" or
   "variously imagined" be better?
8. "so the Self is imagined to be many things" (2.17): the Sanskrit has *tadvad ātmā vikalpitaḥ*,
   "so the Self is imagined"; "many things" is supplied from *bhāvaiḥ* in the first half. Is that a
   fair way to carry the sense across?
9. *rajjur eveti cādvaitam* (2.18): the translation applies *advaitam* to the rope, "only the rope,
   with nothing second to it". Is that the right construal, or does *advaitam* look forward to the
   Self (non-duality as what is ascertained)?
10. *jīvaiḥ ... uditaḥ* (3.3): "It is spoken of as many individual selves". Is "individual selves"
    a good English for *jīva* in a game for general players, or should the page use the Sanskrit
    word with a gloss?

**On the published translation (Hume)**

11. Chāndogya 6.1.4 is quoted only up to "the reality is just 'clay'" and ends on a dash. Is the cut
    in a sensible place?
12. Chāndogya 6.8.7 keeps Hume's "Ātman (Soul)" and "That art thou", while the page's title says
    "that you are". Is the mixed register a problem?

**On the notes and the "Other schools" page**

13. Does the "Other schools" page state the Viśiṣṭādvaita reading (Rāmānuja) and the Dvaita reading
    (Madhva) fairly? Would a teacher of either school accept those two sentences as a summary? They
    rest on [S12] and [S13], both encyclopedia articles.
14. Is the Advaita reading on that page ("You, the self, are Brahman, with no second thing beside
    it. The difference between them is put there by ignorance ...") a fair one-line summary of
    Śaṅkara?
15. Does any note overstate Advaita as "what Hinduism says"? RESEARCH.md §1, point 3, asks the game
    not to. The "Other schools" note says plainly that it is one school's reading; the other notes
    say "Advaita calls ..." or describe the scene. Is there any sentence that reads as speaking for
    Hinduism as a whole?
16. The rope-and-snake note says "Advaita calls an error like this prātibhāsika" and cites
    Śaṅkara's Brahmasūtra Bhāṣya [C1] and Deutsch [S1]. Is the three-level terminology
    (*pāramārthika*, *vyāvahārika*, *prātibhāsika*) Śaṅkara's own, or from later Advaita? Should the
    citation say which?
17. The pot-space note says "Nothing went anywhere." Is that a fair plain-English summary of 3.3–3.4,
    or does it say more than Gauḍapāda does?
18. The clay note says Śvetaketu "has come home proud of his learning". Is that a fair summary of
    the opening of Chāndogya 6.1?
19. The dream note says "Gauḍapāda goes further and likens the waking world itself to a dream" and
    that Śaṅkara "still holds the waking village real for living in (vyāvahārika)". Is that a fair
    way to put the difference, or does it set Gauḍapāda and Śaṅkara further apart than they are?
20. The fourth's second note calls stepping out of the body to watch (V) "a picture of the witness",
    citing the two birds of Muṇḍaka 3.1.1 [P6]. Is the witness (*sākṣin*) the right word for what
    Māṇḍūkya 7 calls the fourth, or does the page blur two ideas?
21. Waking from the deep sleep the game says: "Waking, dream and deep sleep: you were there for all
    three." Is "you" acceptable there, or does it suggest the individual (*jīva*) rather than the
    Self?

## How to send feedback

Open an issue on the public repository,
[Preponderous-Software/atman](https://github.com/Preponderous-Software/atman/issues/new), with a
title such as "Review: codex translations". If a GitHub account is not convenient, the same text
can be sent to the maintainer by any other means and filed as an issue from there.

Copy this block into the issue and fill in what applies. Leave out anything you have no view on.

```
## Reviewer
Name (or "anonymous"):
Background (for example: Indologist, or teacher in which lineage):
May your name be credited in the game? yes / no

## Pages
For each page: OK as is / OK with changes / not OK, and why.

- The rope and the snake (Kārikā 2.17–2.18, working translation):
- Clay and its names (Chāndogya 6.1.4, Hume):
- The space in the pot (Kārikā 3.3–3.4, working translation):
- Other schools (Chāndogya 6.8.7, Hume, and the three readings):

## Suggested wording
Page, the words now shown, and the words you suggest:

## Answers to the questions
Question number and answer:

## Anything else
```

## Hand checks

Some things can only be checked by a person with real hardware. The playtest notes for 0.1.0,
0.1.1 and 0.1.2 ([docs/playtests/](playtests/)) list them as "Not yet checked by hand". They need
no knowledge of Sanskrit. The game can be played in a browser at
[atman.play.danielstephenson.dev](https://atman.play.danielstephenson.dev), on a computer or a
phone, or downloaded for Windows from the
[releases page](https://github.com/Preponderous-Software/atman/releases).

| Check | Where | What to look or listen for |
|---|---|---|
| The sound, by ear | The Windows download, on its own speakers or headphones | Every sound is synthesized as the game plays. Listen for the river, the wind, birds and a koel by day, crickets at night, footsteps (different on earth and on the ghat's stone), the potter's wheel, a heartbeat while the fear meter is up near a snake at dusk, a bell for each new page of the codex, and the tanpura drone on the after-mode's first morning. Report any crackle, clipping, a sound that is far too loud or too quiet beside the others, or one that never plays. Check that M turns all of it off and on again. |
| The sound, by ear | A phone's browser | The valley is silent until the first tap, because browsers allow sound only after one. After that, listen for the same sounds. Check that the **Sound** button turns it off and on and says which, and that after reloading the page the choice is remembered. |
| The night's lighting | The Windows download on a real graphics card (it has only been seen on a software renderer) | Play into the night. Press F: the lamp should be held at the shoulder and light about 16 m around you. After sunset every house should have a lamp lit by its door, and a faint light from the sky should keep the shapes of houses, trees and the path visible. Report a night that is too dark to walk in, one so bright it no longer looks like night, or flickering or blocky shadows. While the lamp is out after dark, the prompt should say "F  light your lamp". |
| The codex, held upright | A real phone, held upright, in its browser | Tap **Codex** along the top. The list of pages should sit above the page, the page should span the width of the screen, and the arrow buttons and **Back to the valley** should be below it. No key names ("Up/Down", "Tab") should be drawn on the page. Check that the Devanagari and IAST can be read at that size. |
| The touch pad | A real phone, in its browser | Drag on the left half to walk (all the way out to hurry) and on the right half to look. The buttons should be **Use**, **Lamp** and **Break**, with **Lens** appearing only after three pots and **Watch** only in the after-mode. There should be no **Keys** button any more. The **?** button along the top should open How to play, listing the touch controls. Report a button that does nothing or covers something that has to be seen. |

Hand-check results can be sent the same way, as an issue, saying the device, the operating system
and the browser.
