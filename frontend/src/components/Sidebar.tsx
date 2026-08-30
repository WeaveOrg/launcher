'use client';

import React from 'react';
import { 
  Gamepad2, 
  Cpu, 
  Settings, 
  Newspaper, 
  FolderSync, 
  Sparkles,
  Terminal,
  Layers
} from 'lucide-react';

export type TabType = 'library' | 'configs' | 'optimizer' | 'news' | 'console' | 'settings';

interface SidebarProps {
  activeTab: TabType;
  onSelectTab: (tab: TabType) => void;
}

export const Sidebar: React.FC<SidebarProps> = ({ activeTab, onSelectTab }) => {
  const navItems = [
    { id: 'library' as TabType, label: 'Game Library', icon: Gamepad2, badge: '6' },
    { id: 'configs' as TabType, label: 'Cloud Configs', icon: FolderSync },
    { id: 'optimizer' as TabType, label: 'System Boost', icon: Cpu, highlight: true },
    { id: 'news' as TabType, label: 'Updates & News', icon: Newspaper },
    { id: 'console' as TabType, label: 'Native Logs', icon: Terminal },
    { id: 'settings' as TabType, label: 'Settings', icon: Settings },
  ];

  return (
    <aside className="w-64 bg-[#0d0e17]/80 border-r border-purple-900/20 flex flex-col justify-between p-3 select-none">
      <div className="space-y-1">
        <div className="px-3 py-2 text-[10px] font-mono tracking-widest text-slate-500 uppercase">
          Navigation
        </div>

        {navItems.map((item) => {
          const Icon = item.icon;
          const isActive = activeTab === item.id;
          return (
            <button
              key={item.id}
              onClick={() => onSelectTab(item.id)}
              className={`w-full flex items-center justify-between px-3.5 py-2.5 rounded-xl font-medium text-xs transition-all duration-200 ${
                isActive
                  ? 'bg-gradient-to-r from-purple-600/30 to-indigo-600/20 text-white border border-purple-500/40 shadow-lg shadow-purple-500/10'
                  : 'text-slate-400 hover:text-slate-200 hover:bg-white/5 border border-transparent'
              }`}
            >
              <div className="flex items-center gap-3">
                <Icon className={`w-4 h-4 ${isActive ? 'text-purple-400' : 'text-slate-400'}`} />
                <span>{item.label}</span>
              </div>
              {item.badge && (
                <span className="text-[10px] font-mono font-bold px-1.5 py-0.5 rounded-md bg-purple-900/50 text-purple-300 border border-purple-500/30">
                  {item.badge}
                </span>
              )}
              {item.highlight && (
                <Sparkles className="w-3.5 h-3.5 text-amber-400 animate-pulse" />
              )}
            </button>
          );
        })}
      </div>

      {/* Engine Status Footer Card */}
      <div className="p-3 rounded-xl bg-purple-950/20 border border-purple-500/20 text-xs">
        <div className="flex items-center justify-between mb-1.5">
          <span className="text-slate-300 font-medium flex items-center gap-1.5">
            <Layers className="w-3.5 h-3.5 text-purple-400" />
            Saucer Driver
          </span>
          <span className="text-[10px] font-mono text-emerald-400 bg-emerald-950/60 px-1.5 py-0.5 rounded border border-emerald-500/30">
            ACTIVE
          </span>
        </div>
        <p className="text-[11px] text-slate-400 leading-relaxed font-mono">
          C++23 Webview Native Bridge running with Direct3D11 acceleration.
        </p>
      </div>
    </aside>
  );
};
