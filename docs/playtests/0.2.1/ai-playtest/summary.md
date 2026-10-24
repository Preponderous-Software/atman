# Playtest summary: 20261024-0932

2 sessions, build atman-strata None (dae6fe4), generated 2026-10-24 10:14.

**Findings:** 16 reports in 15 groups -- 0 blocker, 2 major, 11 minor, 3 polish.

## Outcomes

| Session | Goal | Outcome | Fun | Minutes | Objectives | Deaths | Findings | Verdict |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| s01 | The rope: do what the NEXT line asks until dusk, then walk the path from the village to the ghat, light the lamp (F) and look steadily at each snake until it turns out to be a rope; then sleep at home (E at the indigo door) and, in the morning, try V. Answer the questions at the end. | partial | 3 | 28.9 | 0 | 0 | 9 | The pots-to-clay and snake-to-rope beats land well: the clay lens relabelling each named pot as 'clay', the fear bar falling as the coil resolves into rope, and a carefully sourced, even-handed codex. But sleeping at the door runs the clock backwards on the same day instead of reaching morning, and V is refused with no pointer to what teaching is left, so the goal's last step could not be done. secondsToObjective is wall-clock (no probe), inflated by a slow shared machine. |
| s02 | As a brand-new player, follow the NEXT line from the first afternoon (the potter's wheel, then dusk and the path from the ghat back to the village) until you sleep at home after both teachings. Describe exactly what happens after you sleep, what you took it to mean, and whether anything made you think the game had stopped or frozen; then read the codex pages that open when you wake, and answer the questions at the end. | achieved | 3 | 40.4 | 0 | 0 | 7 | The arc works and the codex is excellent: Sanskrit, transliteration, a cited translation and a short gloss that keeps Advaita's distinctions (pratibhasika vs vyavaharika, and the 'Other schools' page) honest. The biggest risk is the sleep: the backwards-running afternoon is a lovely idea, but nothing tells the player they are dreaming, so it reads as a frozen game or a clock bug until the 'The dream' page explains it after waking. secondsToObjective are wall-clock seconds (no probe), inflated by a slow shared software renderer. |

## Answers

Each agent's own words, verbatim from its report; a missing answer is listed under problems.

### Q1. In your own words, what did you think the game was about? What did the rope and the snake, and the clay and the pots, mean?

- **s01:** An ordinary afternoon used to show that the many things we name are one underlying reality seen under different names and fears. The pots: I made a water jar, a bowl and a cup, gave them names, and through the clay lens each was just 'clay'; breaking one, the codex said the space in the pot was never separate from space, so the pot is a name and shape on clay, and individual selves are like the space in pots. The snake and rope: at dusk the fear bar rose at each coiled snake, and looking at it steadily with the lamp showed it was a rope all along. Fear came from not seeing clearly, not from anything there; light and attention remove the mistake rather than changing the rope. Together they suggest the self and the world are misread on top of one reality (Brahman/Atman). I could not tell what the after-mode (V) adds, as it stayed locked.
- **s02:** An ordinary village day used as a lesson in seeing what is really there. The pots I named (rice, river, chai) were all 'clay' through the lens: the names and shapes are real enough to use, but the stuff is one thing, and breaking one doesn't destroy anything (the 'space in the pot' page: the space just rejoins space). The snake on the dusk path was fear produced by not seeing clearly; holding the lamp steady showed a rope that was always a rope, and the fear meter fell. Together they point at the Self/Brahman as the one reality under many names and appearances. The sleep then turned this on experience itself: the dream (an afternoon replayed backwards that 'seemed not strange') is something the dreamer makes, and waking, 'Nothing more is asked of you' plus 'The fourth' suggested the witness that is there through waking, dream and deep sleep.

### Q2. Did anything feel preachy, confusing, or inaccurate (in the teaching, the verses in the codex, or how the tradition is described)? Say what and where; if nothing did, say so.

- **s01:** Nothing felt preachy; the game shows rather than lectures and the codex quotes the sources (Chandogya 6.1.4 and 6.8.7, Gaudapada's Karika 3.3-3.4) with Sanskrit, transliteration and cited translations. The 'Other schools' page is fair to Vishishtadvaita and Dvaita and openly says the game takes the Advaita reading and that it is one school's reading, not what Hinduism says; I cannot judge its accuracy myself. Confusing: that page mentions the snake and the rope before the dusk scene (a spoiler), one Gaudapada page is labelled a 'working translation ... awaiting review', and V's message 'once the teachings are done' does not say which teachings remain.
- **s02:** Not preachy: the game never lectured in the world; the teaching sits in the optional codex, with verse, transliteration, a cited translation and a short gloss, and it was careful (the rope page says the village and chores are not that kind of error, 'they are real for living in'; 'Other schools' gives Visistadvaita and Dvaita readings of tat tvam asi fairly and says Atman takes one reading). Confusing: the dream. While in it nothing told me I was dreaming, so the backwards clock looked like a bug; only the 'The dream' page after waking explained it. 'Nothing more is asked of you' on waking was also a little abrupt; I wasn't sure if the game had ended. The Mandukya 'The fourth' translation (Hume) is dense and hard to read at 640x360. I can't judge Sanskrit accuracy; the Gaudapada translations are marked 'working translation, awaiting review'.

## Findings

### 1. [major] After sleep, the dream (clock running backwards) has no dream cue; reads as a frozen game or clock bug

- **Category:** ux; **reported by** 1 agent(s): s02
- **Detail:** Pressed E 'sleep until morning' at my door at 7:12 pm on 21 March. The screen went black for a moment, then faded back in to the same spot, in daylight, lamp out, NEXT 'Nothing here seems strange', and the clock ran BACKWARDS at about 6 game-minutes per second: 6:08, 6:02, 5:50, 5:26, 4:56, 2:56, 1:49 pm. Hunger/thirst/tiredness stayed fixed. I could walk, but nothing on screen said I was asleep or dreaming, how long it lasts, or what to do. On this loaded machine it lasted ~6 minutes of wall clock (about 95 game-seconds); I seriously considered it a bug (clock underflow) or a soft-lock. Then it was suddenly 22 March 6:46 am with 'Nothing more is asked of you'. Only afterwards, reading the new codex page 'The dream' (Brhadaranyaka 4.3.9-14: 'there are no chariots... he projects them'), did it click that the backwards afternoon was the dream and the NEXT line was a dream-logic joke. Expected: some small in-world signal (a caption like 'You are dreaming', a tint, a 'The dream' codex toast at the start rather than only on waking), and ideally the deep-sleep part made visible, so the player is not left wondering if the game hung.
- **Repro:** pt start --profile atman-strata --game-dir /root/atman-pt --seed 4; finish pots + snake/rope path; walk right up against the indigo door; pt key e; pt step 10; pt shot a; pt step 300; pt shot b; pt step 1200; pt shot c  (clock decreasing, NEXT 'Nothing here seems strange'); pt step 3600; pt shot d (22 March 6:46 am)
- **Screenshots:** [sleep_rewind.png](s02/shots/sleep_rewind.png)

### 2. [major] Sleeping at the indigo door runs the clock backwards on the same day instead of advancing to morning

- **Category:** bug; **reported by** 1 agent(s): s01
- **Detail:** At 7:00 pm on 21 March the door offered 'E  sleep until morning'. After E the screen went dark and the HUD read '21 March 6:03 pm', then 5:51 (2 s later), 5:21 (5 s), 5:17, and 3:17 pm after 20 more game seconds: the clock runs backwards at about 6 game-minutes per second while the sky brightens back to afternoon. The date never changes, tiredness stays at 5/10, the lamp is out and NEXT says 'Nothing here seems strange'. Expected: the next morning (22 March, early), tiredness reset. If the rewind is a deliberate 'night passing' effect, it reads as a broken clock, and it is far too slow to wait out in a driven session.
- **Repro:** pt start --profile atman-strata --game-dir /root/atman-pt --seed 3; follow NEXT (3 pots, break one, wait at the ghat to dusk, turn 3 snakes to rope with F + gaze); walk to the indigo door until 'E sleep until morning' shows; pt key e; pt step 60; pt shot a; pt wait 2; pt shot b; pt wait 20; pt shot c
- **Screenshots:** [64-clock-back.png](s01/shots/64-clock-back.png)

### 3. [minor] Game time is very slow on the shared machine: 30 s of game wait took over 2 minutes of wall clock

- **Category:** perf; **reported by** 1 agent(s): s01
- **Detail:** Waiting for sunset on the ghat (pt wait 30) exceeded a 120 s shell timeout; pt wait 20 after sleeping took ~3 min. CPU ~190%. Combined with the backwards clock this made reaching morning impossible within 45 minutes. Walking speed is also slow: 4 s of W covered about 5 m.
- **Repro:** pt wait 30 on the ghat steps; time it
- **Screenshots:** [36.png](s01/shots/36.png)

### 4. [minor] Ghat sunset wait is long with no indication of how long

- **Category:** ux; **reported by** 1 agent(s): s02
- **Detail:** 'Wait on the ghat steps for the sunset' took from 5:18 pm to about 6:20 pm (roughly 65 game-seconds) with nothing to do and no progress cue; on a loaded machine that was ~8 minutes of wall clock. Expected a progress hint or a shorter wait.
- **Repro:** after breaking a pot, walk to the ghat steps; pt wait 20; pt wait 30; pt wait 15
- **Screenshots:** [sunset_wait.png](s02/shots/sunset_wait.png)

### 5. [minor] HUD texts overlap: start caption over help block; wheel prompt wraps into the MADE list

- **Category:** text; **reported by** 1 agent(s): s02
- **Detail:** At start, the orange caption 'The NEXT line, top left, says what comes next' is drawn on top of the white help block (W A S D walk / Shift hurry), both unreadable. At the wheel with the clay lens on, the right-aligned 'At the wheel: 1 water jar 2 bowl 3 cup 4 lamp' wraps onto the 'MADE (through the clay lens)' line and the codex toast sits over the pot list. The HUD in general covers the centre-left of a 640x360 view.
- **Repro:** pt start ...; pt shot start. At the wheel: throw 3 pots, pt key c, turn toward the bench; pt shot
- **Screenshots:** [overlap_help.png](s02/shots/overlap_help.png)

### 6. [minor] Indigo door not discoverable at night; white HUD text unreadable on the lamp-lit wall

- **Category:** ux; **reported by** 1 agent(s): s01
- **Detail:** Following 'Go home and sleep (E at your door) 5 m ahead' led me to a blank wall; E did nothing and no prompt appeared. The door was on another face; I had to strafe around. While standing at the lit wall the white HUD text on bright yellow was nearly unreadable (no outline or backing).
- **Repro:** after the third rope: pt hold Shift_L+w 5; pt hold w 3; pt shot wall; pt key e
- **Screenshots:** [56-wall.png](s01/shots/56-wall.png)

### 7. [minor] Opaque HUD text block covers the centre-left of the view, hiding the snakes and the pots you must look at

- **Category:** ux; **reported by** 1 agent(s): s01
- **Detail:** The NEXT / CHORES / MADE block plus the orange hints fill the left half from y=120 to y=290. The first two snakes and the drying plank with my pots were drawn underneath it; I had to turn until they slid out from behind the text. F1 hides the help but not this block (H hides the HUD per the help, but then you lose NEXT).
- **Repro:** dusk path: pt shot after the 'Hold the lamp to it' NEXT appears; or after throwing pots, look for them on the plank
- **Screenshots:** [44-rope1.png](s01/shots/44-rope1.png)

### 8. [minor] Sleep prompt needs you pressed against the door; distance hint vanishes ~2 m earlier and E does nothing

- **Category:** ux; **reported by** 1 agent(s): s02
- **Detail:** The NEXT distance disappears when about 2 m from the house (implying 'you are there'), but pressing E there did nothing, with no prompt. Only after walking right into the indigo door did 'E sleep until morning' appear. Similarly, pots at the wheel were off-screen to the right and lower than the wheel; 'Break one of your pots (B)' needed finding them on a bench by turning around.
- **Repro:** pt hold w until NEXT distance vanishes at the house; pt key e (nothing); pt hold w 1; prompt appears
- **Screenshots:** [door_range.png](s02/shots/door_range.png)

### 9. [minor] Snake gaze has no feedback on range or aim; a near-miss from 4 m does nothing while fear climbs

- **Category:** ux; **reported by** 1 agent(s): s01
- **Detail:** Snake 2: about 6 s of looking from ~4 m with the snake ~40 px above the crosshair did nothing and fear rose to 9/10. After stepping to ~2 m and centring it on the crosshair it became a rope in 3 s. Nothing says how close or how centred you must be, and there is no progress cue (e.g. the snake slowly lightening) while holding the gaze.
- **Repro:** at dusk with the lamp lit, pt look so the second snake is just above the crosshair from ~4 m; pt wait 3 twice; then pt hold w 1.2; centre it; pt wait 3
- **Screenshots:** [51-rope2.png](s01/shots/51-rope2.png)

### 10. [minor] Snake path: after the first rope, NEXT still says 'Hold the lamp to it' with no count of snakes left

- **Category:** ux; **reported by** 1 agent(s): s02
- **Detail:** Holding the lit lamp on the coiled snake turned it into a rope and a codex toast appeared, but the NEXT line kept saying 'Hold the lamp to it and look at it steadily - 7 m ahead'. There turned out to be three snakes along the path; the second gave no codex toast. A count ('1 of 3') like the pots have would make it clear. Also pitch from mouse-look is low-sensitivity, so centring the snake took several tries while Fear climbed to 9/10.
- **Repro:** after sunset at the ghat: pt key f; turn to the path; walk; pt look onto the snake and pt wait 4; pt shot
- **Screenshots:** [rope_next_stale.png](s02/shots/rope_next_stale.png)

### 11. [minor] Start screen: orange intro hint overprints the help's first line

- **Category:** text; **reported by** 1 agent(s): s01
- **Detail:** 'The NEXT line, top left, says what comes next.' is drawn on top of 'W A S D walk  Shift hurry  Mouse look  E use'; both are unreadable where they overlap. Hiding help with F1 fixes it.
- **Repro:** pt start ...; pt shot start
- **Screenshots:** [02-overlap.png](s01/shots/02-overlap.png)

### 12. [minor] Thrown pots appear 4 m behind the wheel, never in view while throwing; hard to find for B

- **Category:** ux; **reported by** 1 agent(s): s01
- **Detail:** Throwing three pots at the wheel shows only a progress bar and a name prompt; the pots never appear in front of you. NEXT then says 'Break one of your pots (B) 4 m behind you, to the right', and once close the distance disappears. 'Look straight at one of your pots, close by' repeated until I did a full 360 panorama and found them on a low plank. The clay lens also turns the scene nearly black, which made finding them harder.
- **Repro:** after 3 pots: pt key c; walk toward 'behind you, to the right'; pt key b -> 'Look straight at one of your pots'; pt look 250 0 repeatedly to find the plank
- **Screenshots:** [24-pots-hidden.png](s01/shots/24-pots-hidden.png)

### 13. [minor] V after sleeping is refused: 'Stepping out of the body to watch comes later, once the teachings are done'

- **Category:** ux; **reported by** 1 agent(s): s01
- **Detail:** After sleeping (no real morning, see above) V shows 'Stepping out of the body to watch comes later, once the teachings are done.' Nothing tells me which teachings remain: NEXT says 'Nothing here seems strange', the two CHORES (water from the ghat to the potter, clay from the bank to the wheel) were never asked for by NEXT and remain unchecked. A player has no pointer to what is left.
- **Repro:** as above, then pt key v; pt step 30; pt shot v
- **Screenshots:** [65.png](s01/shots/65.png)

### 14. [polish] Codex sidebar clips 'The space in the pot'; 'Other schools' page explains the snake/rope before dusk

- **Category:** text; **reported by** 2 agent(s): s01, s02
- **Also reported as:** "Codex page list clips titles ('The rope and the sna', 'The space in the pot')"
- **Detail:** The page list cuts the title at the parchment edge. The 'Other schools: that you are' page (unlocked after breaking a pot) already says 'as the snake is put on the rope', spoiling the dusk scene.
- **Repro:** after breaking a pot: pt key Tab; pt key Down
- **Screenshots:** [29-codex-clip.png](s01/shots/29-codex-clip.png) [codex_trunc.png](s02/shots/codex_trunc.png)

### 15. [polish] V 'step back and watch' changes only the label; the view looks identical

- **Category:** ux; **reported by** 1 agent(s): s02
- **Detail:** In the after-mode V toggled 'Living - step back and watch (V)' to 'Watching - step back in (V)', but standing against my house the camera view was unchanged, so nothing visible showed the difference.
- **Repro:** after waking on 22 March: pt key v; pt step 60; pt shot
- **Screenshots:** [watching_same.png](s02/shots/watching_same.png)

## Objective timings

Game seconds from the start of the world to each objective (event tick / the game's ticks per second, from events.jsonl). Game time, not wall time: with many sessions on one machine a world can run slower than real time, and wall-clock timings would mostly measure the machine.

No objective was completed (or the game has no probe).

