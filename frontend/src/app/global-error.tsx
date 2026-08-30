'use client';

import React from 'react';
import { AlertCircle, RefreshCw } from 'lucide-react';

export default function GlobalError({
  error,
  reset,
}: {
  error: Error & { digest?: string };
  reset: () => void;
}) {
  return (
    <html lang="en">
      <body className="bg-[#0d0d0d] text-[#ddd] flex flex-col h-screen w-screen items-center justify-center p-6 font-sans select-none selection:bg-[#ff8c00] selection:text-black">
        <div className="max-w-md w-full bg-[#121212] border border-[#222] rounded-xl p-6 shadow-2xl flex flex-col gap-4 text-center items-center">
          <div className="w-12 h-12 rounded-xl bg-[#1a1a1a] border border-[#333] flex items-center justify-center text-[#ff8c00]">
            <AlertCircle className="w-6 h-6" />
          </div>
          
          <div className="space-y-1">
            <h1 className="text-base font-bold text-white uppercase tracking-wider">Critical Loader Error</h1>
            <p className="text-xs text-[#777] font-mono leading-relaxed">
              {error.message || 'The launcher encountered an unexpected fatal error.'}
            </p>
          </div>

          <button
            onClick={() => reset()}
            className="mt-2 flex items-center justify-center gap-2 bg-[#ff8c00] hover:bg-[#ffa02b] text-black font-extrabold text-xs tracking-wider uppercase py-2.5 px-6 rounded-full shadow-[0_0_15px_rgba(255,140,0,0.15)] transition active:scale-95"
          >
            <RefreshCw className="w-3.5 h-3.5" />
            Reload Interface
          </button>
        </div>
      </body>
    </html>
  );
}
