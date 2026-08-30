'use client';

import React, { useState, useEffect, useCallback } from 'react';
import { ShieldAlert, Loader2, X, Minus, KeyRound, RefreshCw, AlertCircle } from 'lucide-react';
import { ipc, LauncherProfile } from '@/lib/ipc';
import { motion } from 'framer-motion';

interface AuthScreenProps {
  initialToken?: string | null;
  onSuccess: (user: LauncherProfile, token: string) => void;
}

export const AuthScreen: React.FC<AuthScreenProps> = ({ initialToken, onSuccess }) => {
  const [error, setError] = useState<string>('');
  const [errorDetails, setErrorDetails] = useState<any>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [currentToken, setCurrentToken] = useState<string>('');

  const verifyToken = useCallback(async (targetToken: string) => {
    if (!targetToken) {
      setError('Session Not Found');
      setErrorDetails({
        message: 'No launcher authorization token detected.',
        hint: 'Please start the game through the official Weave launcher.'
      });
      setLoading(false);
      return;
    }

    try {
      setLoading(true);
      setError('');
      setErrorDetails(null);

      const res = await fetch(`/api/launcher/profile?token=${encodeURIComponent(targetToken)}`, {
        method: 'GET',
        headers: {
          'X-Launcher-Token': targetToken,
          'Accept': 'application/json'
        }
      });

      if (res.ok) {
        const profileData: LauncherProfile = await res.json();
        if (profileData && profileData.id) {
          if (typeof window !== 'undefined') {
            localStorage.setItem('launcher_token', targetToken);
          }
          onSuccess(profileData, targetToken);
          return;
        }
      }

      const errData = await res.json().catch(() => null);
      setError(errData?.error || 'Authentication Failed');
      setErrorDetails({
        status: res.status,
        message: errData?.message || errData?.details || 'Invalid or expired launcher session.',
        targetUrl: errData?.targetUrl || '/api/launcher/profile',
        hint: res.status === 401 
          ? 'Backend rejected the token. Make sure your account subscription is active.' 
          : res.status === 502 
          ? 'Cannot connect to backend server. Check server status or internet connection.' 
          : 'Backend returned an error status code.'
      });
    } catch (err: any) {
      setError('Connection Error');
      setErrorDetails({
        message: err?.message || 'Network error connecting to auth server.',
        hint: 'Ensure your server is online and backend API is reachable.'
      });
    } finally {
      setLoading(false);
    }
  }, [onSuccess]);

  useEffect(() => {
    const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
    const queryToken = urlParams?.get('token') || urlParams?.get('launcher_token');
    const token = initialToken || queryToken || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') : null);

    if (token) {
      setCurrentToken(token);
      verifyToken(token);
    } else {
      setError('Session Not Found');
      setErrorDetails({
        message: 'No launcher authorization token detected.',
        hint: 'Please start the game through the official Weave launcher.'
      });
      setLoading(false);
    }
  }, [initialToken, verifyToken]);

  return (
    <div className="flex flex-col h-screen w-screen bg-[#0d0d0d] text-[#ddd] font-sans selection:bg-[#ff8c00] selection:text-black">
      {/* DRAG HEADER */}
      <header
        onMouseDown={(e) => {
          if ((e.target as HTMLElement).closest('.no-drag')) return;
          ipc.startDrag();
        }}
        className="h-10 flex items-center justify-between px-4 drag-region select-none border-b border-[#1c1c1c]"
      >
        <div className="flex items-center gap-2 text-xs font-mono text-[#666]">
          <KeyRound className="w-3.5 h-3.5 text-[#ff8c00]" />
          <span>WEAVE LAUNCHER AUTH</span>
        </div>
        <div className="flex items-center gap-1 no-drag">
          <button
            onClick={() => ipc.minimize()}
            className="w-6 h-6 flex items-center justify-center text-[#666] hover:text-white hover:bg-[#222] rounded transition"
          >
            <Minus className="w-3.5 h-3.5" />
          </button>
          <button
            onClick={() => ipc.close()}
            className="w-6 h-6 flex items-center justify-center text-[#666] hover:text-white hover:bg-red-500/20 hover:text-red-400 rounded transition"
          >
            <X className="w-3.5 h-3.5" />
          </button>
        </div>
      </header>

      {/* MAIN AUTH / STATUS CENTER */}
      <main className="flex-1 flex items-center justify-center p-6 overflow-y-auto">
        <motion.div
          initial={{ opacity: 0, y: 10, scale: 0.95 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          className="w-full max-w-md flex flex-col items-center gap-5"
        >
          {loading ? (
            <div className="flex flex-col items-center gap-5 py-8">
              <Loader2 className="w-10 h-10 text-[#ff8c00] animate-spin" />
              <div className="text-center space-y-1">
                <h1 className="text-base font-bold text-white tracking-wider uppercase">Verifying Launcher Session</h1>
                <p className="text-xs text-[#666] font-mono">Checking authorization with backend...</p>
              </div>
            </div>
          ) : error ? (
            <motion.div
              initial={{ opacity: 0, scale: 0.9 }}
              animate={{ opacity: 1, scale: 1 }}
              className="w-full flex flex-col items-center gap-4 text-center"
            >
              {/* Alert Badge */}
              <div className="w-14 h-14 rounded-2xl bg-rose-500/10 border border-rose-500/30 flex items-center justify-center text-rose-500 shadow-[0_0_20px_rgba(244,63,94,0.15)]">
                <ShieldAlert className="w-7 h-7" />
              </div>

              <div className="space-y-1">
                <h1 className="text-lg font-black text-white tracking-wider uppercase">{error}</h1>
                {errorDetails?.message && (
                  <p className="text-xs text-rose-400 max-w-sm font-mono leading-relaxed">
                    {errorDetails.message}
                  </p>
                )}
              </div>

              {/* Error Details Card */}
              {errorDetails && (
                <div className="w-full bg-[#121212] border border-[#222] rounded-xl p-3.5 text-left text-xs font-mono space-y-2">
                  {errorDetails.status && (
                    <div className="flex items-center justify-between border-b border-[#222] pb-1.5">
                      <span className="text-[#666]">HTTP Status:</span>
                      <span className="font-bold text-rose-400 px-1.5 py-0.5 rounded bg-rose-950/60 border border-rose-500/30 text-[10px]">
                        {errorDetails.status}
                      </span>
                    </div>
                  )}
                  {errorDetails.hint && (
                    <div className="flex gap-2 text-[11px] text-[#888] leading-relaxed">
                      <AlertCircle className="w-3.5 h-3.5 text-amber-500 shrink-0 mt-0.5" />
                      <span>{errorDetails.hint}</span>
                    </div>
                  )}
                </div>
              )}

              {/* Action Buttons */}
              <div className="flex gap-2 mt-2 w-full">
                <button
                  onClick={() => {
                    const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
                    const qToken = urlParams?.get('token') || urlParams?.get('launcher_token');
                    const t = currentToken || initialToken || qToken || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') : null);
                    if (t) verifyToken(t);
                  }}
                  className="flex-1 py-2.5 bg-[#1a1a1a] hover:bg-[#252525] border border-[#333] text-white text-xs font-bold uppercase tracking-wider rounded-xl transition flex items-center justify-center gap-1.5 active:scale-95"
                >
                  <RefreshCw className="w-3 h-3 text-[#ff8c00]" />
                  Retry
                </button>
                <button
                  onClick={() => ipc.close()}
                  className="py-2.5 px-5 bg-rose-600/10 hover:bg-rose-600/20 border border-rose-500/30 text-rose-300 text-xs font-bold uppercase tracking-wider rounded-xl transition active:scale-95"
                >
                  Exit
                </button>
              </div>
            </motion.div>
          ) : null}
        </motion.div>
      </main>
    </div>
  );
};
