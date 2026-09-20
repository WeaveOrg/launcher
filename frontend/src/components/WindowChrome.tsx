'use client';

import React from 'react';
import { Minus, X } from 'lucide-react';
import { ipc } from '@/lib/ipc';

// Shared window chrome for the frameless Saucer window so every screen
// (auth, main, update gate) has the same 52px title bar and controls.

export function WindowControls() {
  return (
    <div className="no-drag flex items-center gap-0.5">
      <button
        type="button"
        onClick={() => ipc.minimize()}
        className="flex size-8 items-center justify-center rounded-md text-fg-2 transition hover:bg-ink-3 hover:text-fg-0 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent"
        aria-label="Minimize"
      >
        <Minus className="size-4" aria-hidden="true" />
      </button>
      <button
        type="button"
        onClick={() => ipc.close()}
        className="flex size-8 items-center justify-center rounded-md text-fg-2 transition hover:bg-danger/15 hover:text-danger focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-danger"
        aria-label="Close"
      >
        <X className="size-4" aria-hidden="true" />
      </button>
    </div>
  );
}

interface TitleBarProps {
  children?: React.ReactNode;
  /** Rendered between the left content and the window controls. */
  trailing?: React.ReactNode;
}

export function TitleBar({ children, trailing }: TitleBarProps) {
  return (
    <header
      onMouseDown={(e) => {
        if ((e.target as HTMLElement).closest('.no-drag')) return;
        ipc.startDrag();
      }}
      className="drag-region flex h-[52px] shrink-0 select-none items-center gap-3 border-b border-line bg-ink-1 pl-4 pr-2"
    >
      <div className="flex min-w-0 flex-1 items-center gap-3">{children}</div>
      {trailing && <div className="no-drag flex items-center gap-2">{trailing}</div>}
      <div className="ml-1 h-5 w-px bg-line" aria-hidden="true" />
      <WindowControls />
    </header>
  );
}

export function WindowShell({ children }: { children: React.ReactNode }) {
  return (
    <div className="flex h-screen w-screen flex-col bg-ink-0 font-sans text-fg-0 selection:bg-accent selection:text-accent-fg">
      {children}
    </div>
  );
}
