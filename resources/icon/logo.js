// Vector reconstruction of the Weave "W" mark, in a 256x256 coordinate space
// (matches the original raster: bbox x 12..243, y 80..175, centre x = 127.5).
const CX = 127.5;
const ORANGE = '#FF9C42';

// Left-half polygons; the right half is a mirror around CX.
function leftHalf({ simplified = false, band = 24 } = {}) {
  // Outer stroke: outer edge L1 x = 12 + 0.70(y-80); inner edge offset by `band`.
  const L1 = y => 12 + 0.70 * (y - 80);
  const L2 = y => 12 + band + 0.70 * (y - 80);
  // Centre leg: outer edge L3 from apex (CX,79) slope -0.75; inner edge L4 from (CX,110).
  const legIn = simplified ? 112 - (band - 24) * 1.3 : 112;
  const L3 = y => CX - 0.75 * (y - 79);
  const L4 = y => CX - 0.75 * (y - legIn);
  const BOTTOM = 175.5;

  // L2 ∩ L3 (where outer stroke meets the leg)
  const yMerge = (CX + 0.75 * 79 - 12 - band + 0.70 * 80) / 1.45;
  // L1 ∩ L4 (tip)
  const yTip = (CX + 0.75 * legIn - 12 + 0.70 * 80) / 1.45;
  const tipY = Math.min(yTip, BOTTOM);

  const polys = [];
  // Main V (outer stroke + centre-left leg)
  polys.push([
    [12, 80], [12 + band, 80],
    [L2(yMerge), yMerge],
    [CX, 79], [CX, legIn],
    [L4(tipY), tipY], [L1(tipY), tipY],
  ]);
  if (!simplified) {
    // Top cap bar
    polys.push([[12, 80], [85, 80], [78.5, 93.5], [L1(93.5), 93.5]]);
    // Inner tail hanging from the cap
    const L5 = y => 57.5 + 0.77 * (y - 94);
    const L6 = y => 79 + 0.64 * (y - 93.5);
    const L7 = y => 80 - 0.75 * (y - 120);
    const y67 = (80 + 0.75 * 120 - 79 + 0.64 * 93.5) / 1.39;
    const y57 = (80 + 0.75 * 120 - 57.5 + 0.77 * 94) / 1.52;
    polys.push([[L5(93.5), 93.5], [78.5, 93.5], [L6(y67), y67], [L7(y57), y57]]);
    // Small bottom chevron
    polys.push([[CX, 134.5], [CX - 0.74 * (BOTTOM - 134.5), BOTTOM], [CX - 0.82 * (BOTTOM - 161.5), BOTTOM], [CX, 161.5]]);
  }
  return polys;
}

function markPath(opts) {
  const polys = leftHalf(opts);
  const mirrored = polys.map(p => p.map(([x, y]) => [2 * CX - x, y]));
  return [...polys, ...mirrored]
    .map(p => 'M' + p.map(([x, y]) => `${x.toFixed(2)} ${y.toFixed(2)}`).join('L') + 'Z')
    .join('');
}

// Bare mark, 256x256 canvas, transparent background (same framing as original).
function markSvg(opts) {
  return `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256" width="256" height="256">
<path fill="${ORANGE}" fill-rule="nonzero" d="${markPath(opts)}"/></svg>`;
}

module.exports = { markSvg, ORANGE };
