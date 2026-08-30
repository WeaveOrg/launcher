'use client';

import React, { useState, useEffect } from 'react';
import { Loader2, CheckCircle2, AlertCircle, Download, Cpu } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { AppItem, ipc } from '@/lib/ipc';

interface LaunchModalProps {
  app: AppItem | null;
  token?: string | null;
  onClose: () => void;
  onLog: (msg: string) => void;
}

interface PhaseState {
  stage: string;
  progress: number;
}

export const LaunchModal: React.FC<LaunchModalProps> = ({ app, token, onClose, onLog }) => {
  const [download, setDownload] = useState<PhaseState>({ stage: 'Connecting to CDN...', progress: 0 });
  const [mmap, setMmap] = useState<PhaseState>({ stage: 'Waiting...', progress: 0 });
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading');
  const [countdown, setCountdown] = useState<number | null>(null);
  const [errorMsg, setErrorMsg] = useState('');

  useEffect(() => {
    if (!app) return;

    let mounted = true;
    const runPipeline = async () => {
      onLog(`Started injection for ${app.name}`);
      setStatus('loading');
      setDownload({ stage: 'Connecting to CDN...', progress: 0 });
      setMmap({ stage: 'Waiting...', progress: 0 });

      const res = await ipc.launchApp(
        app,
        token || '',
        (dlStage, dlProgress, mmapStage, mmapProgress, isFinal) => {
          if (!mounted) return;
          setDownload({ stage: dlStage, progress: dlProgress });
          setMmap({ stage: mmapStage, progress: mmapProgress });
          if (isFinal) setStatus('success');
        }
      );

      if (!mounted) return;
      if (res.success) {
        setStatus('success');
        setDownload(d => ({ ...d, progress: 100 }));
        setMmap({ stage: 'Library loaded successfully!', progress: 100 });
        setCountdown(3);

        const interval = setInterval(() => {
          setCountdown((prev) => {
            if (prev !== null && prev > 1) return prev - 1;
            clearInterval(interval);
            return 0;
          });
        }, 1000);

        setTimeout(() => {
          if (mounted) {
            clearInterval(interval);
            ipc.close();
            onClose();
          }
        }, 3000);
      } else {
        setStatus('error');
        setErrorMsg(res.message);
      }
    };

    runPipeline();
    return () => { mounted = false; };
  }, [app, token]);

  const isError = status === 'error';
  const isSuccess = status === 'success';

  return (
    <AnimatePresence>
      {app && (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          className="fixed inset-0 z-[100] flex items-center justify-center bg-black/85 backdrop-blur-sm p-4"
        >
          <motion.div
            initial={{ opacity: 0, scale: 0.9, y: 10 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: -10 }}
            transition={{ type: 'spring', damping: 25, stiffness: 300 }}
            className="w-full max-w-sm bg-[#121212] border border-[#2a2a2a] rounded-2xl p-6 shadow-2xl shadow-black flex flex-col items-center gap-5 text-center select-none"
          >

            {/* Game Icon */}
            <div className="w-20 h-12 rounded-xl bg-[#1a1a1a] border border-[#333] flex items-center justify-center overflow-hidden shadow-lg shadow-black/50">
              <img
                src="https://cdn.akamai.steamstatic.com/steam/apps/730/header.jpg"
                alt={app.name}
                className="w-full h-full object-cover"
                onError={(e) => { e.currentTarget.style.display = 'none'; }}
              />
            </div>

            {/* Title */}
            <div className="flex flex-col gap-0.5">
              <h2 className="text-lg font-extrabold text-white tracking-wide">
                {isSuccess ? 'Injection Complete' : `Injecting ${app.name}`}
              </h2>
              <p className="text-xs text-[#555]">Weave Loader Engine v2</p>
            </div>

            {/* Two Phase Progress Bars */}
            <div className="w-full flex flex-col gap-3 mt-1">

              {/* Phase 1: Download loader.dll */}
              <div className="w-full flex flex-col gap-1.5">
                <div className="flex items-center justify-between px-0.5">
                  <div className="flex items-center gap-1.5">
                    <Download className={`w-3 h-3 ${isError ? 'text-rose-400' : isSuccess ? 'text-emerald-400' : 'text-[#ff8c00]'}`} />
                    <span className="text-[10px] font-semibold text-[#666] uppercase tracking-wider">Loader Download</span>
                  </div>
                  <span className={`text-[10px] font-bold tabular-nums ${
                    isError ? 'text-rose-400' : isSuccess ? 'text-emerald-400' : 'text-[#ff8c00]'
                  }`}>
                    {download.progress}%
                  </span>
                </div>
                <div className="w-full h-1.5 bg-[#1e1e1e] rounded-full overflow-hidden relative">
                  <motion.div
                    initial={{ width: 0 }}
                    animate={{ width: `${download.progress}%` }}
                    transition={{ ease: 'easeOut', duration: 0.3 }}
                    className={`absolute top-0 bottom-0 left-0 rounded-full ${
                      isError
                        ? 'bg-rose-500 shadow-[0_0_6px_#f43f5e]'
                        : isSuccess || download.progress === 100
                        ? 'bg-emerald-500 shadow-[0_0_8px_#10b981]'
                        : 'bg-[#ff8c00] shadow-[0_0_6px_#ff8c00]'
                    }`}
                  />
                </div>
                <motion.p
                  key={download.stage}
                  initial={{ opacity: 0, y: 1 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={`text-[10px] font-mono text-left truncate ${
                    isError ? 'text-rose-400' : download.progress === 100 ? 'text-emerald-400' : 'text-[#555]'
                  }`}
                >
                  {download.stage}
                </motion.p>
              </div>

              {/* Divider */}
              <div className="w-full h-px bg-[#1e1e1e]" />

              {/* Phase 2: Library Mapping */}
              <div className="w-full flex flex-col gap-1.5">
                <div className="flex items-center justify-between px-0.5">
                  <div className="flex items-center gap-1.5">
                    <Cpu className={`w-3 h-3 ${isError ? 'text-rose-400' : isSuccess ? 'text-emerald-400' : mmap.progress > 0 ? 'text-[#ff8c00]' : 'text-[#333]'}`} />
                    <span className="text-[10px] font-semibold text-[#666] uppercase tracking-wider">Loading Library</span>
                  </div>
                  <span className={`text-[10px] font-bold tabular-nums ${
                    isError ? 'text-rose-400' : isSuccess ? 'text-emerald-400' : mmap.progress > 0 ? 'text-[#ff8c00]' : 'text-[#333]'
                  }`}>
                    {mmap.progress}%
                  </span>
                </div>
                <div className="w-full h-1.5 bg-[#1e1e1e] rounded-full overflow-hidden relative">
                  <motion.div
                    initial={{ width: 0 }}
                    animate={{ width: `${mmap.progress}%` }}
                    transition={{ ease: 'easeOut', duration: 0.3 }}
                    className={`absolute top-0 bottom-0 left-0 rounded-full ${
                      isError
                        ? 'bg-rose-500 shadow-[0_0_6px_#f43f5e]'
                        : isSuccess || mmap.progress === 100
                        ? 'bg-emerald-500 shadow-[0_0_8px_#10b981]'
                        : mmap.progress > 0
                        ? 'bg-[#ff8c00] shadow-[0_0_6px_#ff8c00]'
                        : 'bg-[#2a2a2a]'
                    }`}
                  />
                </div>
                <motion.p
                  key={mmap.stage}
                  initial={{ opacity: 0, y: 1 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={`text-[10px] font-mono text-left truncate ${
                    isError ? 'text-rose-400' : mmap.progress === 100 ? 'text-emerald-400' : mmap.progress > 0 ? 'text-[#555]' : 'text-[#333]'
                  }`}
                >
                  {mmap.stage}
                </motion.p>
              </div>
            </div>

            {/* Status Label */}
            <div className="flex items-center gap-2 mt-1">
              {status === 'loading' && <Loader2 className="w-3.5 h-3.5 text-[#ff8c00] animate-spin" />}
              {isSuccess && (
                <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}>
                  <CheckCircle2 className="w-4 h-4 text-emerald-400" />
                </motion.div>
              )}
              {isError && (
                <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}>
                  <AlertCircle className="w-4 h-4 text-rose-500" />
                </motion.div>
              )}
              <motion.span
                key={isSuccess ? 'success' : isError ? 'error' : 'loading'}
                initial={{ opacity: 0, y: 2 }}
                animate={{ opacity: 1, y: 0 }}
                className={`text-xs font-medium ${
                  isError ? 'text-rose-400 font-mono text-[11px]' : isSuccess ? 'text-emerald-400 font-semibold' : 'text-[#555]'
                }`}
              >
                {isSuccess
                  ? (countdown !== null && countdown > 0
                    ? `Ready! Closing in ${countdown}s...`
                    : 'Done! Closing launcher...')
                  : isError
                  ? errorMsg
                  : 'Running injection pipeline...'}
              </motion.span>
            </div>

            {/* Dismiss on error */}
            <AnimatePresence>
              {isError && (
                <motion.button
                  initial={{ opacity: 0, height: 0, marginTop: 0 }}
                  animate={{ opacity: 1, height: 'auto', marginTop: 8 }}
                  exit={{ opacity: 0, height: 0, marginTop: 0 }}
                  onClick={onClose}
                  className="px-6 py-2 bg-[#222] hover:bg-[#333] text-white rounded-full text-xs font-bold transition"
                >
                  Dismiss
                </motion.button>
              )}
            </AnimatePresence>
          </motion.div>
        </motion.div>
      )}
    </AnimatePresence>
  );
};
