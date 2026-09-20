'use client';

import React, { useState, useEffect } from 'react';
import { CheckCircle2, AlertCircle, Cpu } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import { AppItem, ipc } from '@/lib/ipc';
import { WeaveTile } from './WeaveMark';

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
  const [phase, setPhase] = useState<PhaseState>({ stage: 'Connecting to CDN...', progress: 0 });
  const [status, setStatus] = useState<'loading' | 'success' | 'error'>('loading');
  const [countdown, setCountdown] = useState<number | null>(null);
  const [errorMsg, setErrorMsg] = useState('');

  useEffect(() => {
    if (!app) return;

    let mounted = true;
    const runPipeline = async () => {
      onLog(`Started injection for ${app.name}`);
      setStatus('loading');
      setPhase({ stage: 'Connecting to CDN...', progress: 0 });

      const res = await ipc.launchApp(
        app,
        token || '',
        (stage, progress, isFinal) => {
          if (!mounted) return;
          setPhase({ stage, progress });
          if (isFinal) setStatus('success');
        }
      );

      if (!mounted) return;
      if (res.success) {
        setStatus('success');
        setPhase({ stage: `Payload injected into ${app.processName}`, progress: 100 });
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
          className="fixed inset-0 z-[100] flex items-center justify-center bg-black/80 p-4 backdrop-blur-sm"
        >
          <motion.div
            initial={{ opacity: 0, scale: 0.9, y: 10 }}
            animate={{ opacity: 1, scale: 1, y: 0 }}
            exit={{ opacity: 0, scale: 0.95, y: -10 }}
            transition={{ type: 'spring', damping: 25, stiffness: 300 }}
            role="dialog"
            aria-modal="true"
            aria-labelledby="launch-title"
            className="flex w-full max-w-sm select-none flex-col items-center gap-5 rounded-2xl border border-line bg-ink-1 p-6 text-center shadow-2xl shadow-black"
          >
            {app.banner ? (
              <div className="flex h-12 w-20 items-center justify-center overflow-hidden rounded-xl border border-line bg-ink-2">
                <img src={app.banner} alt="" className="size-full object-cover" onError={(e) => { e.currentTarget.style.display = 'none'; }} />
              </div>
            ) : (
              <WeaveTile size={48} />
            )}

            <div className="flex flex-col gap-0.5">
              <h2 id="launch-title" className="text-base font-semibold text-fg-0">
                {isSuccess ? 'Ready' : isError ? 'Launch failed' : `Launching ${app.name}`}
              </h2>
              {!isSuccess && !isError && (
                <p className="text-[11px] text-fg-2">Keep the launcher open until {app.processName} is ready.</p>
              )}
            </div>

            {/* Unified progress */}
            <div className="w-full flex flex-col gap-3 mt-1">
              <div className="w-full flex flex-col gap-1.5">
                <div className="flex items-center justify-between px-0.5">
                  <div className="flex items-center gap-1.5">
                    <Cpu className={`size-3 ${isError ? 'text-danger' : isSuccess ? 'text-ok' : 'text-accent'}`} aria-hidden="true" />
                    <span className="text-[10px] font-semibold uppercase tracking-wider text-fg-2">Progress</span>
                  </div>
                  <span className={`font-mono text-[11px] font-semibold tabular-nums ${
                    isError ? 'text-danger' : isSuccess ? 'text-ok' : 'text-accent'
                  }`}>
                    {phase.progress}%
                  </span>
                </div>
                <div
                  role="progressbar"
                  aria-valuemin={0}
                  aria-valuemax={100}
                  aria-valuenow={phase.progress}
                  className="relative h-1.5 w-full overflow-hidden rounded-full bg-ink-3"
                >
                  <motion.div
                    initial={{ width: 0 }}
                    animate={{ width: `${phase.progress}%` }}
                    transition={{ ease: 'easeOut', duration: 0.3 }}
                    className={`absolute top-0 bottom-0 left-0 rounded-full ${
                      isError
                        ? 'bg-danger'
                        : isSuccess || phase.progress === 100
                        ? 'bg-ok'
                        : 'bg-accent shadow-[0_0_6px_rgb(var(--accent))]'
                    }`}
                  />
                </div>
                <motion.p
                  key={phase.stage}
                  initial={{ opacity: 0, y: 1 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={`truncate text-left text-[11px] ${
                    isError ? 'text-danger' : phase.progress === 100 ? 'text-ok' : 'text-fg-1'
                  }`}
                >
                  {isError ? 'Injection stopped due to an error' : phase.stage}
                </motion.p>
              </div>
            </div>

            {/* Status Label (only on success or error) */}
            {(isSuccess || isError) && (
              <div className={`mt-1 flex w-full items-start gap-2.5 rounded-xl px-3 py-2.5 text-left ${
                isError ? 'border border-danger/25 bg-danger/10' : 'border border-ok/25 bg-ok/10'
              }`}>
                {isSuccess && (
                  <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }} className="shrink-0 mt-0.5">
                    <CheckCircle2 className="size-4 text-ok" aria-hidden="true" />
                  </motion.div>
                )}
                {isError && (
                  <motion.div initial={{ scale: 0 }} animate={{ scale: 1 }} className="shrink-0 mt-0.5">
                    <AlertCircle className="size-4 text-danger" aria-hidden="true" />
                  </motion.div>
                )}
                <motion.span
                  key={isSuccess ? 'success' : 'error'}
                  initial={{ opacity: 0, y: 2 }}
                  animate={{ opacity: 1, y: 0 }}
                  className={`select-text break-words text-xs leading-relaxed ${
                    isError ? 'font-mono text-[11px] text-danger' : 'font-medium text-ok'
                  }`}
                >
                  {isSuccess
                    ? (countdown !== null && countdown > 0
                      ? `Ready! Closing in ${countdown}s...`
                      : 'Done! Closing launcher...')
                    : errorMsg}
                </motion.span>
              </div>
            )}
          </motion.div>
        </motion.div>
      )}
    </AnimatePresence>
  );
};
