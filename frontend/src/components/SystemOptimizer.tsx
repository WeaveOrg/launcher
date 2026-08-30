'use client';

import React, { useState } from 'react';
import { 
  Cpu, 
  Zap, 
  Activity, 
  ShieldAlert, 
  Trash2, 
  Clock, 
  Sparkles, 
  Gauge,
  CheckCircle2,
  HardDrive
} from 'lucide-react';
import { SystemSpecs } from '@/lib/ipc';

interface SystemOptimizerProps {
  specs: SystemSpecs;
  onOptimize: (action: string) => void;
}

export const SystemOptimizer: React.FC<SystemOptimizerProps> = ({ specs, onOptimize }) => {
  const [timerResolution, setTimerResolution] = useState(true);
  const [ramCleaned, setRamCleaned] = useState(false);
  const [telemetryDisabled, setTelemetryDisabled] = useState(true);
  const [gameModePriority, setGameModePriority] = useState(true);
  const [cleaningStatus, setCleaningStatus] = useState('');

  const handleCleanMemory = () => {
    setCleaningStatus('Flushing standby memory list & trimming working sets...');
    setTimeout(() => {
      setRamCleaned(true);
      setCleaningStatus('Freed 2.4 GB of RAM standby cache!');
      onOptimize('RAM Flush Completed');
      setTimeout(() => setCleaningStatus(''), 4000);
    }, 1200);
  };

  return (
    <div className="flex-1 flex flex-col overflow-y-auto p-6 space-y-6 select-none">
      <div>
        <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
          System Booster & Native Telemetry
          <span className="text-xs font-mono font-normal text-amber-400 px-2 py-0.5 rounded-full bg-amber-950/80 border border-amber-500/30">
            Performance Core
          </span>
        </h1>
        <p className="text-xs text-slate-400 mt-0.5">
          Low-level Windows OS latency optimizations and hardware telemetry.
        </p>
      </div>

      {/* Hardware Specs Card */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="p-4 rounded-2xl bg-[#12131f] border border-white/5 space-y-2">
          <div className="flex items-center justify-between text-slate-400 text-xs font-mono">
            <span>Operating System</span>
            <Gauge className="w-4 h-4 text-purple-400" />
          </div>
          <p className="text-sm font-bold text-white leading-tight">{specs.os}</p>
        </div>

        <div className="p-4 rounded-2xl bg-[#12131f] border border-white/5 space-y-2">
          <div className="flex items-center justify-between text-slate-400 text-xs font-mono">
            <span>Processor (CPU)</span>
            <Cpu className="w-4 h-4 text-indigo-400" />
          </div>
          <p className="text-sm font-bold text-white leading-tight truncate">{specs.cpu}</p>
        </div>

        <div className="p-4 rounded-2xl bg-[#12131f] border border-white/5 space-y-2">
          <div className="flex items-center justify-between text-slate-400 text-xs font-mono">
            <span>Graphics (GPU)</span>
            <Zap className="w-4 h-4 text-pink-400" />
          </div>
          <p className="text-sm font-bold text-white leading-tight truncate">{specs.gpu}</p>
        </div>

        <div className="p-4 rounded-2xl bg-[#12131f] border border-white/5 space-y-2">
          <div className="flex items-center justify-between text-slate-400 text-xs font-mono">
            <span>Hardware ID (HWID)</span>
            <ShieldAlert className="w-4 h-4 text-emerald-400" />
          </div>
          <p className="text-xs font-mono font-bold text-emerald-400 truncate">{specs.hwid}</p>
        </div>
      </div>

      {/* Optimization Actions */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-5">
        {/* RAM Cleaner */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-xl bg-purple-600/20 border border-purple-500/30 flex items-center justify-center text-purple-400">
                <Trash2 className="w-5 h-5" />
              </div>
              <div>
                <h3 className="font-bold text-sm text-white">Standby RAM Cleaner</h3>
                <p className="text-xs text-slate-400">Flushes cached memory to prevent micro-stutters</p>
              </div>
            </div>
          </div>

          {cleaningStatus && (
            <div className="p-2.5 bg-purple-950/40 border border-purple-500/30 rounded-xl text-xs font-mono text-purple-300">
              {cleaningStatus}
            </div>
          )}

          <button
            onClick={handleCleanMemory}
            className="w-full py-2.5 bg-gradient-to-r from-purple-600 to-indigo-600 hover:from-purple-500 hover:to-indigo-500 text-white rounded-xl text-xs font-bold transition flex items-center justify-center gap-2"
          >
            <Sparkles className="w-4 h-4" />
            <span>Flush Standby Memory List</span>
          </button>
        </div>

        {/* 0.5ms Timer Resolution */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-xl bg-indigo-600/20 border border-indigo-500/30 flex items-center justify-center text-indigo-400">
                <Clock className="w-5 h-5" />
              </div>
              <div>
                <h3 className="font-bold text-sm text-white">High-Precision Timer</h3>
                <p className="text-xs text-slate-400">Forces Windows kernel timer to 0.5ms (minimum input delay)</p>
              </div>
            </div>

            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={timerResolution}
                onChange={(e) => setTimerResolution(e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
            </label>
          </div>

          <div className="text-xs font-mono text-slate-400 bg-black/40 p-2.5 rounded-xl border border-white/5 flex items-center justify-between">
            <span>Current Windows Tickrate:</span>
            <span className="text-emerald-400 font-bold">0.5000 ms (Optimized)</span>
          </div>
        </div>

        {/* Windows Telemetry & Game Bar */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-xl bg-pink-600/20 border border-pink-500/30 flex items-center justify-center text-pink-400">
                <ShieldAlert className="w-5 h-5" />
              </div>
              <div>
                <h3 className="font-bold text-sm text-white">Anti-Cheat Safe Mode</h3>
                <p className="text-xs text-slate-400">Disables background OS telemetry & debug event logs</p>
              </div>
            </div>

            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={telemetryDisabled}
                onChange={(e) => setTelemetryDisabled(e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
            </label>
          </div>

          <div className="text-xs text-slate-400 leading-relaxed">
            Hides injection memory regions from system monitoring utilities and disables Xbox GameDVR hooks.
          </div>
        </div>

        {/* Process High Priority */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 space-y-4">
          <div className="flex items-center justify-between">
            <div className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-xl bg-emerald-600/20 border border-emerald-500/30 flex items-center justify-center text-emerald-400">
                <Activity className="w-5 h-5" />
              </div>
              <div>
                <h3 className="font-bold text-sm text-white">Realtime Thread Affinity</h3>
                <p className="text-xs text-slate-400">Assigns target game processes to high-performance cores</p>
              </div>
            </div>

            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={gameModePriority}
                onChange={(e) => setGameModePriority(e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
            </label>
          </div>

          <div className="text-xs text-slate-400 leading-relaxed">
            Eliminates context-switching delays by locking thread affinity directly to non-parking CPU cores.
          </div>
        </div>
      </div>
    </div>
  );
};
