'use client';

import React, { useState } from 'react';
import { 
  FolderSync, 
  Download, 
  Upload, 
  Share2, 
  Check, 
  Sparkles, 
  FileCode,
  Copy
} from 'lucide-react';

interface ConfigItem {
  id: string;
  name: string;
  game: string;
  author: string;
  downloads: number;
  lastUpdated: string;
  isCustom?: boolean;
}

export const ConfigsView: React.FC = () => {
  const [copiedId, setCopiedId] = useState<string | null>(null);
  const [activeConfig, setActiveConfig] = useState('cfg_1');

  const configs: ConfigItem[] = [
    { id: 'cfg_1', name: 'Legit Semi-Rage Dynamic Curve', game: 'Counter-Strike 2', author: 's1mple_pro', downloads: 8410, lastUpdated: 'Today' },
    { id: 'cfg_2', name: 'Stream-Proof Minimalist Overlay', game: 'Apex Legends', author: 'ShroudClone', downloads: 5120, lastUpdated: 'Yesterday' },
    { id: 'cfg_3', name: 'Rust AK-47 Spray DSP Exact 100m', game: 'Rust Experimental', author: 'BeamMaster', downloads: 12050, lastUpdated: 'Aug 25' },
    { id: 'cfg_4', name: 'HvH Max DT & Rapid Fire 2026', game: 'Counter-Strike 2', author: 'NeverLoseGod', downloads: 9340, lastUpdated: 'Aug 20' },
  ];

  const handleCopyCode = (id: string) => {
    setCopiedId(id);
    setTimeout(() => setCopiedId(null), 2000);
  };

  return (
    <div className="flex-1 flex flex-col overflow-y-auto p-6 space-y-6 select-none">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
            Cloud Configurations
            <span className="text-xs font-mono font-normal text-purple-400 px-2 py-0.5 rounded-full bg-purple-950/80 border border-purple-500/30">
              Sync Hub
            </span>
          </h1>
          <p className="text-xs text-slate-400 mt-0.5">
            Share, import, and sync your favorite game presets with low-latency cloud synchronization.
          </p>
        </div>

        <button className="flex items-center gap-2 px-4 py-2 rounded-xl bg-purple-600 hover:bg-purple-500 text-white text-xs font-bold transition shadow-lg shadow-purple-600/20">
          <Upload className="w-3.5 h-3.5" />
          <span>Upload Custom Config</span>
        </button>
      </div>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        {configs.map((cfg) => {
          const isCurrent = activeConfig === cfg.id;
          return (
            <div
              key={cfg.id}
              className={`p-5 rounded-2xl bg-[#11121c] border transition-all duration-200 flex flex-col justify-between space-y-4 ${
                isCurrent
                  ? 'border-purple-500/50 bg-purple-950/10 shadow-lg shadow-purple-900/10'
                  : 'border-white/5 hover:border-purple-500/30'
              }`}
            >
              <div className="flex items-start justify-between">
                <div className="space-y-1">
                  <span className="text-[10px] font-mono font-bold text-purple-400 px-2 py-0.5 rounded bg-purple-950/50 border border-purple-500/20">
                    {cfg.game}
                  </span>
                  <h3 className="font-bold text-sm text-white pt-1">{cfg.name}</h3>
                  <p className="text-xs text-slate-400 font-mono">Author: {cfg.author} • {cfg.downloads} downloads</p>
                </div>

                <button
                  onClick={() => handleCopyCode(cfg.id)}
                  className="p-2 rounded-lg bg-white/5 hover:bg-white/10 text-slate-400 hover:text-white transition"
                  title="Share Config Code"
                >
                  {copiedId === cfg.id ? <Check className="w-4 h-4 text-emerald-400" /> : <Share2 className="w-4 h-4" />}
                </button>
              </div>

              <div className="flex items-center justify-between pt-2 border-t border-white/5">
                <span className="text-[11px] text-slate-500 font-mono">Updated {cfg.lastUpdated}</span>
                <button
                  onClick={() => setActiveConfig(cfg.id)}
                  className={`px-3 py-1.5 rounded-lg text-xs font-bold transition ${
                    isCurrent
                      ? 'bg-emerald-600/30 border border-emerald-500/40 text-emerald-300'
                      : 'bg-white/5 hover:bg-white/10 text-slate-200'
                  }`}
                >
                  {isCurrent ? 'ACTIVE PRESET' : 'LOAD TO MEMORY'}
                </button>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};
