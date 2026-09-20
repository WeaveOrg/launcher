import React from 'react';

// The Weave "W" mark. Path data is generated from resources/icon/logo.js
// (the same source as app.ico); the viewBox is cropped to the mark's bounds.
const PATH =
  'M12.00 80.00L36.00 80.00L79.81 142.59L127.50 79.00L127.50 112.00L79.88 175.50L78.85 175.50Z' +
  'M12.00 80.00L85.00 80.00L78.50 93.50L21.45 93.50Z' +
  'M57.12 93.50L78.50 93.50L88.61 108.52L78.78 121.63Z' +
  'M127.50 134.50L97.16 175.50L116.02 175.50L127.50 161.50Z' +
  'M243.00 80.00L219.00 80.00L175.19 142.59L127.50 79.00L127.50 112.00L175.13 175.50L176.15 175.50Z' +
  'M243.00 80.00L170.00 80.00L176.50 93.50L233.55 93.50Z' +
  'M197.88 93.50L176.50 93.50L166.39 108.52L176.22 121.63Z' +
  'M127.50 134.50L157.84 175.50L138.98 175.50L127.50 161.50Z';

interface WeaveMarkProps {
  /** Rendered width in px; height follows the mark's 231:96.5 aspect. */
  width?: number;
  className?: string;
}

export function WeaveMark({ width = 28, className }: WeaveMarkProps) {
  const height = (width * 96.5) / 231;
  return (
    <svg
      viewBox="12 79 231 96.5"
      width={width}
      height={height}
      className={className}
      aria-hidden="true"
      focusable="false"
    >
      <path fill="currentColor" d={PATH} />
    </svg>
  );
}

/** Square tile with the mark, for places that need an icon-shaped slot. */
export function WeaveTile({ size = 32, className = '' }: { size?: number; className?: string }) {
  return (
    <div
      className={`flex shrink-0 items-center justify-center rounded-lg border border-line bg-ink-2 text-accent ${className}`}
      style={{ width: size, height: size }}
    >
      <WeaveMark width={size * 0.62} />
    </div>
  );
}
