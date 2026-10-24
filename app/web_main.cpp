// Atman in a browser: the same game on Strata's WebGL 2 backend (Emscripten).
//
// The same rules, scene, HUD, codex and session (src/play) as the desktop
// game, with these differences:
//
//  - The page owns the pointer lock (a click on the canvas takes it, Esc
//    gives it back) and turns mouse movement into atman_look(); there is no
//    quit.
//  - On a touch screen the page draws a stick, a look pad and buttons, which
//    call the atman_* functions exported below. A pot is named in the page's
//    own text box, so a phone's keyboard comes up (atman_name()).
//  - On a narrow page the canvas shows no words: the page shows the HUD, the
//    crosshair and the prompt as text (atman_status()).
//  - Saves are the desktop format (src/save: checksummed, previous save kept),
//    written to /atman-save/default, which the page mounts on IndexedDB
//    (IDBFS) and loads before main() runs. Every save is flushed to IndexedDB
//    at once (Module.flushSaves in web/shell.html). The browser's saves stay
//    in that browser: they are never sent anywhere.
#include <cinttypes>
#include <cstdio>
#include <memory>
#include <string>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include "codex/codex.h"
#include "play/session.h"
#include "play/view.h"
#include "sim/chores.h"
#include "sim/clay.h"
#include "strata/strata.hpp"

using namespace atman;

namespace {

struct Game {
    std::unique_ptr<strata::Engine> engine;
    std::unique_ptr<play::Session> session;
    double last = 0;
    std::string statusJson;
};

constexpr const char* kSlot = "/atman-save/default";

Game* g = nullptr;

void frame(void*) {
    StrataEvent e;
    while (g->engine->pollEvent(e)) g->session->handle(e);
    const double now = emscripten_get_now() / 1000.0;
    g->session->frame(now - g->last);
    g->last = now;
    g->engine->present();
}

void jsonString(std::string& out, const std::string& s) {
    out += '"';
    for (char ch : s) {
        if (ch == '"' || ch == '\\') {
            out += '\\';
            out += ch;
        } else if (static_cast<unsigned char>(ch) < 0x20) {
            out += ' ';
        } else {
            out += ch;
        }
    }
    out += '"';
}

void jsonBool(std::string& out, const char* key, bool v) {
    out += ",\"";
    out += key;
    out += v ? "\":true" : "\":false";
}

}  // namespace

