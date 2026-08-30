'use client';

import React, { useEffect, useState } from 'react';
import { AlertTriangle, RefreshCw, Terminal, ChevronDown, ChevronUp, WifiOff } from 'lucide-react';
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
  const [currentUrl, setCurrentUrl] = useState<string>('');

  useEffect(() => {
    console.error('Next.js Page Error caught by Error Boundary:', error);
    if (typeof window !== 'undefined') {
      setCurrentUrl(window.location.href);
      const urlParams = new URLSearchParams(window.location.search);
      const token = urlParams.get('token') || localStorage.getItem('launcher_token');
      setTokenStatus(token ? `Token found (${token.substring(0, 8)}...)` : 'Missing (?token=... was not passed)');
    }
  }, [error]);

  return (
    <div className="flex flex-col h-screen w-screen bg-[#0a0a0f] text-slate-100 font-sans p-6 items-center justify-center select-none overflow-y-auto">
      <motion.div 
        initial={{ opacity: 0, scale: 0.95, y: 15 }}
        animate={{ opacity: 1, scale: 1, y: 0 }}
        className="w-full max-w-lg bg-[#12131e] border border-rose-500/30 rounded-2xl p-6 shadow-[0_0_30px_rgba(244,63,94,0.1)] flex flex-col gap-5"
      >
        {/* Header with Icon */}
        <div className="flex items-center gap-4">
          <div className="w-12 h-12 rounded-xl bg-rose-500/10 border border-rose-500/30 flex items-center justify-center text-rose-400 shrink-0 shadow-[0_0_15px_rgba(244,63,94,0.2)]">
            <AlertTriangle className="w-6 h-6" />
          </div>
          <div>
            <h1 className="text-lg font-bold text-white tracking-wide">
              Failed to Load Launcher Page
            </h1>
            <p className="text-xs text-rose-300/80">
              An error occurred during page rendering or initialization.
            </p>
          </div>
        </div>

        {/* Diagnostic Card */}
        <div className="bg-[#0b0c14] border border-white/5 rounded-xl p-3.5 space-y-2 text-xs font-mono">
          <div className="flex items-center justify-between text-slate-400 border-b border-white/5 pb-2">
            <span>Diagnostics</span>
            <span className="text-[10px] px-1.5 py-0.5 rounded bg-rose-950/80 text-rose-400 border border-rose-500/20">ERROR</span>
          </div>

          <div className="grid grid-cols-3 gap-1 text-[11px]">
            <span className="text-slate-500">Reason:</span>
            <span className="col-span-2 text-rose-300 font-bold truncate">{error.message || 'Unknown runtime exception'}</span>
            
            <span className="text-slate-500">Token Status:</span>
            <span className="col-span-2 text-slate-300">{tokenStatus}</span>

            {error.digest && (
              <>
                <span className="text-slate-500">Error Digest:</span>
                <span className="col-span-2 text-slate-400">{error.digest}</span>
              </>
            )}
          </div>
        </div>

        {/* Stack Trace Collapsible */}
        {error.stack && (
          <div className="border border-white/5 rounded-xl overflow-hidden bg-[#07080d]">
            <button
              onClick={() => setShowDetails(!showDetails)}
              className="w-full flex items-center justify-between p-3 text-xs text-slate-400 hover:text-slate-200 transition"
            >
              <span className="flex items-center gap-2 font-mono text-[11px]">
                <Terminal className="w-3.5 h-3.5 text-indigo-400" />
                Technical Error Stack
              </span>
              {showDetails ? <ChevronUp className="w-4 h-4" /> : <ChevronDown className="w-4 h-4" />}
            </button>

            {showDetails && (
              <div className="p-3 pt-0 max-h-40 overflow-y-auto font-mono text-[10px] text-slate-500 whitespace-pre-wrap select-text border-t border-white/5">
                {error.stack}
              </div>
            )}
          </div>
        )}

        {/* Action Buttons */}
        <div className="flex items-center gap-3 pt-2">
          <button
            onClick={() => reset()}
            className="flex-1 flex items-center justify-center gap-2 py-2.5 px-4 rounded-xl bg-gradient-to-r from-orange-500 to-amber-600 hover:from-orange-400 hover:to-amber-500 text-black font-bold text-xs uppercase tracking-wider transition-all shadow-[0_0_15px_rgba(249,115,22,0.2)] active:scale-95"
          >
            <RefreshCw className="w-3.5 h-3.5" />
            Try Again
          </button>

          <button
            onClick={() => {
              if (typeof window !== 'undefined') {
                localStorage.removeItem('launcher_token');
                window.location.reload();
              }
            }}
            className="py-2.5 px-4 rounded-xl bg-[#1a1b29] hover:bg-[#232438] border border-white/10 text-slate-300 text-xs font-semibold tracking-wider transition-all"
          >
            Reset Session & Reload
          </button>
        </div>
      </motion.div>
    </div>
  );
}
