'use client';

import React from 'react';
import { AlertOctagon, RefreshCw } from 'lucide-react';

export default function GlobalError({
  error,
  reset,
}: {
  error: Error & { digest?: string };
  reset: () => void;
}) {
  return (
    <html lang="en">
      <body className="bg-[#07080d] text-white flex flex-col h-screen w-screen items-center justify-center p-6 font-sans">
        <div className="max-w-md w-full bg-[#11121d] border border-rose-500/40 rounded-2xl p-6 shadow-2xl flex flex-col gap-4 text-center items-center">
          <div className="w-14 h-14 rounded-2xl bg-rose-500/10 border border-rose-500/30 flex items-center justify-center text-rose-400">
            <AlertOctagon className="w-7 h-7" />
          </div>
          
          <h1 className="text-xl font-extrabold text-white">Critical System Error</h1>
          <p className="text-xs text-rose-300/80 leading-relaxed font-mono">
            {error.message || 'A critical rendering error occurred.'}
          </p>

          <button
            onClick={() => reset()}
            className="mt-2 flex items-center justify-center gap-2 py-2.5 px-6 rounded-xl bg-orange-500 hover:bg-orange-400 text-black font-bold text-xs uppercase tracking-wider transition active:scale-95"
          >
            <RefreshCw className="w-4 h-4" />
            Reload Interface
          </button>
        </div>
      </body>
    </html>
  );
}
