'use client';

import React, { useState } from 'react';
import { 
  X, 
  KeyRound, 
  ShieldCheck, 
  User, 
  Loader2, 
  Sparkles,
  Lock,
  ExternalLink
} from 'lucide-react';
import { ipc, UserProfile } from '@/lib/ipc';

interface AuthModalProps {
  isOpen: boolean;
  currentUser: UserProfile | null;
  onClose: () => void;
  onSuccess: (user: UserProfile) => void;
}

export const AuthModal: React.FC<AuthModalProps> = ({
  isOpen,
  currentUser,
  onClose,
  onSuccess
}) => {
  const [licenseKey, setLicenseKey] = useState('');
  const [username, setUsername] = useState('');
  const [isLoading, setIsLoading] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  if (!isOpen) return null;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!licenseKey.trim() && !username.trim()) {
      setErrorMessage('Please enter a license key or username.');
      return;
    }

    setIsLoading(true);
    setErrorMessage('');

    try {
      const res = await ipc.login(licenseKey, username);
      if (res.success && res.user) {
        onSuccess(res.user);
        onClose();
      } else {
        setErrorMessage(res.message || 'Authentication failed. Please check key.');
      }
    } catch (err: any) {
      setErrorMessage(err?.message || 'Server connection error');
    } finally {
      setIsLoading(false);
    }
  };

  const handleUseDemoKey = () => {
    setLicenseKey('WEAVE-DEV-9999-PREMIUM');
    setUsername('WeaveDeveloper');
  };

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center bg-black/80 backdrop-blur-md p-4 animate-in fade-in duration-200 select-none">
      <div className="w-full max-w-md bg-[#0f101a] border border-orange-500/30 rounded-2xl overflow-hidden shadow-2xl shadow-orange-900/40">
        {/* Header */}
        <div className="px-6 py-5 bg-[#141522] border-b border-white/10 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-xl bg-gradient-to-tr from-orange-600 to-orange-600 flex items-center justify-center text-white shadow-md">
              <Lock className="w-5 h-5" />
            </div>
            <div>
              <h2 className="text-base font-bold text-white leading-tight">
                Account & License Binding
              </h2>
              <span className="text-[11px] font-mono text-orange-400">
                HWID Automated Hardware Lock
              </span>
            </div>
          </div>

          <button
            onClick={onClose}
            className="w-8 h-8 rounded-lg bg-white/5 hover:bg-white/10 flex items-center justify-center text-slate-400 hover:text-white transition"
          >
            <X className="w-4 h-4" />
          </button>
        </div>

        {/* Form Body */}
        <form onSubmit={handleSubmit} className="p-6 space-y-4">
          {currentUser && (
            <div className="p-3 bg-orange-950/40 border border-orange-500/30 rounded-xl flex items-center justify-between mb-2">
              <div className="flex items-center gap-2">
                <ShieldCheck className="w-4 h-4 text-emerald-400" />
                <span className="text-xs text-slate-200">
                  Active: <strong className="text-orange-300">{currentUser.username}</strong>
                </span>
              </div>
              <span className="text-[10px] font-mono font-bold text-emerald-400 bg-emerald-950/60 px-2 py-0.5 rounded border border-emerald-500/30">
                {currentUser.plan}
              </span>
            </div>
          )}

          {errorMessage && (
            <div className="p-3 bg-rose-950/40 border border-rose-500/40 rounded-xl text-xs text-rose-300">
              {errorMessage}
            </div>
          )}

          <div className="space-y-1.5">
            <label className="text-xs font-mono text-slate-400 flex items-center gap-1.5">
              <KeyRound className="w-3.5 h-3.5 text-orange-400" />
              License Key
            </label>
            <input
              type="text"
              placeholder="e.g. WEAVE-XXXX-XXXX-XXXX"
              value={licenseKey}
              onChange={(e) => setLicenseKey(e.target.value)}
              className="w-full bg-[#141522] border border-white/10 rounded-xl px-3.5 py-2.5 text-xs font-mono text-orange-200 placeholder:text-slate-600 focus:outline-none focus:border-orange-500/60 transition"
            />
          </div>

          <div className="space-y-1.5">
            <label className="text-xs font-mono text-slate-400 flex items-center gap-1.5">
              <User className="w-3.5 h-3.5 text-orange-400" />
              Custom Username (Optional)
            </label>
            <input
              type="text"
              placeholder="Leave blank to use license profile"
              value={username}
              onChange={(e) => setUsername(e.target.value)}
              className="w-full bg-[#141522] border border-white/10 rounded-xl px-3.5 py-2.5 text-xs text-slate-200 placeholder:text-slate-600 focus:outline-none focus:border-orange-500/60 transition"
            />
          </div>

          {/* Preset test credentials */}
          <div className="pt-1 flex items-center justify-between text-xs">
            <button
              type="button"
              onClick={handleUseDemoKey}
              className="text-orange-400 hover:text-orange-300 flex items-center gap-1 font-mono transition"
            >
              <Sparkles className="w-3.5 h-3.5" />
              <span>Fill Developer VIP Key</span>
            </button>
          </div>

          <div className="pt-3">
            <button
              type="submit"
              disabled={isLoading}
              className="w-full py-3 bg-gradient-to-r from-orange-600 to-orange-600 hover:from-orange-500 hover:to-orange-500 text-white rounded-xl text-xs font-bold transition flex items-center justify-center gap-2 shadow-lg shadow-orange-600/25 cursor-pointer disabled:opacity-60"
            >
              {isLoading ? (
                <Loader2 className="w-4 h-4 animate-spin" />
              ) : (
                <ShieldCheck className="w-4 h-4" />
              )}
              <span>{isLoading ? 'Verifying Hardware Signature...' : 'Activate & Bind HWID'}</span>
            </button>
          </div>
        </form>
      </div>
    </div>
  );
};
