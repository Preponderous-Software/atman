# Atman in a browser

Atman has a browser build at **https://atman.play.danielstephenson.dev**, one of the games on
[danielstephenson.dev/play](https://danielstephenson.dev/play). It is the same game as the desktop
build: the same rules, scene, HUD, codex and session code, drawn by Strata's WebGL 2 backend and
compiled with Emscripten.

## What differs from the desktop game

| | Desktop (`ATMAN_WITH_STRATA`) | Browser (`ATMAN_WEB`) |
|---|---|---|
| Rules, scene, HUD, codex, session (`src/sim`, `src/bridge`, `src/codex`, `src/play`) | the same code | the same code. CI runs the whole test suite as wasm under Node, the pinned determinism and playthrough fingerprints included |
| Entry point | `app/main.cpp` (its own frame loop) | `app/web_main.cpp` (the browser runs the frame loop) |
| Mouse | captured by the window; Esc frees it, Esc again quits | the page takes the pointer lock on a click; Esc gives it back. There is no quit |
| Tab and F1 | the codex and the controls | the same: the page keeps them from the browser |
| Touch | none | a stick (drag on the left half), look (drag on the right half), Use / Lamp / Break / Lens buttons, the four forms at the wheel, page buttons in the codex, and a text box for a pot's name, so a phone's keyboard comes up |
| HUD | drawn on the canvas | drawn on the canvas on a wide page; below 760 CSS pixels wide the page shows the HUD, the crosshair and the prompt itself |
| Shadows | 4 cascades of 2048² | 3 cascades of 1024², for phones |
| Resolution | the window's | the page's size times the device pixel ratio, capped at 2 |
| Saves | `atman.save` in the slot directory (`%APPDATA%\Atman\default`, `~/.local/share/atman/default`, or `--slot`) | the same format and code (`src/save`), in `/atman-save/default`, which the page keeps in IndexedDB. Saves stay in that browser on that device |
| When it saves | every two minutes, F5, and on quit | every two minutes, F5 or the Save button, and when the page is hidden or closed |

### Saves

The browser build writes the desktop save format with the desktop code: the checksum is checked
before a byte of a save is read, a new save is written to a temporary file and moved into place,
and the save it replaces is kept as the previous save, loaded instead if the current one is
damaged. The page mounts `/atman-save` on IndexedDB (Emscripten's IDBFS), reads it before the game
starts, and flushes it to IndexedDB after every save. If the browser refuses to store it, the page
says "Could not store the save in this browser".

Saves belong to the page's origin, so they stay with `atman.play.danielstephenson.dev` in that
browser. Clearing the site's data deletes them. **New** on the page starts again at 4:30 on the
first afternoon after asking; the current day is replaced the next time the game saves (and is then
kept as the previous save until the save after that).

## Controls

On a keyboard and mouse, the desktop controls (see the [README](../README.md)), except that Esc
only frees the mouse. On a touch screen:

- drag on the left half to walk (all the way out to hurry), and on the right half to look;
- **Use** does what is here (fill, give, dig, drink, eat, sleep); **Lamp**, **Break** and **Lens**
  do what they say, and **Keys** shows the keyboard controls on the canvas;
- at the wheel, tap a form to throw it, then type its name in the box;
- **Codex** along the top opens the codex (the arrows choose a page), and in the after-mode
  **Watch** steps out of the body and back in;
- **Save**, **New** and **?** (how to play) are along the top as well.

## Building

Needs emsdk (CI pins 6.0.11, as Strata does) and CMake 3.24 or newer:

```sh
emcmake cmake -S . -B build-web -DATMAN_WITH_STRATA=OFF -DATMAN_WEB=ON
cmake --build build-web -j8
ctest --test-dir build-web --output-on-failure   # the test suite, as wasm under Node
cd build-web && python3 -m http.server 8000      # open http://localhost:8000/atman.html
```

`-DFETCHCONTENT_SOURCE_DIR_STRATA=<checkout>` builds against a local Strata checkout instead of the
pinned commit. The page is `atman.html` + `atman.js` + `atman.wasm` (about 1.8 MB, the codex plates
included); the page shell, with the touch controls, is `web/shell.html`.

## CI and deployment

`.github/workflows/browser.yml` builds the page, runs the tests under Node, and smoke-tests the
page in headless Chromium (SwiftShader) with `web/smoke/smoke.cjs`, on a desktop viewport and on
the iPhone 13 profile. The smoke test checks that the page starts without errors and shows How to
play on a first visit, that the canvas draws the valley, that walking moves the player (W, or the
touch stick) and looking turns them, that the lamp lights and the codex opens and closes (Tab, or
the page's buttons), and that the pointer lock is taken and given back (desktop). On the desktop
viewport it also walks to the potter's wheel, throws a bowl and types its name, which must land in
the list of things made. On both it saves, reloads the page, and checks that the place, the time of
day and the pots came back with a "Loaded your save" message. It uploads its screenshots.

Headless Chromium fakes mouse movement under the pointer lock (it jumps to the cursor and back),
so the smoke test looks through the same call the page's mouse handler makes; how the mouse feels
is checked by hand in a real browser.

When the repository variable `ARCADE_ENABLED` is `true`, a push to `main` deploys the page to
arcade under the slug `atman` with `Stephenson-Software/arcade-deploy`, using the `ARCADE_TOKEN`
secret. A manual run of the workflow deploys too.
