'use client';

import React from 'react';
import { ipc, AppItem, LauncherProfile, ChangelogItem } from '../lib/ipc';
import { Play, Minus, X, CheckCircle2, User, Sparkles } from 'lucide-react';

interface LauncherRedesignProps {
  apps: AppItem[];
  user: LauncherProfile;
  token: string;
  changelogs: ChangelogItem[];
  onlineCount: number;
  ping: number;
  onLaunch: (app: AppItem) => void;
  onLogout: () => void;
}

export const LauncherRedesign: React.FC<LauncherRedesignProps> = ({
  apps,
  user,
  token,
  changelogs,
  onLaunch,
  onLogout
}) => {
  const safeApps = Array.isArray(apps) ? apps : [];
  const safeChangelogs = Array.isArray(changelogs) ? changelogs : [];

  // CS2 is the primary app
  const cs2App = safeApps.find(a => a?.name?.includes('Counter') || a?.name?.includes('CS')) || ({
    id: 'cs2',
    name: 'Counter-Strike 2',
    processName: 'cs2.exe',
    status: 'Undetected',
    version: safeChangelogs.length > 0 ? safeChangelogs[0].version : 'v2.1.4'
  } as AppItem);

  return (
    <div className="flex flex-col h-screen w-screen bg-[#0d0d0d] text-[#ddd] font-sans selection:bg-[#ff8c00] selection:text-black">
      
      {/* UNIFIED MINIMAL HEADER (TitleBar + Game Info + User Profile) */}
      <header
        onMouseDown={(e) => {
          if ((e.target as HTMLElement).closest('.no-drag')) return;
          ipc.startDrag();
        }}
        className="h-14 flex items-center justify-between px-4 drag-region select-none bg-[#121212] border-b border-[#222]"
      >
        {/* Left: Game Info */}
        <div className="flex items-center gap-3 no-drag">
          <div className="w-8 h-8 rounded-md bg-[#1a1a1a] border border-[#333] flex items-center justify-center overflow-hidden">
            <img 
              src="https://cdn.akamai.steamstatic.com/steam/apps/730/header.jpg" 
              alt="CS2"
              className="w-full h-full object-cover"
              onError={(e) => { e.currentTarget.style.display = 'none'; }}
            />
          </div>
          <div className="flex flex-col">
            <span className="font-bold text-white text-sm tracking-wide">{cs2App.name}</span>
            <div className="flex items-center gap-1.5 text-[10px] font-mono text-[#888]">
              <span className="w-1.5 h-1.5 rounded-full bg-emerald-500 shadow-[0_0_5px_#10b981]" />
              <span className="text-emerald-400">UNDETECTED</span>
              <span className="mx-1">•</span>
              <span>{cs2App.version || 'v2.1.4'}</span>
            </div>
          </div>
        </div>

        {/* Right: User Profile & Window Controls */}
        <div className="flex items-center gap-3 no-drag">
          {/* User Profile Pill */}
          <div className="flex items-center gap-2 px-2.5 py-1 rounded-full bg-[#181818] border border-[#2a2a2a]">
            {user.avatar ? (
              <img 
                src={user.avatar} 
                alt={user.username} 
                className="w-5 h-5 rounded-full object-cover border border-[#444]"
                onError={(e) => { e.currentTarget.style.display = 'none'; }}
              />
            ) : (
              <div className="w-5 h-5 rounded-full bg-[#252525] flex items-center justify-center text-[#888]">
                <User className="w-3 h-3" />
              </div>
            )}
            <span className="text-xs font-semibold text-white tracking-tight">{user.username}</span>
          </div>

          {/* Window Controls */}
          <div className="flex items-center gap-1 pl-2 border-l border-[#222]">
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
        </div>
      </header>

      {/* MAIN CONTENT: Minimal Changelog */}
      <main className="flex-1 overflow-y-auto custom-scrollbar px-6 py-4 bg-[#0d0d0d] flex justify-center">
        <div className="max-w-3xl w-full flex flex-col gap-3">
          
          <div className="flex items-center justify-between mb-1">
            <div className="flex items-center gap-2">
              <CheckCircle2 className="w-4 h-4 text-[#ff8c00]" />
              <h2 className="text-base font-bold text-white tracking-wide">Changelog</h2>
            </div>
            <div className="text-[10px] uppercase font-mono tracking-wider text-[#555] flex items-center gap-2">
              <span className="w-1.5 h-1.5 rounded-full bg-emerald-500/50"></span>
              All systems operational
            </div>
          </div>

          <div className="flex flex-col gap-4 relative before:absolute before:inset-y-0 before:left-[11px] before:w-[2px] before:bg-[#222]">
            {safeChangelogs.length > 0 ? (
              safeChangelogs.map((item, idx) => (
                <div key={item.id || idx} className="relative pl-8">
                  <div className={`absolute left-[6px] top-[6px] w-3 h-3 rotate-45 z-10 ${
                    idx === 0 
                      ? 'bg-[#ff8c00] shadow-[0_0_10px_rgba(255,140,0,0.6)]' 
                      : 'bg-[#121212] border-2 border-[#444]'
                  }`} />
                  <div className="flex items-center gap-3 mb-1">
                    <span className={`font-mono text-sm font-bold ${idx === 0 ? 'text-white' : 'text-[#888]'}`}>
                      {item.version || `Update #${idx + 1}`}
                    </span>
                    <span className="text-xs text-[#666]">
                      {item.created_at ? new Date(item.created_at).toLocaleDateString('en-US', { month: 'long', day: 'numeric', year: 'numeric' }) : 'Recent'}
                    </span>
                    {item.title && (
                      <span className="text-xs font-semibold text-[#aaa]">• {item.title}</span>
                    )}
                  </div>
                  <div className={`border rounded-lg p-3.5 shadow-sm ${
                    idx === 0 ? 'bg-[#121212] border-[#222]' : 'bg-[#121212]/50 border-[#1a1a1a]'
                  }`}>
                    <div className="text-xs text-[#aaa] whitespace-pre-line leading-relaxed">
                      {item.content}
                    </div>
                  </div>
                </div>
              ))
            ) : (
              // Fallback static updates when backend changelogs list is empty
              <>
                <div className="relative pl-8">
                  <div className="absolute left-[6px] top-[6px] w-3 h-3 rotate-45 bg-[#ff8c00] shadow-[0_0_10px_rgba(255,140,0,0.6)] z-10" />
                  <div className="flex items-center gap-3 mb-1">
                    <span className="font-mono text-sm font-bold text-white">v2.1.4</span>
                    <span className="text-xs text-[#666]">August 25, 2026</span>
                  </div>
                  <div className="bg-[#121212] border border-[#222] rounded-lg p-3.5 shadow-sm">
                    <ul className="list-disc pl-4 text-xs text-[#aaa] space-y-1.5 marker:text-[#444]">
                      <li><strong className="text-[#ccc]">Security:</strong> Improved injection stealth and stability for the latest game patch.</li>
                      <li><strong className="text-[#ccc]">Engine:</strong> Added kernel-level memory mapping to bypass anti-cheat checks.</li>
                      <li><strong className="text-[#ccc]">Fix:</strong> Resolved crash during alt-tab in exclusive fullscreen.</li>
                    </ul>
                  </div>
                </div>

                <div className="relative pl-8">
                  <div className="absolute left-[6px] top-[6px] w-3 h-3 rotate-45 bg-[#121212] border-2 border-[#444] z-10" />
                  <div className="flex items-center gap-3 mb-1">
                    <span className="font-mono text-sm font-bold text-[#888]">v2.1.3</span>
                    <span className="text-xs text-[#555]">August 10, 2026</span>
                  </div>
                  <div className="bg-[#121212]/50 border border-[#1a1a1a] rounded-lg p-3.5">
                    <ul className="list-disc pl-4 text-xs text-[#777] space-y-1.5 marker:text-[#333]">
                      <li>Updated internal offsets for the latest patch.</li>
                      <li>Fixed cloud profile synchronization issue.</li>
                    </ul>
                  </div>
                </div>
              </>
            )}
          </div>
        </div>
      </main>

      {/* BOTTOM ACTION BAR */}
      <footer className="h-20 bg-[#121212] border-t border-[#222] flex items-center justify-between px-8">
        <div className="flex flex-col">
          <span className="text-sm font-bold text-[#ccc]">Ready to inject</span>
          <span className="text-[11px] font-mono text-[#666]">Authorized as {user.username} (ID: {user.id.substring(0, 8)}...)</span>
        </div>
        
        <button
          onClick={() => {
            if (cs2App) onLaunch(cs2App);
          }}
          className="flex items-center gap-2 bg-[#ff8c00] hover:bg-[#ffa02b] text-black font-extrabold text-xs tracking-wider uppercase py-2.5 px-7 rounded-full shadow-[0_0_15px_rgba(255,140,0,0.15)] transition-all active:scale-95"
        >
          <Play className="w-3.5 h-3.5 fill-black" />
          Launch
        </button>
      </footer>
    </div>
  );
};
