#!/usr/bin/env python3
"""Render the codex plates, or check that they are current.

Each entry in content/codex/*.json becomes content/plates/<id>.png: a 1600 x
900 page with the verse in Devanagari, IAST and English, the translator, the
edition and the citation IDs. Strata's text overlay is ASCII only, so the
game shows these images instead (see ARCHITECTURE.md, "Text"). The left
column is left empty: the game draws the codex's menu there.

Each plate records a SHA-256 of its entry and of this script in a PNG text
chunk, so --check can tell a stale plate without Pillow or the fonts.

Usage:
  python3 tools/plates/make_plates.py          render every plate (needs Pillow with libraqm)
  python3 tools/plates/make_plates.py --check  validate the entries and plates; exits 1 on any problem
"""
import hashlib
import json
import re
import struct
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
CODEX = ROOT / "content" / "codex"
PLATES = ROOT / "content" / "plates"
FONTS = Path(__file__).resolve().parent / "fonts"
KEY = "atman-plate-source"

UNLOCKS = {"RopeAndSnake", "ClayAndPots", "PotSpace", "Dream", "Fourth", "always"}
REQUIRED = ["id", "menu", "unlock", "title", "source", "cites", "devanagari", "iast", "english", "translator",
            "edition"]
MENU_LENGTH = 22  # the menu column fits this many characters after the "> " marker

WIDTH, HEIGHT = 1600, 900
MENU_WIDTH = 460  # the dark column the game writes the menu on
LEFT, RIGHT, TOP, BOTTOM = 540, 1540, 56, 860

PAPER = (236, 226, 204)
PANEL = (35, 26, 19)
INK = (43, 30, 22)
MUTED = (110, 84, 62)
RED = (128, 38, 20)
RULE = (190, 168, 134)


def source_hash(entry_path):
    h = hashlib.sha256()
    h.update(entry_path.read_bytes())
    h.update(Path(__file__).read_bytes())
    return h.hexdigest()


def png_text(path):
    """The tEXt chunks of a PNG, read without Pillow."""
    data = path.read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        return None
    out, pos = {}, 8
    while pos + 8 <= len(data):
        length, kind = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + length]
        if kind == b"tEXt" and b"\0" in body:
            k, v = body.split(b"\0", 1)
            out[k.decode("latin-1")] = v.decode("latin-1")
        elif kind == b"zTXt" and b"\0" in body:
            k, v = body.split(b"\0", 1)
            out[k.decode("latin-1")] = zlib.decompress(v[1:]).decode("latin-1")
        pos += 12 + length
    return out


def entries():
    return sorted(CODEX.glob("*.json"))


def validate(path, entry, defined):
    rel = path.relative_to(ROOT)
    problems = []
    for key in REQUIRED:
        if not entry.get(key):
            problems.append(f"{rel}: '{key}' is missing or empty")
    if problems:
        return problems
    if not re.fullmatch(r"\d+-" + re.escape(entry["id"]) + r"\.json", path.name):
        problems.append(f"{rel}: the file name should be <order>-{entry['id']}.json")
    if entry["unlock"] not in UNLOCKS:
        problems.append(f"{rel}: unlock '{entry['unlock']}' is not one of {sorted(UNLOCKS)}")
    menu = entry["menu"]
    if not all(32 <= ord(c) <= 126 for c in menu) or len(menu) > MENU_LENGTH:
        problems.append(f"{rel}: menu '{menu}' must be printable ASCII of at most {MENU_LENGTH} characters")
    if len(entry["devanagari"]) != len(entry["iast"]):
        problems.append(f"{rel}: devanagari and iast need the same number of lines")
    cites = list(entry["cites"])
    for r in entry.get("readings", []):
        cites += r.get("cites", [])
    for text in entry.get("notes", []):
        cites += re.findall(r"\[([PCSMG]\d+)\]", text)
    for cid in cites:
        if cid not in defined:
            problems.append(f"{rel}: citation [{cid}] is not defined in RESEARCH.md")
    return problems


def check():
    defined = set(re.findall(r"^\| ([PCSMG]\d+) \|", (ROOT / "RESEARCH.md").read_text(encoding="utf-8"), re.M))
    problems, ids, unlocks = [], set(), []
    for path in entries():
        entry = json.loads(path.read_text(encoding="utf-8"))
        problems += validate(path, entry, defined)
        ids.add(entry.get("id"))
        unlocks.append(entry.get("unlock"))
        plate = PLATES / f"{entry.get('id')}.png"
        if not plate.exists():
            problems.append(f"{plate.relative_to(ROOT)}: missing; run tools/plates/make_plates.py")
            continue
        text = png_text(plate) or {}
        if text.get(KEY) != source_hash(path):
            problems.append(f"{plate.relative_to(ROOT)}: out of date with {path.name}; run tools/plates/make_plates.py")
    for u in UNLOCKS - {"always"}:
        if unlocks.count(u) != 1:
            problems.append(f"content/codex: {u} should open exactly one entry, not {unlocks.count(u)}")
    for plate in PLATES.glob("*.png"):
        if plate.stem not in ids:
            problems.append(f"{plate.relative_to(ROOT)}: no entry in content/codex")
    for p in problems:
        print(p)
    print(f"{len(entries())} codex entries, {len(problems)} problems")
    return 1 if problems else 0


