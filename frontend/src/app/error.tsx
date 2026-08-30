'use client';

import React, { useEffect, useState } from 'react';
import { AlertCircle, RefreshCw, Terminal, ChevronDown, ChevronUp, Minus, X, RotateCcw } from 'lucide-react';
import { ipc } from '@/lib/ipc';
import { motion } from 'framer-motion';

export default function Error({
  error,
  reset,
}: {
  error: Error & { digest?: string };
  reset: () => void;
}) {
  const [showDetails, setShowDetails] = useState(false);
  const [tokenStatus, setTokenStatus] = useState<string>('Checking...');

  useEffect(() => {
    console.error('Weave Launcher Exception:', error);
    if (typeof window !== 'undefined') {
      const urlParams = new URLSearchParams(window.location.search);
      const token = urlParams.get('token') || localStorage.getItem('launcher_token');
      setTokenStatus(token ? `${token.substring(0, 10)}...` : 'Not detected');
    }
  }, [error]);

  return (
    <div className="flex flex-col h-screen w-screen bg-[#0d0d0d] text-[#ddd] font-sans selection:bg-[#ff8c00] selection:text-black">
      {/* UNIFIED MINIMAL HEADER */}
      <header
        onMouseDown={(e) => {
          if ((e.target as HTMLElement).closest('.no-drag')) return;
          ipc.startDrag();
        }}
        className="h-14 flex items-center justify-between px-4 drag-region select-none bg-[#121212] border-b border-[#222]"
      >
        <div className="flex items-center gap-3 no-drag">
          <div className="w-8 h-8 rounded-md bg-[#1a1a1a] border border-[#333] flex items-center justify-center overflow-hidden">
            <div className="w-3.5 h-3.5 rotate-45 bg-[#ff8c00] shadow-[0_0_10px_rgba(255,140,0,0.6)]" />
          </div>
          <div className="flex flex-col">
            <span className="font-bold text-white text-sm tracking-wide">Weave Launcher</span>
            <div className="flex items-center gap-1.5 text-[10px] font-mono text-[#888]">
              <span className="w-1.5 h-1.5 rounded-full bg-rose-500 shadow-[0_0_5px_#f43f5e]" />
              <span className="text-rose-400">INITIALIZATION ERROR</span>
            </div>
          </div>
        </div>

        {/* Window Controls */}
        <div className="flex items-center gap-1 no-drag pl-4">
          <button
            onClick={() => ipc.minimize()}
            className="w-7 h-7 flex items-center justify-center text-[#666] hover:text-white hover:bg-[#222] rounded transition"
            title="Minimize"
          >
            <Minus className="w-4 h-4" />
          </button>
          <button
            onClick={() => ipc.close()}
            className="w-7 h-7 flex items-center justify-center text-[#666] hover:text-white hover:bg-red-500/20 hover:text-red-400 rounded transition"
            title="Close"
          >
            <X className="w-4 h-4" />
          </button>
        </div>
      </header>

      {/* MAIN ERROR BODY */}
      <main className="flex-1 overflow-y-auto custom-scrollbar px-6 py-6 bg-[#0d0d0d] flex items-center justify-center">
        <motion.div
          initial={{ opacity: 0, y: 8, scale: 0.98 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          className="max-w-md w-full flex flex-col gap-4"
        >
          {/* Status Header */}
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-[#161616] border border-[#262626] flex items-center justify-center text-[#ff8c00] shrink-0">
              <AlertCircle className="w-5 h-5" />
            </div>
            <div>
              <h2 className="text-base font-bold text-white tracking-wide">Interface Error</h2>
              <p className="text-xs text-[#777]">The launcher encountered an unexpected issue while loading.</p>
            </div>
          </div>

          {/* Diagnostic Box */}
          <div className="bg-[#121212] border border-[#222] rounded-lg p-3.5 shadow-sm space-y-2 text-xs font-mono">
            <div className="flex items-center justify-between border-b border-[#1f1f1f] pb-2">
              <span className="text-[#666] uppercase text-[10px] tracking-wider">Exception Details</span>
              <span className="text-[10px] text-rose-400 bg-rose-950/40 border border-rose-500/20 px-1.5 py-0.5 rounded">
                CRITICAL
              </span>
            </div>

            <div className="space-y-1.5 pt-1 text-[11px]">
              <div className="flex flex-col gap-0.5">
                <span className="text-[#555] text-[10px]">REASON</span>
                <span className="text-rose-300 font-semibold break-words leading-snug">
                  {error.message || 'An unhandled exception occurred.'}
                </span>
              </div>

              <div className="flex justify-between items-center pt-1 border-t border-[#1a1a1a]">
                <span className="text-[#555] text-[10px]">SESSION TOKEN</span>
                <span className="text-[#aaa]">{tokenStatus}</span>
              </div>
            </div>
          </div>

          {/* Collapsible Tech Stack */}
          {error.stack && (
            <div className="border border-[#222] rounded-lg overflow-hidden bg-[#121212]/50">
              <button
                onClick={() => setShowDetails(!showDetails)}
                className="w-full flex items-center justify-between p-2.5 text-xs text-[#666] hover:text-[#aaa] transition"
              >
                <span className="flex items-center gap-1.5 font-mono text-[10px] tracking-wide">
                  <Terminal className="w-3.5 h-3.5 text-[#ff8c00]" />
                  TECHNICAL STACK TRACE
                </span>
                {showDetails ? <ChevronUp className="w-3.5 h-3.5" /> : <ChevronDown className="w-3.5 h-3.5" />}
              </button>

              {showDetails && (
                <div className="p-3 pt-0 max-h-36 overflow-y-auto font-mono text-[10px] text-[#777] whitespace-pre-wrap select-text border-t border-[#1a1a1a]">
                  {error.stack}
                </div>
              )}
            </div>
          )}

          {/* Action Buttons */}
          <div className="flex items-center gap-2 pt-2">
            <button
              onClick={() => reset()}
              className="flex-1 flex items-center justify-center gap-2 bg-[#ff8c00] hover:bg-[#ffa02b] text-black font-extrabold text-xs tracking-wider uppercase py-2.5 px-5 rounded-full shadow-[0_0_15px_rgba(255,140,0,0.15)] transition-all active:scale-95"
            >
              <RefreshCw className="w-3.5 h-3.5 fill-black" />
              Try Again
            </button>

            <button
              onClick={() => {
                if (typeof window !== 'undefined') {
                  localStorage.removeItem('launcher_token');
                  window.location.reload();
                }
              }}
              className="flex items-center justify-center gap-1.5 bg-[#161616] hover:bg-[#202020] border border-[#2a2a2a] text-[#aaa] hover:text-white font-bold text-xs uppercase tracking-wider py-2.5 px-4 rounded-full transition active:scale-95"
            >
              <RotateCcw className="w-3.5 h-3.5" />
              Reset
            </button>
          </div>
        </motion.div>
      </main>
    </div>
  );
}
