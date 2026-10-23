// Smoke test for the browser build in headless Chromium (Playwright).
//
//   node web/smoke/smoke.cjs <url> <out-dir> [--phone]
//
// Checks: the page starts with no page errors and shows How to play on a first
// visit; the canvas draws the valley (not one flat colour); walking moves the
// player (W on a desktop, the stick on the phone profile) and looking turns
// them; the lamp lights; the codex opens on its page and closes again. On a
// desktop the player also walks to the potter's wheel, throws a bowl and
// types its name, which lands in the list of things made. Writes PNGs to
// <out-dir>.
'use strict';
const fs = require('fs');
const path = require('path');
const zlib = require('zlib');
const { chromium, devices } = require('playwright');

const [url, outDir] = process.argv.slice(2);
const phone = process.argv.includes('--phone');
if (!url || !outDir) {
  console.error('usage: smoke.cjs <url> <out-dir> [--phone]');
  process.exit(2);
}
fs.mkdirSync(outDir, { recursive: true });

// Minimal PNG decoder (8-bit RGB/RGBA, non-interlaced: what Chromium writes).
function decodePng(buf) {
  let pos = 8, width = 0, height = 0, channels = 0;
  const idat = [];
  while (pos < buf.length) {
    const len = buf.readUInt32BE(pos);
    const type = buf.toString('ascii', pos + 4, pos + 8);
    const data = buf.subarray(pos + 8, pos + 8 + len);
    if (type === 'IHDR') {
      width = data.readUInt32BE(0);
      height = data.readUInt32BE(4);
      channels = { 2: 3, 6: 4 }[data[9]];
      if (data[8] !== 8 || !channels || data[12] !== 0) throw new Error('unsupported PNG');
    } else if (type === 'IDAT') idat.push(data);
    pos += 12 + len;
  }
  const raw = zlib.inflateSync(Buffer.concat(idat));
  const stride = width * channels;
  const out = Buffer.alloc(height * stride);
  for (let y = 0; y < height; y++) {
    const f = raw[y * (stride + 1)];
    for (let x = 0; x < stride; x++) {
      const v = raw[y * (stride + 1) + 1 + x];
      const a = x >= channels ? out[y * stride + x - channels] : 0;
      const b = y > 0 ? out[(y - 1) * stride + x] : 0;
      const c = x >= channels && y > 0 ? out[(y - 1) * stride + x - channels] : 0;
      let p;
      if (f === 0) p = 0;
      else if (f === 1) p = a;
      else if (f === 2) p = b;
      else if (f === 3) p = (a + b) >> 1;
      else {
        const pa = Math.abs(b - c), pb = Math.abs(a - c), pc = Math.abs(a + b - 2 * c);
        p = pa <= pb && pa <= pc ? a : pb <= pc ? b : c;
      }
      out[y * stride + x] = (v + p) & 255;
    }
  }
  return { width, height, channels, data: out };
}

// Distinct colours (quantised to 4 bits per channel) in a PNG.
function colourCount(png) {
  const img = decodePng(png);
  const seen = new Set();
  for (let i = 0; i < img.data.length; i += img.channels)
    seen.add(((img.data[i] >> 4) << 8) | ((img.data[i + 1] >> 4) << 4) | (img.data[i + 2] >> 4));
  return seen.size;
}

