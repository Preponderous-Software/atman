# Save fixtures

Real saves written by released builds of Atman. `tests/test_saves.cpp` loads each one by name, so a
change that stops a player's existing save from loading fails the tests. A fixture is never edited
or regenerated once it is committed: a new save format gets new fixtures of its own.

| File | Format | Written by | State |
|---|---|---|---|
| `v1-three-pots.save` | 1 | the 0.1.2 rules (#21) | the first afternoon, three pots thrown |
| `v1-after-mode.save` | 1 | the 0.1.2 rules (#21) | the end of the 0.1.x playthrough, in the after-mode, before chapter 3 existed |
| `v2-mid-dream.save` | 2 | 0.2.0, tag `0.2.0` (`1100130c44eb2a440ce9530d222623440c6b5b97`) | tick 4790: halfway through the first night's dream (fingerprint `a7df0f9b07b2a198`) |
| `v2-after-the-fourth.save` | 2 | 0.2.0, tag `0.2.0` (`1100130c44eb2a440ce9530d222623440c6b5b97`) | tick 43200, day 1: the after-mode, ten game minutes after the fourth's codex page opened (fingerprint `ebf51994201ebeb9`) |

## How the format-2 fixtures were made

[`tools/make_save_fixtures.cpp`](../../tools/make_save_fixtures.cpp) plays the autopilot on seed 1
and saves through the game's own `writeSave`, the call the game makes on F5 and on quit. It was
built against the checkout of the `0.2.0` tag and run on 2026-10-24 (Ubuntu 20.04, GCC 9.4):

```bash
git checkout 0.2.0
g++ -std=c++17 -O2 -ffp-contract=off -Isrc tools/make_save_fixtures.cpp \
    src/sim/*.cpp src/save/*.cpp -o make_save_fixtures
./make_save_fixtures tests/fixtures
```

The tool is newer than the tag; it uses only `newWorld`, `tick`, `Autopilot` and `writeSave`, which
0.2.0 has. Built the same way at `main` (`dae6fe4`), it writes byte-identical files.

SHA-256 of the files as written:

```
46dcb26516c157e7acb1531d9e3a225013968ad198ca70b9e25a23d937fde5c1  v2-mid-dream.save
c0e9efed84a2dd0a756f76fbefa264f8580b156acd71d6ec73a2f6a1f913fd55  v2-after-the-fourth.save
```
