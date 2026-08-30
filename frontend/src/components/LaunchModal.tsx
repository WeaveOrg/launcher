'use client';

import React, { useState, useEffect } from 'react';
import { Loader2, CheckCircle2, AlertCircle } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { AppItem, ipc } from '@/lib/ipc';

interface LaunchModalProps {
  app: AppItem | null;
  token?: string | null;
  onClose: () => void;
  onLog: (msg: string) => void;
}

export const LaunchModal: React.FC<LaunchModalProps> = ({ app, token, onClose, onLog }) => {
  const [stage, setStage] = useState('Initializing Loader pipeline...');
  const [progress, setProgress] = useState(0);
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading');
  const [countdown, setCountdown] = useState<number | null>(null);

  useEffect(() => {
    if (!app) return;

    let mounted = true;
    const runPipeline = async () => {
      onLog(`Started injection for ${app.name}`);
      setStatus('loading');
      setStage('Initializing Loader pipeline...');
      setProgress(5);

      const res = await ipc.launchApp(app, token || '', (currentStage, currentProgress) => {
        if (!mounted) return;
        setStage(currentStage);
        setProgress(currentProgress);
        if (currentProgress >= 100) {
          setStatus('success');
        }
      });

      if (!mounted) return;
      if (res.success) {
        setStatus('success');
        setProgress(100);
        setStage(res.message || 'Payload injected successfully!');
        setCountdown(3);

        // Countdown interval from 3 to 0
        const interval = setInterval(() => {
          setCountdown((prev) => {
            if (prev !== null && prev > 1) return prev - 1;
            clearInterval(interval);
            return 0;
          });
        }, 1000);

        // 3 seconds timer before closing the launcher itself
        setTimeout(() => {
          if (mounted) {
            clearInterval(interval);
            ipc.close();
            onClose();
          }
        }, 3000);
      } else {
        setStatus('error');
        setStage(`Error: ${res.message}`);
      }
    };

    runPipeline();
    return () => { mounted = false; };
  }, [app, token]);

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
            <div className="flex flex-col gap-1">
              <h2 className="text-lg font-extrabold text-white tracking-wide">
                {status === 'success' ? 'Injection Complete' : `Injecting ${app.name}`}
              </h2>
            </div>

            {/* Progress & Stage */}
            <div className="w-full flex flex-col gap-3 mt-1">
              <div className="w-full h-2 bg-[#222] rounded-full overflow-hidden relative">
                <motion.div
                  initial={{ width: 0 }}
                  animate={{ width: `${progress}%` }}
                  transition={{ ease: "easeOut", duration: 0.3 }}
                  className={`absolute top-0 bottom-0 left-0 rounded-full transition-colors duration-500 ${
                    status === 'error'
                      ? 'bg-rose-500 shadow-[0_0_8px_#f43f5e]'
                      : status === 'success' || progress >= 100
                      ? 'bg-emerald-500 shadow-[0_0_12px_#10b981]'
                      : 'bg-[#ff8c00] shadow-[0_0_8px_#ff8c00]'
                  }`}
                />
              </div>
              
              <div className="flex items-center justify-center gap-2 text-xs font-medium text-[#888]">
                {status === 'loading' && <Loader2 className="w-3.5 h-3.5 text-[#ff8c00] animate-spin" />}
                {(status === 'success' || progress >= 100) && (
                  <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}>
                    <CheckCircle2 className="w-4 h-4 text-emerald-400" />
                  </motion.div>
                )}
                {status === 'error' && (
                  <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}>
                    <AlertCircle className="w-4 h-4 text-rose-500" />
                  </motion.div>
                )}
                
                <motion.span 
                  key={stage}
                  initial={{ opacity: 0, y: 2 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={
                    status === 'error' 
                      ? 'text-rose-400 font-mono text-[11px]' 
                      : (status === 'success' || progress >= 100) 
                      ? 'text-emerald-400 font-semibold' 
                      : 'text-[#aaa]'
                  }
                >
                  {status === 'success'
                    ? (countdown !== null && countdown > 0 
                        ? `Ready! Closing launcher in ${countdown}s...` 
                        : 'Done! Closing launcher...')
                    : `${stage} (${progress}%)`}
                </motion.span>
              </div>
            </div>
            
            {/* Close button on error */}
            <AnimatePresence>
              {status === 'error' && (
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