(async () => {
  const browser = await chromium.launch({ args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader'] });
  const base = phone ? devices['iPhone 13'] : { viewport: { width: 1280, height: 800 } };
  // "headless" in the UA keeps arcade from counting a smoke run as a play.
  const context = await browser.newContext({ ...base, userAgent: (base.userAgent || 'Mozilla/5.0') + ' atman-smoke headless' });
  const page = await context.newPage();
  const errors = [];
  page.on('pageerror', (e) => errors.push(String(e)));
  page.on('console', (m) => { if (m.type() === 'error') errors.push('console: ' + m.text()); });
  const fail = async (why) => {
    console.error('FAIL: ' + why);
    await page.screenshot({ path: path.join(outDir, 'failure.png') }).catch(() => {});
    await browser.close();
    process.exit(1);
  };

  // The main loop keeps the load event from ever firing: wait for commit only.
  await page.goto(url, { waitUntil: 'commit' });
  try {
    await page.waitForFunction(() => window.atman && window.atmanStatus, null, { timeout: 90000 });
  } catch (e) {
    await fail('the game never became ready: ' + errors.join(' | '));
  }
  const status = async () => JSON.parse(await page.evaluate(() => window.atman.status()));
  // Input is handled on the next frame, and SwiftShader frames are slow: poll.
  const until = async (pred, ms = 20000) => {
    const end = Date.now() + ms;
    let st = await status();
    while (!pred(st) && Date.now() < end) {
      await page.waitForTimeout(200);
      st = await status();
    }
    return st;
  };

  if (!(await page.locator('#help').isVisible())) await fail('How to play was not shown on a first visit');
  await page.locator('#helpClose').click();
  let s = await status();
  console.log(`ready: ${s.clock} at (${s.x}, ${s.z})`);
  if (s.clock !== '4:30 pm') await fail('a first visit should start at 4:30 pm, not ' + s.clock);

  await page.waitForTimeout(1500);
  const shot = await page.screenshot({ path: path.join(outDir, 'atman.png'), timeout: 180000 });
  const colours = colourCount(shot);
  console.log(`screenshot: ${colours} distinct colours`);
  if (colours < 12) await fail('the canvas looks blank (' + colours + ' colours)');

  const start = { x: s.x, z: s.z, yaw: s.yaw };
  const box = await page.locator('#canvas').boundingBox();
  if (phone) {
    if (!(await page.locator('#panel').isVisible())) await fail('the HUD panel is not shown on a narrow page');
    const panel = await page.locator('#panel').innerText();
    if (!/QUESTS/.test(panel)) await fail('the HUD panel does not show the quests: ' + panel);
    // Walk: hold the stick forward on the left half.
    const cdp = await context.newCDPSession(page);
    const sx = box.x + box.width * 0.2, sy = box.y + box.height * 0.6;
    const touch = (type, pts) => cdp.send('Input.dispatchTouchEvent', { type, touchPoints: pts });
    await touch('touchStart', [{ x: sx, y: sy, id: 1 }]);
    for (let i = 1; i <= 6; i++) await touch('touchMove', [{ x: sx, y: sy - 10 * i, id: 1 }]);
    s = await until((st) => Math.hypot(st.x - start.x, st.z - start.z) > 0.3);
    await touch('touchEnd', []);
    console.log(`after the stick: at (${s.x}, ${s.z})`);
    if (Math.hypot(s.x - start.x, s.z - start.z) <= 0.3) await fail('the stick did not walk the player');
    // Look: drag on the right half.
    const lx = box.x + box.width * 0.75, ly = box.y + box.height * 0.4;
    await touch('touchStart', [{ x: lx, y: ly, id: 2 }]);
    for (let i = 1; i <= 5; i++) await touch('touchMove', [{ x: lx + 12 * i, y: ly, id: 2 }]);
    await touch('touchEnd', []);
    s = await until((st) => Math.abs(st.yaw - start.yaw) > 0.05);
    if (Math.abs(s.yaw - start.yaw) <= 0.05) await fail('a drag on the right half did not turn the view');
    // The lamp, from its button (after the drag's touch has ended, or the tap
    // reads as a second finger and makes no click).
    await page.waitForTimeout(400);
    await page.locator('#pad [data-key="9"]').tap();
    s = await until((st) => st.lampLit);
    if (!s.lampLit) await fail('the Lamp button did not light the lamp');
    // The codex, from the top bar, and back.
    await page.locator('#codexBtn').tap();
    s = await until((st) => st.codex);
    if (!s.codex) await fail('the Codex button did not open the codex');
    await page.waitForTimeout(1500);
    await page.screenshot({ path: path.join(outDir, 'atman-codex.png'), timeout: 180000 });
    await page.locator('#codexNav [data-key="43"]').tap();
    s = await until((st) => !st.codex);
    if (s.codex) await fail('Back to the valley did not close the codex');
  } else {
    await page.keyboard.down('KeyW');
    s = await until((st) => Math.hypot(st.x - start.x, st.z - start.z) > 0.3);
    await page.keyboard.up('KeyW');
    console.log(`after W: at (${s.x}, ${s.z})`);
    if (Math.hypot(s.x - start.x, s.z - start.z) <= 0.3) await fail('W did not walk the player');
    // Closing How to play took the pointer lock: the mouse now looks.
    s = await until((st) => st.captured, 10000);
    if (!s.captured) await fail('the pointer lock was not taken when How to play closed');
    // Headless Chromium fakes movement under the pointer lock (it jumps to the
    // cursor and back), which the page drops as spikes: look through the same
    // call the page's mouse handler makes.
    await page.evaluate(() => window.atman.look(200, 0));
    s = await until((st) => Math.abs(st.yaw - start.yaw) > 0.1);
    console.log(`after looking: yaw ${start.yaw} -> ${s.yaw}`);
    if (Math.abs(s.yaw - start.yaw) <= 0.1) await fail('looking did not turn the view');
    await page.keyboard.press('KeyF');
    s = await until((st) => st.lampLit);
    if (!s.lampLit) await fail('F did not light the lamp');
    // Tab is the codex here, not the browser's focus.
    await page.keyboard.press('Tab');
    s = await until((st) => st.codex);
    if (!s.codex) await fail('Tab did not open the codex');
    await page.waitForTimeout(1500);
    await page.screenshot({ path: path.join(outDir, 'atman-codex.png'), timeout: 180000 });
    await page.keyboard.press('Tab');
    s = await until((st) => !st.codex);
    if (s.codex) await fail('Tab did not close the codex');

    // To the potter's wheel, steering with the same look() call, hurrying.
    const route = [[-1.0, 4.5], [-1.5, -3.0], [6.0, -6.0], [9.0, -12.2]];
    await page.keyboard.down('ShiftLeft');
    for (const [tx, tz] of route) {
      const end = Date.now() + 90000;
      await page.keyboard.down('KeyW');
      for (;;) {
        s = await status();
        const dx = tx - s.x, dz = tz - s.z;
        if (Math.hypot(dx, dz) < 0.8 || (tx === 9.0 && s.atWheel) || Date.now() > end) break;
        let turn = Math.atan2(dx, -dz) - s.yaw;
        while (turn > Math.PI) turn -= 2 * Math.PI;
        while (turn < -Math.PI) turn += 2 * Math.PI;
        await page.evaluate((px) => window.atman.look(px, 0), turn / 0.0025);
        await page.waitForTimeout(100);
      }
      await page.keyboard.up('KeyW');
    }
    await page.keyboard.up('ShiftLeft');
    s = await until((st) => st.atWheel, 5000);
    console.log(`at the wheel: (${s.x}, ${s.z})`);
    if (!s.atWheel) await fail('the walk to the wheel did not arrive');
    await page.keyboard.press('Digit2');
    s = await until((st) => st.naming, 60000);
    if (!s.naming) await fail('2 at the wheel did not throw a bowl to name');
    await page.keyboard.type('Smoke bowl', { delay: 60 });
    s = await until((st) => st.typing === 'Smoke bowl', 20000);
    if (s.typing !== 'Smoke bowl') await fail('typing did not reach the name: "' + s.typing + '"');
    await page.keyboard.press('Enter');
    s = await until((st) => !st.naming);
    console.log(`made: ${JSON.stringify(s.made)}`);
    if (!s.made.includes('Smoke bowl')) await fail('the named bowl is not in the list of things made');
    await page.screenshot({ path: path.join(outDir, 'atman-wheel.png'), timeout: 180000 });

    // Esc's job in a browser: give the mouse back, as the page's buttons need it.
    await page.evaluate(() => document.exitPointerLock());
    s = await until((st) => !st.captured, 10000);
    if (s.captured) await fail('releasing the pointer lock did not reach the game');
    if (!(await page.locator('#clickToPlay').isVisible())) await fail('"Click to play" is not shown without the lock');
  }
  await page.screenshot({ path: path.join(outDir, 'atman-moved.png'), timeout: 180000 });

  if (errors.length) await fail('page errors: ' + errors.join(' | '));
  console.log(`OK (${phone ? 'phone' : 'desktop'})`);
  await browser.close();
})().catch((e) => {
  console.error(e);
  process.exit(1);
});