# ---- rendering

class Page:
    def __init__(self, scale):
        from PIL import Image, ImageDraw, ImageFont
        self.Image, self.ImageFont = Image, ImageFont
        self.scale = scale
        self.image = Image.new("RGB", (WIDTH, HEIGHT), PAPER)
        self.draw = ImageDraw.Draw(self.image)
        self.draw.rectangle([0, 0, MENU_WIDTH, HEIGHT], fill=PANEL)
        self.y = TOP
        self.fonts = {}

    def font(self, size, italic=False):
        size = round(size * self.scale)
        key = (size, italic)
        if key not in self.fonts:
            name = "TiroDevanagariSanskrit-Italic.ttf" if italic else "TiroDevanagariSanskrit-Regular.ttf"
            self.fonts[key] = self.ImageFont.truetype(str(FONTS / name), size,
                                                      layout_engine=self.ImageFont.Layout.RAQM)
        return self.fonts[key]

    def wrap(self, text, font, width, language):
        words, lines, line = text.split(" "), [], ""
        for word in words:
            trial = word if not line else line + " " + word
            if self.draw.textlength(trial, font=font, language=language) <= width or not line:
                line = trial
            else:
                lines.append(line)
                line = word
        if line:
            lines.append(line)
        return lines

    def text(self, text, size, colour, italic=False, indent=0, gap=0.0, language="en", leading=1.32):
        font = self.font(size, italic)
        for line in self.wrap(text, font, RIGHT - LEFT - indent, language):
            self.draw.text((LEFT + indent, self.y), line, font=font, fill=colour, language=language)
            self.y += round(size * self.scale * leading)
        self.y += round(gap * self.scale)

    def rule(self, gap):
        self.draw.line([(LEFT, self.y), (RIGHT, self.y)], fill=RULE, width=2)
        self.y += round(gap * self.scale)


def layout(entry, scale):
    page = Page(scale)
    page.text(entry["title"], 46, INK, gap=2)
    page.text(entry["source"], 26, MUTED, italic=True, gap=14)
    page.rule(22)
    for line in entry["devanagari"]:
        page.text(line, 36, RED, language="sa", leading=1.5)
    page.y += round(8 * scale)
    for line in entry["iast"]:
        page.text(line, 25, MUTED, italic=True, leading=1.3)
    page.y += round(18 * scale)
    for para in entry["english"]:
        page.text(para, 28, INK, gap=10)
    for r in entry.get("readings", []):
        cites = " ".join(f"[{c}]" for c in r.get("cites", []))
        page.text(f"{r['school']}  ·  {r['teacher']}", 25, RED, gap=0)
        page.text(f"{r['text']} {cites}".strip(), 23, INK, indent=24, gap=8)
    for note in entry.get("notes", []):
        page.text(note, 23, MUTED, italic=True, gap=8)
    page.y += round(6 * scale)
    page.rule(12)
    page.text("Translation: " + entry["translator"], 19, MUTED, leading=1.25)
    page.text(entry["edition"], 19, MUTED, leading=1.25)
    cites = list(entry["cites"])
    for c in [c for r in entry.get("readings", []) for c in r.get("cites", [])] + \
            [c for n in entry.get("notes", []) for c in re.findall(r"\[([PCSMG]\d+)\]", n)]:
        if c not in cites:
            cites.append(c)
    page.text("Sources: " + " ".join(f"[{c}]" for c in cites) + "  (see RESEARCH.md)", 19, MUTED, leading=1.25)
    return page


def render():
    from PIL import PngImagePlugin, features
    if not features.check("raqm"):
        print("make_plates: Pillow was built without libraqm, so Devanagari cannot be shaped")
        return 1
    PLATES.mkdir(parents=True, exist_ok=True)
    for path in entries():
        entry = json.loads(path.read_text(encoding="utf-8"))
        # Shrink a page whose text runs past the bottom margin, a step at a time.
        scale = 1.0
        page = layout(entry, scale)
        while page.y > BOTTOM and scale > 0.6:
            scale = round(scale - 0.04, 2)
            page = layout(entry, scale)
        if page.y > BOTTOM:
            print(f"make_plates: {path.name} does not fit on a plate; shorten it")
            return 1
        info = PngImagePlugin.PngInfo()
        info.add_text(KEY, source_hash(path))
        out = PLATES / f"{entry['id']}.png"
        page.image.save(out, pnginfo=info, optimize=True)
        print(f"{out.relative_to(ROOT)}  scale {scale:.2f}  {out.stat().st_size // 1024} KiB")
    return 0


if __name__ == "__main__":
    sys.exit(check() if "--check" in sys.argv[1:] else render())
