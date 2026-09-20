'use client';

import React from 'react';
import { WeaveMark } from './WeaveMark';
import { TitleBar, WindowShell } from './WindowChrome';

interface SplashProps {
  /** Screen-reader label; the splash itself shows no text. */
  label?: string;
}

// Loading state shared by the Suspense fallback and the session check: an
// indeterminate bar under the usual title bar. The animation is plain CSS
// (globals.css) so the fallback needs no JS to move.
export function Splash({ label = 'Loading' }: SplashProps) {
  return (
    <WindowShell>
      <TitleBar>
        <WeaveMark width={30} className="shrink-0 text-accent" />
        <span className="text-sm font-semibold text-fg-0">Weave Launcher</span>
      </TitleBar>
      <main role="status" aria-label={label} className="flex flex-1 items-center justify-center">
        <div className="splash-bar h-0.5 w-40 overflow-hidden rounded-full bg-ink-3" aria-hidden="true">
          <span className="block h-full w-1/3 rounded-full bg-accent shadow-[0_0_8px_rgb(var(--accent))]" />
        </div>
      </main>
    </WindowShell>
  );
}
