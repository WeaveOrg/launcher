'use client';

import React, { useState, useEffect } from 'react';
import { Loader2, CheckCircle2, AlertCircle } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { AppItem, ipc } from '@/lib/ipc';

interface LaunchModalProps {
  app: AppItem | null;
  onClose: () => void;
  onLog: (msg: string) => void;
}

export const LaunchModal: React.FC<LaunchModalProps> = ({ app, onClose, onLog }) => {
  const [stage, setStage] = useState('Initializing Loader pipeline...');
  const [progress, setProgress] = useState(0);
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading');

  useEffect(() => {
    if (!app) return;

    let mounted = true;
    const runPipeline = async () => {
      onLog(`Started injection for ${app.name}`);

      const res = await ipc.launchApp(app, (currentStage, currentProgress) => {
        if (!mounted) return;
        setStage(currentStage);
        setProgress(currentProgress);
      });

      if (!mounted) return;
      if (res.success) {
        setStatus('success');
        setStage('Successfully injected!');
        setTimeout(() => {
          if (mounted) onClose();
        }, 2000);
      } else {
        setStatus('error');
        setStage(`Error: ${res.message}`);
      }
    };

    runPipeline();
    return () => { mounted = false; };
  }, [app]);

  return (
    <AnimatePresence>
      {app && (
        <motion.div 
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          exit={{ opacity: 0 }}
          className="fixed inset-0 z-[100] flex items-center justify-center bg-black/80 backdrop-blur-sm p-4"
        >
          <motion.div 
            initial={{ opacity: 0, scale: 0.9, y: 10 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: -10 }}
            transition={{ type: 'spring', damping: 25, stiffness: 300 }}
            className="w-full max-w-sm bg-[#121212] border border-[#2a2a2a] rounded-2xl p-6 shadow-2xl shadow-black flex flex-col items-center gap-5 text-center"
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
                Injecting {app.name}
              </h2>
            </div>

            {/* Progress & Stage */}
            <div className="w-full flex flex-col gap-3 mt-2">
              <div className="w-full h-1.5 bg-[#222] rounded-full overflow-hidden relative">
                <motion.div
                  initial={{ width: 0 }}
                  animate={{ width: `${progress}%` }}
                  transition={{ ease: "easeOut", duration: 0.3 }}
                  className={`absolute top-0 bottom-0 left-0 rounded-full ${
                    status === 'error'
                      ? 'bg-rose-500'
                      : status === 'success'
                      ? 'bg-emerald-500'
                      : 'bg-[#ff8c00]'
                  }`}
                />
              </div>
              
              <div className="flex items-center justify-center gap-2 text-xs font-medium text-[#888]">
                {status === 'loading' && <Loader2 className="w-3.5 h-3.5 text-[#ff8c00] animate-spin" />}
                {status === 'success' && <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}><CheckCircle2 className="w-3.5 h-3.5 text-emerald-500" /></motion.div>}
                {status === 'error' && <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }}><AlertCircle className="w-3.5 h-3.5 text-rose-500" /></motion.div>}
                
                <motion.span 
                  key={stage}
                  initial={{ opacity: 0, y: 2 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={status === 'error' ? 'text-rose-400' : status === 'success' ? 'text-emerald-400' : 'text-[#aaa]'}
                >
                  {stage} {status === 'loading' && `(${progress}%)`}
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
