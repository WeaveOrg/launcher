'use client';

import React from 'react';
import { Terminal, Trash2, Copy, Shield, Cpu } from 'lucide-react';

interface ConsoleViewProps {
  logs: string[];
  onClear: () => void;
}

export const ConsoleView: React.FC<ConsoleViewProps> = ({ logs, onClear }) => {
  return (
    <div className="flex-1 flex flex-col overflow-hidden p-6 space-y-4 select-none">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
            Native Debug Console
            <span className="text-xs font-mono font-normal text-emerald-400 px-2 py-0.5 rounded-full bg-emerald-950/80 border border-emerald-500/30">
              Direct Pipe
            </span>
          </h1>
          <p className="text-xs text-slate-400 mt-0.5">
            Real-time messages from Saucer C++ Smartview, Windows Kernel Hook, and Loader events.
          </p>
        </div>

        <button
          onClick={onClear}
          className="flex items-center gap-1.5 px-3 py-1.5 rounded-xl bg-white/5 hover:bg-white/10 text-slate-300 text-xs font-medium border border-white/10 transition"
        >
          <Trash2 className="w-3.5 h-3.5" />
          <span>Clear Logs</span>
        </button>
      </div>

      <div className="flex-1 bg-[#090a10] border border-purple-500/20 rounded-2xl p-4 font-mono text-xs text-slate-300 overflow-y-auto space-y-1.5 shadow-inner">
        {logs.length === 0 ? (
          <div className="h-full flex items-center justify-center text-slate-600">
            No native log entries recorded yet.
          </div>
        ) : (
          logs.map((log, i) => (
            <div key={i} className="leading-relaxed">
              {log.includes('SUCCESS') ? (
                <span className="text-emerald-400 font-semibold">{log}</span>
              ) : log.includes('ERROR') ? (
                <span className="text-rose-400 font-semibold">{log}</span>
              ) : log.includes('Weave') ? (
                <span className="text-purple-300">{log}</span>
              ) : (
                <span className="text-slate-400">{log}</span>
              )}
            </div>
          ))
        )}
      </div>
    </div>
  );
};
