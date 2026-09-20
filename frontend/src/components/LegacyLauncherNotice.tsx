'use client';

import { useState } from 'react';
import { ArrowUpRight, Download } from 'lucide-react';
import { ipc } from '@/lib/ipc';
import { WeaveMark } from './WeaveMark';
import { TitleBar, WindowShell } from './WindowChrome';

// Mandatory update gate: this build is no longer accepted by the backend, so
// the only way forward is the dashboard download.
export function LegacyLauncherNotice() {
  const [openingDashboard, setOpeningDashboard] = useState(false);
  const [openFailed, setOpenFailed] = useState(false);

  const openDashboard = async () => {
    if (openingDashboard) return;

    setOpeningDashboard(true);
    setOpenFailed(false);

    const opened = await ipc.openDashboard();
    setOpeningDashboard(false);

    if (!opened) {
      setOpenFailed(true);
    }
  };

  return (
    <WindowShell>
      <TitleBar>
        <WeaveMark width={30} className="shrink-0 text-accent" />
        <span className="text-sm font-semibold text-fg-0">Weave Launcher</span>
      </TitleBar>

      <main className="flex flex-1 items-center justify-center px-8">
        <section
          aria-labelledby="launcher-update-title"
          className="flex w-full max-w-md flex-col items-center gap-5 text-center"
        >
          <span className="flex size-14 items-center justify-center rounded-2xl border border-accent/30 bg-accent/10 text-accent">
            <Download className="size-6" aria-hidden="true" />
          </span>

          <div className="flex flex-col gap-1.5">
            <h1 id="launcher-update-title" className="text-lg font-semibold text-fg-0">
              Update required
            </h1>
            <p className="text-xs leading-relaxed text-fg-1">
              This launcher build is no longer supported. Download the current version from the dashboard to continue.
            </p>
          </div>

          <div className="flex w-full flex-col items-center gap-2">
            <button
              type="button"
              onClick={openDashboard}
              disabled={openingDashboard}
              className="flex h-10 w-full max-w-xs items-center justify-center gap-2 rounded-full bg-accent text-[13px] font-bold uppercase tracking-wide text-accent-fg shadow-[0_0_18px_rgba(255,140,0,0.25)] transition hover:bg-accent-hover active:scale-[0.97] disabled:cursor-wait disabled:opacity-60 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-ink-0"
            >
              <ArrowUpRight className="size-4" aria-hidden="true" />
              {openingDashboard ? 'Opening dashboard…' : 'Open dashboard'}
            </button>
            <button
              type="button"
              onClick={() => ipc.close()}
              className="h-8 rounded-full px-4 text-xs font-medium text-fg-2 transition hover:text-fg-0 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent"
            >
              Exit
            </button>
          </div>

          {openFailed && (
            <p className="text-xs text-danger" role="alert">
              Could not open the dashboard. Please try again.
            </p>
          )}
        </section>
      </main>
    </WindowShell>
  );
}
