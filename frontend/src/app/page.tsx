'use client';

import React, { useState, useEffect, useCallback, useRef, Suspense } from 'react';
import { useSearchParams } from 'next/navigation';
import { LaunchModal } from '@/components/LaunchModal';
import { LauncherRedesign } from '@/components/LauncherRedesign';
import { LegacyLauncherNotice } from '@/components/LegacyLauncherNotice';
import { AuthScreen } from '@/components/AuthScreen';
import { Splash } from '@/components/Splash';
import { ipc, AppItem, LauncherProfile, ChangelogItem } from '@/lib/ipc';
import { MOCK_ENABLED, mockParam } from '@/lib/mock';

function MainLauncherContent() {
  const searchParams = useSearchParams();
  const queryToken = searchParams.get('token');

  const [token, setToken] = useState<string | null>(null);
  const [user, setUser] = useState<LauncherProfile | null>(null);
  const [apps, setApps] = useState<AppItem[]>([]);
  const [changelogs, setChangelogs] = useState<ChangelogItem[]>([]);
  const [activeLaunchApp, setActiveLaunchApp] = useState<AppItem | null>(null);
  const [ready, setReady] = useState(false);
  const initialLoad = useRef<AbortController | null>(null);

  useEffect(() => () => initialLoad.current?.abort(), []);

  // This effect runs after React commits the ready screen to the DOM. Avoid
  // requestAnimationFrame here: a hidden WebView can suspend animation frames.
  useEffect(() => {
    if (ready) void ipc.notifyLauncherReady();
  }, [ready]);

  const handleAuthScreenReady = useCallback(() => setReady(true), []);

  // Re-fetch apps after a build switch: each build carries its own app id.
  const refreshApps = useCallback(async (signal?: AbortSignal): Promise<boolean> => {
    try {
      const fetchedApps = await ipc.getApps(signal);
      if (signal?.aborted) return false;
      setApps(Array.isArray(fetchedApps) ? fetchedApps : []);
      return true;
    } catch (e) {
      if (signal?.aborted) return false;
      console.warn('Could not load apps list', e);
      setApps([]);
      return false;
    }
  }, []);

  // When auth succeeds with verified profile and token
  const handleAuthSuccess = useCallback(async (verifiedUser: LauncherProfile, authToken: string) => {
    initialLoad.current?.abort();
    const controller = new AbortController();
    initialLoad.current = controller;
    setReady(false);
    setUser(verifiedUser);
    setToken(authToken);

    // Stale launchers must go directly to the site download page. Avoid
    // loading game data that cannot be used while this mandatory gate is shown.
    if (verifiedUser.launcher_update_required) {
      setReady(true);
      return;
    }

    // A successful empty catalog is valid; a network failure is not readiness.
    while (!controller.signal.aborted) {
      const loaded = await refreshApps(controller.signal);
      if (controller.signal.aborted) return;
      if (loaded) {
        setReady(true);
        return;
      }
      await new Promise<void>((resolve) => setTimeout(resolve, 3000));
    }
  }, [refreshApps]);

  // Called by the build selector once the choice is stored. The new build may
  // carry another app id, so the list is fetched again.
  const handleBuildChanged = async (productId: string, build: string) => {
    setUser((prev) => (prev && prev.builds
      ? { ...prev, builds: prev.builds.map((b) => (b.product_id === productId ? { ...b, selected: build } : b)) }
      : prev));
    await refreshApps();
  };

  // The changelog shown is the one of the first app, for the build the account runs.
  const primaryProductId = apps[0]?.product_id ?? '';
  const primaryBuild = user?.builds?.find((b) => b.product_id === primaryProductId)?.selected ?? '';
  useEffect(() => {
    if (!token || !primaryProductId) return;
    let cancelled = false;
    ipc.getChangelogs(primaryProductId, primaryBuild || undefined, token).then((logs) => {
      if (!cancelled) setChangelogs(logs);
    });
    return () => {
      cancelled = true;
    };
  }, [token, primaryProductId, primaryBuild]);

  // Mock mode: `?mock_open=launch` opens the launch modal as soon as the
  // product list is in, so every modal state can be screenshotted headlessly.
  useEffect(() => {
    if (MOCK_ENABLED && apps.length > 0 && mockParam('mock_open') === 'launch') {
      setActiveLaunchApp(apps[0]);
    }
  }, [apps]);

  const handleLogout = () => {
    initialLoad.current?.abort();
    if (typeof window !== 'undefined') {
      localStorage.removeItem('launcher_token');
    }
    setUser(null);
    setToken(null);
    setReady(false);
  };

  if (!user || !token) {
    return (
      <AuthScreen 
        initialToken={queryToken} 
        onSuccess={handleAuthSuccess} 
        onReady={handleAuthScreenReady}
      />
    );
  }

  if (user.launcher_update_required) {
    return <LegacyLauncherNotice />;
  }

  return (
    <>
      <LauncherRedesign
        apps={apps}
        user={user}
        token={token}
        changelogs={changelogs}
        onLaunch={(app) => setActiveLaunchApp(app)}
        onLogout={handleLogout}
        onBuildChanged={handleBuildChanged}
      />

      <LaunchModal
        app={activeLaunchApp}
        token={token}
        onClose={() => setActiveLaunchApp(null)}
        onLog={(msg) => console.log(msg)}
      />
    </>
  );
}

export default function Home() {
  return (
    <Suspense fallback={<Splash />}>
      <MainLauncherContent />
    </Suspense>
  );
}