// ---- Called by the page (web/shell.html) ------------------------------------
extern "C" {

EMSCRIPTEN_KEEPALIVE void atman_look(double dx, double dy) {
    if (g) g->session->look(static_cast<float>(dx), static_cast<float>(dy));
}
EMSCRIPTEN_KEEPALIVE void atman_stick(double forward, double strafe) {
    if (g) g->session->setStick(static_cast<float>(forward), static_cast<float>(strafe));
}
// A key press from an on-screen button (an SDL scancode, as the keyboard sends).
EMSCRIPTEN_KEEPALIVE void atman_press(int scancode) {
    if (g) g->session->press(scancode);
}
// The name typed in the page's box for the pot waiting for one.
EMSCRIPTEN_KEEPALIVE void atman_name(const char* text) {
    if (g && text) g->session->name(text);
}
EMSCRIPTEN_KEEPALIVE void atman_captured(int on) {
    if (g) g->session->setCaptured(on != 0);
}
// Saves now (the page calls this when it is hidden); 1 on success.
EMSCRIPTEN_KEEPALIVE int atman_save() { return g && g->session->save("Saved.") ? 1 : 0; }
EMSCRIPTEN_KEEPALIVE void atman_restart() {
    if (g) g->session->restart();
}
// Sound on or off, from the page's Sound button (or its remembered choice,
// quietly, when the page opens).
EMSCRIPTEN_KEEPALIVE void atman_sound(int on, int announce) {
    if (g) g->session->setMuted(on == 0, announce != 0);
}
// Where the page's own HUD covers the canvas, so the names of places keep clear of it.
EMSCRIPTEN_KEEPALIVE void atman_cover(double right, double bottom) {
    if (g) g->session->setCover(static_cast<float>(right), static_cast<float>(bottom));
}
// A touch screen: the codex leaves its keys unnamed, as the page has buttons for them.
EMSCRIPTEN_KEEPALIVE void atman_touch(int on) {
    if (g) g->session->setKeyHints(on == 0);
}
// The page's text size (0 to 2) and captions, from How to play's settings.
EMSCRIPTEN_KEEPALIVE void atman_text(int size) {
    if (g) g->session->setTextSize(size);
}
EMSCRIPTEN_KEEPALIVE void atman_captions(int on) {
    if (g) g->session->setCaptions(on != 0);
}
EMSCRIPTEN_KEEPALIVE void atman_help(int on) {
    if (g) g->session->setHelp(on != 0);
}
// The canvas changed size: draw at w x h (the page caps the device pixel
// ratio), with the HUD's words on the page instead when `compact`.
EMSCRIPTEN_KEEPALIVE void atman_resize(double w, double h, int compact) {
    if (!g || w < 1 || h < 1) return;
    g->engine->resize(static_cast<int32_t>(w), static_cast<int32_t>(h));
    g->session->setCompactHud(compact != 0);
}
// The game's state for the page, as JSON.
EMSCRIPTEN_KEEPALIVE const char* atman_status() {
    if (!g) return "{}";
    const play::Session& s = *g->session;
    const World& w = s.world();
    std::string o = "{\"clock\":";
    jsonString(o, clockText(w));
    o += ",\"day\":" + std::to_string(w.day);
    char buf[128];
    std::snprintf(buf, sizeof buf, ",\"tick\":%" PRId64 ",\"x\":%.3f,\"z\":%.3f,\"yaw\":%.3f,\"pots\":%d", w.tick,
                  static_cast<double>(w.player.x), static_cast<double>(w.player.z), static_cast<double>(w.player.yaw),
                  static_cast<int>(w.potsMade));
    o += buf;
    jsonBool(o, "captured", s.captured());
    jsonBool(o, "help", s.help());
    jsonBool(o, "watching", s.watching());
    jsonBool(o, "after", w.after);
    jsonBool(o, "deep", w.sleep == Sleep::Deep);
    jsonBool(o, "codex", s.codexOpen());
    jsonBool(o, "naming", w.naming >= 0);
    jsonBool(o, "atWheel", atWheel(w) && w.naming < 0 && !w.throwing.active);
    jsonBool(o, "lens", lensAvailable(w));
    jsonBool(o, "lampLit", w.lampLit);
    jsonBool(o, "muted", s.muted());
    jsonBool(o, "captions", s.captions());
    o += ",\"textSize\":" + std::to_string(s.textSize());
    o += ",\"caption\":";
    jsonString(o, s.caption());
    o += ",\"codexOpen\":" + std::to_string(codex::openEntries(w).size());
    o += ",\"typing\":";
    jsonString(o, s.typing());
    o += ",\"message\":";
    jsonString(o, s.message());
    o += ",\"prompt\":";
    jsonString(o, s.watching() ? std::string() : play::prompt(w, s.typing()));
    o += ",\"made\":[";
    bool first = true;
    for (const std::string& name : inventory(w)) {
        if (!first) o += ',';
        first = false;
        jsonString(o, name);
    }
    o += "],\"hud\":[";
    first = true;
    for (const play::HudLine& l : play::hudLines(w, s.watching())) {
        if (!first) o += ',';
        first = false;
        o += "{\"heading\":";
        o += l.heading ? "true" : "false";
        o += ",\"text\":";
        jsonString(o, l.text);
        o += '}';
    }
    o += "]}";
    g->statusJson = std::move(o);
    return g->statusJson.c_str();
}

}  // extern "C"

int main() {
    static Game game;
    g = &game;

    double cw = 0, ch = 0;
    emscripten_get_element_css_size("#canvas", &cw, &ch);
    if (cw < 1 || ch < 1) {
        cw = 1280;
        ch = 720;
    }
    strata::EngineDesc desc;
    desc.surface = STRATA_SURFACE_WINDOW;
    desc.width = static_cast<int32_t>(cw);
    desc.height = static_cast<int32_t>(ch);
    desc.title = "Atman";
    try {
        g->engine = strata::Engine::create(desc);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "atman: cannot start the renderer: %s\n", ex.what());
        std::string js = "if (Module.onRendererFailed) Module.onRendererFailed(";
        jsonString(js, ex.what());
        js += ")";
        emscripten_run_script(js.c_str());
        return 1;
    }
    play::setLighting(*g->engine, true);

    play::SessionOptions o;
    o.browser = true;
    o.slot = kSlot;
    o.afterSave = [] { emscripten_run_script("if (Module.flushSaves) Module.flushSaves()"); };
    g->session = std::make_unique<play::Session>(*g->engine, o);

    g->last = emscripten_get_now() / 1000.0;
    emscripten_run_script("if (Module.onGameReady) Module.onGameReady()");
    emscripten_set_main_loop_arg(frame, nullptr, 0, false);
    return 0;
}
