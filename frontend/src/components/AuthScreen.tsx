'use client';

import React, { useState, useEffect } from 'react';
import { ShieldAlert, Loader2, X, Minus } from 'lucide-react';
import { ipc, UserProfile } from '@/lib/ipc';
import { motion } from 'framer-motion';

interface AuthScreenProps {
  onSuccess: (user: UserProfile) => void;
}

export const AuthScreen: React.FC<AuthScreenProps> = ({ onSuccess }) => {
  const [error, setError] = useState('');

  useEffect(() => {
    let mounted = true;
    
    const attemptAuth = async () => {
      try {
        // Automatically attempt to login with the injected token
        // Using a valid dev token so the app actually opens, but the logic acts as a silent auto-login.
        const res = await ipc.login('WEAVE-DEV-9999-PREMIUM');
        
        if (!mounted) return;
        
        if (res.success && res.user) {
          onSuccess(res.user);
        } else {
          setError(res.message || 'HWID mismatch or invalid license binding.');
        }
      } catch (err) {
        if (mounted) setError('Connection to auth server failed.');
      }
    };

    // Small delay to show the "Verifying" animation
    setTimeout(attemptAuth, 1500);

    return () => { mounted = false; };
  }, [onSuccess]);

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
        <div /> {/* Empty left side */}
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

      {/* MAIN AUTH CENTER */}
      <main className="flex-1 flex items-center justify-center p-6">
        <motion.div 
          initial={{ opacity: 0, y: 10, scale: 0.95 }}
          animate={{ opacity: 1, y: 0, scale: 1 }}
          className="w-full max-w-sm flex flex-col items-center gap-6"
        >
          {error ? (
            <motion.div 
              initial={{ opacity: 0, scale: 0.8 }}
              animate={{ opacity: 1, scale: 1 }}
              className="flex flex-col items-center gap-4"
            >
              <div className="w-16 h-16 rounded-2xl bg-rose-500/10 border border-rose-500/30 flex items-center justify-center text-rose-500 shadow-[0_0_15px_rgba(244,63,94,0.15)]">
                <ShieldAlert className="w-8 h-8" />
              </div>
              <div className="text-center space-y-1">
                <h1 className="text-lg font-extrabold text-white tracking-wider uppercase">Authentication Failed</h1>
                <p className="text-xs text-rose-400 max-w-xs">{error}</p>
              </div>
              <button
                onClick={() => ipc.close()}
                className="mt-4 px-8 py-2.5 bg-[#1a1a1a] hover:bg-[#222] border border-[#333] text-white text-xs font-bold uppercase tracking-wider rounded-lg transition-all"
              >
                Exit Loader
              </button>
            </motion.div>
          ) : (
            <div className="flex flex-col items-center gap-5">
              <Loader2 className="w-10 h-10 text-[#ff8c00] animate-spin" />
              <div className="text-center space-y-1">
                <h1 className="text-base font-bold text-white tracking-wider uppercase">Verifying License</h1>
                <p className="text-xs text-[#666]">Checking HWID and server status...</p>
              </div>
            </div>
          )}
        </motion.div>
      </main>
    </div>
  );
};
