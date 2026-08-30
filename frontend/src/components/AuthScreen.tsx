'use client';

import React, { useState, useEffect } from 'react';
import { ShieldAlert, Loader2, X, Minus, KeyRound } from 'lucide-react';
import { ipc, LauncherProfile } from '@/lib/ipc';
import { motion } from 'framer-motion';

interface AuthScreenProps {
  initialToken?: string | null;
  onSuccess: (user: LauncherProfile, token: string) => void;
}

export const AuthScreen: React.FC<AuthScreenProps> = ({ initialToken, onSuccess }) => {
  const [error, setError] = useState<string>('');
  const [loading, setLoading] = useState<boolean>(true);

  useEffect(() => {
    let mounted = true;

    const verifyToken = async () => {
      // 1. Get token from URL query params or localStorage
      const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
      const queryToken = urlParams?.get('token');
      const token = initialToken || queryToken || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') : null);

      if (!token) {
        if (mounted) {
          setError('No launcher token provided. Please run the launcher with ?token=<launcher_token>');
          setLoading(false);
        }
        return;
      }

      try {
        setLoading(true);
        // Call Next.js server proxy GET /api/launcher/profile?token=... with X-Launcher-Token
        const res = await fetch(`/api/launcher/profile?token=${encodeURIComponent(token)}`, {
          method: 'GET',
          headers: {
            'X-Launcher-Token': token,
            'Accept': 'application/json'
          }
        });

        if (!mounted) return;

        if (res.ok) {
          const profileData: LauncherProfile = await res.json();
          if (profileData && profileData.id) {
            if (typeof window !== 'undefined') {
              localStorage.setItem('launcher_token', token);
            }
            onSuccess(profileData, token);
            return;
          }
        }

        const errData = await res.json().catch(() => null);
        setError(errData?.message || errData?.error || 'Invalid or expired launcher token.');
      } catch (err: any) {
        if (mounted) {
          setError('Failed to reach backend verification service.');
        }
      } finally {
        if (mounted) setLoading(false);
      }
    };

    verifyToken();

    return () => {
      mounted = false;
    };
  }, [initialToken, onSuccess]);

  return (
    <div className="flex flex-col h-screen w-screen bg-[#0d0d0d] text-[#ddd] font-sans selection:bg-[#ff8c00] selection:text-black">
      {/* DRAG HEADER */}
      <header
        onMouseDown={(e) => {
          if ((e.target as HTMLElement).closest('.no-drag')) return;
          ipc.startDrag();
        }}
        className="h-10 flex items-center justify-between px-4 drag-region select-none"
      >
        <div className="flex items-center gap-2 text-xs font-mono text-[#666]">
          <KeyRound className="w-3.5 h-3.5 text-[#ff8c00]" />
          <span>WEAVE SESSION AUTH</span>
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

      {/* MAIN AUTH / ERROR CENTER */}
      <main className="flex-1 flex items-center justify-center p-6">
        <motion.div
          initial={{ opacity: 0, y: 10, scale: 0.95 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          className="w-full max-w-sm flex flex-col items-center gap-6"
        >
          {loading ? (
            <div className="flex flex-col items-center gap-5">
              <Loader2 className="w-10 h-10 text-[#ff8c00] animate-spin" />
              <div className="text-center space-y-1">
                <h1 className="text-base font-bold text-white tracking-wider uppercase">Verifying Token</h1>
                <p className="text-xs text-[#666]">Checking session authorization via Next.js Proxy...</p>
              </div>
            </div>
          ) : error ? (
            <motion.div
              initial={{ opacity: 0, scale: 0.85 }}
              animate={{ opacity: 1, scale: 1 }}
              className="flex flex-col items-center gap-4 text-center"
            >
              <div className="w-16 h-16 rounded-2xl bg-rose-500/10 border border-rose-500/30 flex items-center justify-center text-rose-500 shadow-[0_0_15px_rgba(244,63,94,0.15)]">
                <ShieldAlert className="w-8 h-8" />
              </div>
              <div className="space-y-1">
                <h1 className="text-lg font-extrabold text-white tracking-wider uppercase">Invalid Token</h1>
                <p className="text-xs text-rose-400 max-w-xs leading-relaxed">{error}</p>
              </div>
              <div className="flex gap-2 mt-2">
                <button
                  onClick={() => {
                    if (typeof window !== 'undefined') {
                      localStorage.removeItem('launcher_token');
                      window.location.reload();
                    }
                  }}
                  className="px-4 py-2 bg-[#1a1a1a] hover:bg-[#252525] border border-[#333] text-white text-xs font-bold uppercase tracking-wider rounded-lg transition-all"
                >
                  Retry
                </button>
                <button
                  onClick={() => ipc.close()}
                  className="px-4 py-2 bg-rose-600/20 hover:bg-rose-600/30 border border-rose-500/40 text-rose-300 text-xs font-bold uppercase tracking-wider rounded-lg transition-all"
                >
                  Exit Loader
                </button>
              </div>
            </motion.div>
          ) : null}
        </motion.div>
      </main>
    </div>
  );
};
