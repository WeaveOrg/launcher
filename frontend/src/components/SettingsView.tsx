'use client';

import React, { useState } from 'react';
import { Settings, Shield, Bell, Disc as Discord, EyeOff, Save, Check } from 'lucide-react';

export const SettingsView: React.FC = () => {
  const [discordRpc, setDiscordRpc] = useState(true);
  const [streamProof, setStreamProof] = useState(true);
  const [autoStart, setAutoStart] = useState(false);
  const [closeToTray, setCloseToTray] = useState(true);
  const [overlayKeybind, setOverlayKeybind] = useState('INSERT');
  const [saved, setSaved] = useState(false);

  const handleSave = () => {
    setSaved(true);
    setTimeout(() => setSaved(false), 2000);
  };

  return (
    <div className="flex-1 flex flex-col overflow-y-auto p-6 space-y-6 select-none">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
            Launcher Settings
            <span className="text-xs font-mono font-normal text-purple-400 px-2 py-0.5 rounded-full bg-purple-950/80 border border-purple-500/30">
              Preferences
            </span>
          </h1>
          <p className="text-xs text-slate-400 mt-0.5">
            Configure Saucer runtime parameters, hotkeys, and stealth overlay modes.
          </p>
        </div>

        <button
          onClick={handleSave}
          className="flex items-center gap-2 px-4 py-2 rounded-xl bg-purple-600 hover:bg-purple-500 text-white text-xs font-bold transition shadow-lg shadow-purple-600/20"
        >
          {saved ? <Check className="w-3.5 h-3.5 text-white" /> : <Save className="w-3.5 h-3.5" />}
          <span>{saved ? 'Saved!' : 'Save Preferences'}</span>
        </button>
      </div>

      <div className="space-y-4 max-w-3xl">
        {/* Stream Proof Mode */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 flex items-center justify-between">
          <div className="space-y-1">
            <div className="flex items-center gap-2">
              <EyeOff className="w-4 h-4 text-purple-400" />
              <h3 className="text-sm font-bold text-white">Stream-Proof Overlay (OBS / Discord)</h3>
            </div>
            <p className="text-xs text-slate-400">
              Uses DirectX 11 presentation flags to ensure injected menus are completely invisible on screen recordings.
            </p>
          </div>

          <label className="relative inline-flex items-center cursor-pointer">
            <input
              type="checkbox"
              checked={streamProof}
              onChange={(e) => setStreamProof(e.target.checked)}
              className="sr-only peer"
            />
            <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
          </label>
        </div>

        {/* Discord Rich Presence */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 flex items-center justify-between">
          <div className="space-y-1">
            <div className="flex items-center gap-2">
              <Discord className="w-4 h-4 text-indigo-400" />
              <h3 className="text-sm font-bold text-white">Discord Rich Presence</h3>
            </div>
            <p className="text-xs text-slate-400">
              Display active game and playtime in your Discord status.
            </p>
          </div>

          <label className="relative inline-flex items-center cursor-pointer">
            <input
              type="checkbox"
              checked={discordRpc}
              onChange={(e) => setDiscordRpc(e.target.checked)}
              className="sr-only peer"
            />
            <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
          </label>
        </div>

        {/* In-Game Menu Hotkey */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 flex items-center justify-between">
          <div className="space-y-1">
            <h3 className="text-sm font-bold text-white">In-Game Menu Toggle Hotkey</h3>
            <p className="text-xs text-slate-400">
              The keyboard button pressed to toggle in-game interface menu.
            </p>
          </div>

          <input
            type="text"
            value={overlayKeybind}
            onChange={(e) => setOverlayKeybind(e.target.value.toUpperCase())}
            className="w-28 bg-[#181926] border border-purple-500/40 rounded-xl px-3 py-1.5 text-center text-xs font-mono font-bold text-purple-300 focus:outline-none focus:border-purple-400"
          />
        </div>

        {/* Close to system tray */}
        <div className="p-5 rounded-2xl bg-[#11121c] border border-purple-500/20 flex items-center justify-between">
          <div className="space-y-1">
            <h3 className="text-sm font-bold text-white">Minimize to System Tray on Exit</h3>
            <p className="text-xs text-slate-400">
              Keep the C++ native background driver running silently in notification tray.
            </p>
          </div>

          <label className="relative inline-flex items-center cursor-pointer">
            <input
              type="checkbox"
              checked={closeToTray}
              onChange={(e) => setCloseToTray(e.target.checked)}
              className="sr-only peer"
            />
            <div className="w-11 h-6 bg-slate-800 peer-focus:outline-none rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-purple-600"></div>
          </label>
        </div>
      </div>
    </div>
  );
};
