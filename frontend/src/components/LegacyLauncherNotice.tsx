'use client';

import { useState } from 'react';
import { ArrowUpRight, CircleAlert, Minus, X } from 'lucide-react';
import { ipc } from '@/lib/ipc';

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
    <div className="flex h-screen w-screen flex-col bg-[#0d0d0d] font-sans text-[#ddd] selection:bg-[#ff8c00] selection:text-black">
      <header
        onMouseDown={(event) => {
          if ((event.target as HTMLElement).closest('.no-drag')) return;
          ipc.startDrag();
        }}
        className="drag-region flex h-14 shrink-0 select-none items-center border-b border-[#222] bg-[#121212] px-4"
      >
        <span className="text-sm font-bold tracking-wide text-white">Launcher update</span>
        <div className="no-drag ml-auto flex items-center gap-1">
          <button
            type="button"
            onClick={() => ipc.minimize()}
            className="flex size-7 items-center justify-center rounded text-[#666] transition hover:bg-[#222] hover:text-white focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-[#ff8c00]"
            aria-label="Minimize launcher"
          >
            <Minus className="size-4" aria-hidden="true" />
          </button>
          <button
            type="button"
            onClick={() => ipc.close()}
            className="flex size-7 items-center justify-center rounded text-[#666] transition hover:bg-red-500/20 hover:text-red-400 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-red-400"
            aria-label="Close launcher"
          >
            <X className="size-4" aria-hidden="true" />
          </button>
        </div>
      </header>

      <main className="flex-1 overflow-y-auto px-6 py-5">
        <div className="flex items-center gap-2 text-base font-bold tracking-wide text-white">
          <CircleAlert className="size-4 text-[#ff8c00]" aria-hidden="true" />
          <h1>Update required</h1>
        </div>

        <section className="relative mt-6 max-w-3xl pl-8" aria-labelledby="launcher-update-message">
          <div className="absolute bottom-[-30px] left-[10px] top-[20px] w-0.5 bg-[#222]" aria-hidden="true" />
          <div className="absolute left-[5px] top-[4px] size-3 rotate-45 bg-[#ff8c00] shadow-[0_0_10px_rgba(255,140,0,0.55)]" aria-hidden="true" />

          <h2 id="launcher-update-message" className="font-mono text-sm font-bold text-[#f5f1e8]">
            Please update the launcher
          </h2>
          <p className="mt-2 max-w-xl text-xs leading-relaxed text-[#aaa]">
            This version is no longer supported. Open the dashboard to download the current launcher and continue.
          </p>

          <div className="mt-4 max-w-xl rounded-lg border border-[#292929] bg-[#121212] p-4">
            <p className="text-xs font-semibold text-[#ebe6dc]">Update required to continue</p>
            <p className="mt-1 text-[11px] text-[#777]">The launcher will remain unavailable until it is updated.</p>
          </div>

          <button
            type="button"
            onClick={openDashboard}
            disabled={openingDashboard}
            className="mt-4 inline-flex min-h-10 items-center gap-2 rounded-md border border-[#3b3b3b] bg-[#171717] px-4 text-[11px] font-bold uppercase tracking-wide text-[#e8e3da] transition hover:border-[#555] hover:bg-[#202020] focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-[#ff8c00] disabled:cursor-wait disabled:opacity-60"
          >
            <ArrowUpRight className="size-3.5 text-[#ff8c00]" aria-hidden="true" />
            {openingDashboard ? 'Opening dashboard...' : 'Go to dashboard'}
          </button>

          {openFailed && (
            <p className="mt-3 text-xs text-red-400" role="alert">
              Could not open the dashboard. Please try again.
            </p>
          )}
        </section>
      </main>

      <footer className="h-20 shrink-0 border-t border-[#222] bg-[#121212]" aria-hidden="true" />
    </div>
  );
}
