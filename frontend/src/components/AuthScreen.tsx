'use client';

import React, { useState, useEffect, useCallback, useRef } from 'react';
import { ShieldAlert, RefreshCw, AlertCircle } from 'lucide-react';
import { ipc, LauncherProfile } from '@/lib/ipc';
import { motion } from 'framer-motion';
import { WeaveMark } from './WeaveMark';
import { TitleBar, WindowShell } from './WindowChrome';
import { Splash } from './Splash';

interface AuthScreenProps {
  initialToken?: string | null;
  onSuccess: (user: LauncherProfile, token: string) => void;
  onReady?: () => void;
}

export const AuthScreen: React.FC<AuthScreenProps> = ({ initialToken, onSuccess, onReady }) => {
  const [error, setError] = useState<string>('');
  const [errorDetails, setErrorDetails] = useState<any>(null);
  const [loading, setLoading] = useState<boolean>(true);
  const [currentToken, setCurrentToken] = useState<string>('');
  const retryTimer = useRef<ReturnType<typeof setTimeout> | null>(null);
  const activeRequest = useRef<AbortController | null>(null);
  const [retrying, setRetrying] = useState(false);

  const verifyToken = useCallback(async (targetToken: string) => {
    if (retryTimer.current !== null) clearTimeout(retryTimer.current);
    retryTimer.current = null;
    activeRequest.current?.abort();
    const controller = new AbortController();
    activeRequest.current = controller;

    const scheduleRetry = () => {
      setRetrying(true);
      retryTimer.current = setTimeout(() => { void verifyToken(targetToken); }, 3000);
    };
    if (!targetToken) {
      setError('Session Not Found');
      setErrorDetails({
        message: 'No launcher authorization token detected.',
        hint: 'Please start the game through the official Weave launcher.'
      });
      setLoading(false);
      return;
    }

    try {
      setLoading(true);
      setRetrying(false);
      setError('');
      setErrorDetails(null);

      const res = await fetch(`/api/launcher/profile?token=${encodeURIComponent(targetToken)}`, {
        method: 'GET',
        cache: 'no-store',
        signal: AbortSignal.any([controller.signal, AbortSignal.timeout(15_000)]),
        headers: {
          'X-Launcher-Token': targetToken,
          'Accept': 'application/json'
        }
      });

      const profileData = await res.json().catch(() => null);
      if (controller.signal.aborted) return;

      if (res.ok) {
        if (profileData && profileData.id) {
          if (typeof window !== 'undefined') {
            localStorage.setItem('launcher_token', targetToken);
          }
          onSuccess(profileData, targetToken);
          return;
        }
      }

      // Keep trying transient failures while the native startup window is
      // hidden. A rejected session is terminal and must remain visible.
      if (res.status >= 500 || res.status === 408 || res.status === 429 || res.ok) {
        scheduleRetry();
        return;
      }

      const errData = profileData;
      setError(errData?.error || 'Authentication Failed');
      setErrorDetails({
        status: res.status,
        message: errData?.message || errData?.details || 'Invalid or expired launcher session.',
        targetUrl: errData?.targetUrl || '/api/launcher/profile',
        hint: res.status === 401 
          ? 'Backend rejected the token. Make sure your account subscription is active.' 
          : res.status === 502 
          ? 'Cannot connect to backend server. Check server status or internet connection.' 
          : 'Backend returned an error status code.'
      });
    } catch {
      if (controller.signal.aborted) return;
      scheduleRetry();
    } finally {
      if (!controller.signal.aborted) setLoading(false);
    }
  }, [onSuccess]);

  useEffect(() => {
    // Notify only after the terminal auth screen has been committed, never
    // for a temporary connection error or a pending retry.
    if (!loading && !retrying && error) onReady?.();
  }, [loading, retrying, error, onReady]);

  useEffect(() => {
    const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
    const queryToken = urlParams?.get('token') || urlParams?.get('launcher_token');
    const token = initialToken || queryToken || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') : null);

    if (token) {
      setCurrentToken(token);
      verifyToken(token);
    } else {
      setError('Session Not Found');
      setErrorDetails({
        message: 'No launcher authorization token detected.',
        hint: 'Please start the game through the official Weave launcher.'
      });
      setLoading(false);
    }

    return () => {
      activeRequest.current?.abort();
      if (retryTimer.current !== null) clearTimeout(retryTimer.current);
    };
  }, [initialToken, verifyToken]);

  const resolveToken = () => {
    const urlParams = typeof window !== 'undefined' ? new URLSearchParams(window.location.search) : null;
    const qToken = urlParams?.get('token') || urlParams?.get('launcher_token');
    return currentToken || initialToken || qToken || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') : null);
  };
  // Without a token there is nothing to retry — the launcher has to be
  // restarted from the site — so only offer Retry when one exists.
  const canRetry = Boolean(resolveToken());
  const retry = () => {
    const t = resolveToken();
    if (t) verifyToken(t);
  };

  if (loading || retrying) {
    return <Splash label={retrying ? 'Connecting to server' : 'Checking your session'} />;
  }

  return (
    <WindowShell>
      <TitleBar>
        <WeaveMark width={30} className="shrink-0 text-accent" />
        <span className="text-sm font-semibold text-fg-0">Weave Launcher</span>
      </TitleBar>

      <main className="flex flex-1 items-center justify-center overflow-y-auto px-8">
        <motion.div
          initial={{ opacity: 0, y: 8 }}
          animate={{ opacity: 1, y: 0 }}
          className="flex w-full max-w-md flex-col items-center gap-5"
        >
          {error ? (
            <motion.div
              initial={{ opacity: 0, scale: 0.96 }}
              animate={{ opacity: 1, scale: 1 }}
              className="flex w-full flex-col items-center gap-4 text-center"
            >
              <span className="flex size-14 items-center justify-center rounded-2xl border border-danger/30 bg-danger/10 text-danger">
                <ShieldAlert className="size-6" aria-hidden="true" />
              </span>

              <div className="flex flex-col gap-1">
                <h1 className="text-lg font-semibold text-fg-0">{error}</h1>
                {errorDetails?.message && (
                  <p className="max-w-sm text-xs leading-relaxed text-fg-1">{errorDetails.message}</p>
                )}
              </div>

              {errorDetails && (errorDetails.hint || errorDetails.status) && (
                <div className="flex w-full items-start gap-2.5 rounded-lg border border-line bg-ink-1 px-3.5 py-3 text-left">
                  <AlertCircle className="mt-px size-3.5 shrink-0 text-warn" aria-hidden="true" />
                  <div className="flex min-w-0 flex-1 flex-col gap-1">
                    {errorDetails.hint && (
                      <p className="text-xs leading-relaxed text-fg-1">{errorDetails.hint}</p>
                    )}
                    {errorDetails.status && (
                      <p className="font-mono text-[11px] text-fg-2">HTTP {errorDetails.status}</p>
                    )}
                  </div>
                </div>
              )}

              <div className="mt-1 flex w-full gap-2">
                {canRetry && (
                  <button
                    type="button"
                    onClick={retry}
                    className="flex h-10 flex-1 items-center justify-center gap-2 rounded-full bg-accent text-[13px] font-bold uppercase tracking-wide text-accent-fg transition hover:bg-accent-hover active:scale-[0.97] focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-ink-0"
                  >
                    <RefreshCw className="size-3.5" aria-hidden="true" />
                    Retry
                  </button>
                )}
                <button
                  type="button"
                  onClick={() => ipc.close()}
                  className={`h-10 rounded-full text-[13px] font-semibold transition focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent ${
                    canRetry
                      ? 'border border-line-strong bg-ink-2 px-5 text-fg-1 hover:bg-ink-3 hover:text-fg-0'
                      : 'flex-1 bg-accent uppercase tracking-wide text-accent-fg hover:bg-accent-hover active:scale-[0.97]'
                  }`}
                >
                  Exit
                </button>
              </div>
            </motion.div>
          ) : null}
        </motion.div>
      </main>
    </WindowShell>
  );
};
