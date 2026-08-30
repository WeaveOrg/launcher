'use client';

import React from 'react';
import { Minus, Square, X, ShieldCheck, Activity, Cpu } from 'lucide-react';
import { ipc, UserProfile } from '@/lib/ipc';

interface TitleBarProps {
  user: UserProfile | null;
  onlineCount: number;
  ping: number;
  onOpenAuth: () => void;
}

export const TitleBar: React.FC<TitleBarProps> = ({ user, onlineCount, ping, onOpenAuth }) => {
  return (
    <header
      onMouseDown={(e) => {
        if ((e.target as HTMLElement).closest('.no-drag')) return;
        ipc.startDrag();
      }}
      className="h-12 bg-[#121212] border-b border-[#333] flex items-center justify-between px-4 drag-region select-none relative z-50 cursor-grab active:cursor-grabbing"
    >
      {/* Brand & App Info */}
      <div className="flex items-center gap-3 no-drag">
        <div className="flex items-center gap-2">
          <div className="w-7 h-7 rounded bg-[#ff8c00] flex items-center justify-center shadow-lg shadow-orange-500/20">
            <ShieldCheck className="w-4 h-4 text-black" />
          </div>
          <span className="font-extrabold tracking-wider text-white text-sm">
            WEAVE <span className="text-xs text-[#ff8c00] font-mono font-medium px-1.5 py-0.5 rounded bg-orange-950/30 border border-[#ff8c00]/30">C++ CORE</span>
          </span>
        </div>

        {/* Live Network & Status */}
        <div className="hidden md:flex items-center gap-4 ml-6 pl-4 border-l border-[#333] text-xs text-slate-400">
          <div className="flex items-center gap-1.5">
            <span className="w-2 h-2 rounded-full bg-emerald-500 animate-pulse" />
            <span className="text-slate-300 font-mono">{onlineCount} Online</span>
          </div>
          <div className="flex items-center gap-1 text-slate-400 font-mono">
            <Activity className="w-3.5 h-3.5 text-[#ff8c00]" />
            <span>{ping}ms</span>
          </div>
        </div>
      </div>

      {/* Center Drag Title */}
      <div className="text-xs font-mono text-[#666] uppercase tracking-widest hidden lg:block">
        Saucer Native Engine • v2.0-STABLE
      </div>

      {/* User Info & Window Controls */}
      <div className="flex items-center gap-3 no-drag">
        {user ? (
          <div
            onClick={onOpenAuth}
            className="flex items-center gap-2.5 px-2.5 py-1 rounded bg-[#1a1a1a] border border-[#333] hover:border-[#ff8c00]/40 cursor-pointer transition"
          >
            <div className="w-5 h-5 rounded bg-[#ff8c00] flex items-center justify-center text-[10px] font-bold text-black uppercase">
              {user.username.charAt(0)}
            </div>
            <div className="flex flex-col text-left">
              <span className="text-xs font-medium text-slate-200">{user.username}</span>
              <span className="text-[9px] text-[#ff8c00] font-mono">{user.plan}</span>
            </div>
          </div>
        ) : (
          <button
            onClick={onOpenAuth}
            className="text-xs px-3 py-1 bg-orange-600/30 hover:bg-orange-600/50 border border-orange-500/40 text-orange-200 rounded-md font-medium transition"
          >
            Authenticate
          </button>
        )}

        {/* Window action buttons */}
        <div className="flex items-center gap-1 ml-2 pl-2 border-l border-white/10">
          <button
            onClick={() => ipc.minimize()}
            className="w-7 h-7 flex items-center justify-center text-slate-400 hover:text-white hover:bg-white/10 rounded transition"
            title="Minimize"
          >
            <Minus className="w-3.5 h-3.5" />
          </button>
          <button
            onClick={() => ipc.maximize()}
            className="w-7 h-7 flex items-center justify-center text-slate-400 hover:text-white hover:bg-white/10 rounded transition"
            title="Maximize"
          >
            <Square className="w-3 h-3" />
          </button>
          <button
            onClick={() => ipc.close()}
            className="w-7 h-7 flex items-center justify-center text-slate-400 hover:text-white hover:bg-red-600/80 rounded transition"
            title="Close"
          >
            <X className="w-3.5 h-3.5" />
          </button>
        </div>
      </div>
    </header>
  );
};
