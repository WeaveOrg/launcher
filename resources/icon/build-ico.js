// Builds resources/app.ico (and the frontend favicon) from the vector mark.
// 256px is stored as PNG, everything else as uncompressed 32bpp BMP, which is
// what Windows / GDI / .NET expect for the small sizes.
//
//   node resources/icon/build-ico.js            # writes app.ico + favicon.ico
//   node resources/icon/build-ico.js --preview  # also writes preview sheets next to this file
//
// Requires `sharp` from frontend/node_modules (run `npm install` in frontend/ first).
const path = require('path');
const fs = require('fs');
const ROOT = path.resolve(__dirname, '..', '..');
const sharp = require(path.join(ROOT, 'frontend', 'node_modules', 'sharp'));
const { markSvg } = require('./logo.js');

const PREVIEW = process.argv.includes('--preview');
const OUT_DIR = __dirname;
const TARGETS = [
  path.join(ROOT, 'resources', 'app.ico'),
  path.join(ROOT, 'frontend', 'src', 'app', 'favicon.ico'),
];
const SIZES = [256, 128, 64, 48, 40, 32, 24, 20, 16];

async function renderRaw(px) {
  // Bare mark on transparent background, same framing as mark.svg, at every size.
  const svg = Buffer.from(markSvg());
  const { data, info } = await sharp(svg, { density: 72 * 4 })
    .resize(px, px)
    .ensureAlpha()
    .raw()
    .toBuffer({ resolveWithObject: true });
  if (info.width !== px || info.height !== px) throw new Error(`bad size ${info.width}`);
  return data; // RGBA, top-down
}

function toBmpEntry(rgba, px) {
  const rowBytes = px * 4;
  const andRow = Math.ceil(px / 32) * 4;
  const header = Buffer.alloc(40);
  header.writeUInt32LE(40, 0);        // biSize
  header.writeInt32LE(px, 4);         // biWidth
  header.writeInt32LE(px * 2, 8);     // biHeight (XOR + AND)
  header.writeUInt16LE(1, 12);        // biPlanes
  header.writeUInt16LE(32, 14);       // biBitCount
  header.writeUInt32LE(0, 16);        // BI_RGB
  header.writeUInt32LE(rowBytes * px + andRow * px, 20);
  const xor = Buffer.alloc(rowBytes * px);
  const and = Buffer.alloc(andRow * px); // all 0 = opaque per mask; alpha channel rules
  for (let y = 0; y < px; y++) {
    const srcRow = px - 1 - y; // bottom-up
    for (let x = 0; x < px; x++) {
      const s = (srcRow * px + x) * 4;
      const d = y * rowBytes + x * 4;
      const a = rgba[s + 3];
      // straight (non-premultiplied) BGRA
      xor[d] = rgba[s + 2];
      xor[d + 1] = rgba[s + 1];
      xor[d + 2] = rgba[s];
      xor[d + 3] = a;
      if (a === 0) and[y * andRow + (x >> 3)] |= 0x80 >> (x & 7);
    }
  }
  return Buffer.concat([header, xor, and]);
}

async function toPngEntry(rgba, px) {
  return sharp(rgba, { raw: { width: px, height: px, channels: 4 } })
    .png({ compressionLevel: 9, palette: false })
    .toBuffer();
}

async function main() {
  const entries = [];
  const previews = [];
  for (const px of SIZES) {
    const rgba = await renderRaw(px);
    const bytes = px === 256 ? await toPngEntry(rgba, 256) : toBmpEntry(rgba, px);
    entries.push({ px, bytes });
    previews.push({ px, rgba });
  }

  const dirSize = 6 + 16 * entries.length;
  let offset = dirSize;
  const dir = Buffer.alloc(dirSize);
  dir.writeUInt16LE(0, 0);
  dir.writeUInt16LE(1, 2);
  dir.writeUInt16LE(entries.length, 4);
  entries.forEach((e, i) => {
    const o = 6 + i * 16;
    dir.writeUInt8(e.px === 256 ? 0 : e.px, o);
    dir.writeUInt8(e.px === 256 ? 0 : e.px, o + 1);
    dir.writeUInt8(0, o + 2);
    dir.writeUInt8(0, o + 3);
    dir.writeUInt16LE(1, o + 4);
    dir.writeUInt16LE(32, o + 6);
    dir.writeUInt32LE(e.bytes.length, o + 8);
    dir.writeUInt32LE(offset, o + 12);
    offset += e.bytes.length;
  });
  const ico = Buffer.concat([dir, ...entries.map(e => e.bytes)]);
  for (const t of TARGETS) {
    fs.writeFileSync(t, ico);
    console.log(path.relative(ROOT, t), ico.length, 'bytes');
  }
  if (!PREVIEW) return;

  // Contact sheet: each size at 1:1 and at 4x nearest, on the Windows taskbar grey and on white.
  const cell = 4 * 64 + 16;
  const W = cell * previews.length, H = 2 * (256 + 32) + 64;
  const composites = [];
  let x = 0;
  for (const { px, rgba } of previews) {
    const one = await sharp(rgba, { raw: { width: px, height: px, channels: 4 } }).png().toBuffer();
    const big = await sharp(rgba, { raw: { width: px, height: px, channels: 4 } })
      .resize(Math.min(256, px * 4), Math.min(256, px * 4), { kernel: 'nearest' }).png().toBuffer();
    composites.push({ input: one, left: x + 8, top: 8 });
    composites.push({ input: big, left: x + 8, top: 80 });
    composites.push({ input: one, left: x + 8, top: 80 + 256 + 40 });
    x += cell;
  }
  await sharp({ create: { width: W, height: H, channels: 4, background: '#202020' } })
    .composite(composites).png().toFile(path.join(OUT_DIR, 'sheet_dark.png'));
  await sharp({ create: { width: W, height: H, channels: 4, background: '#f3f3f3' } })
    .composite(composites).png().toFile(path.join(OUT_DIR, 'sheet_light.png'));
}
main().catch(e => { console.error(e); process.exit(1); });
