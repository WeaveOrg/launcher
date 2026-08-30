'use client';

import React, { useState, useEffect, Suspense } from 'react';
import { useSearchParams } from 'next/navigation';
import { LaunchModal } from '@/components/LaunchModal';
import { LauncherRedesign } from '@/components/LauncherRedesign';
import { AuthScreen } from '@/components/AuthScreen';
import { ipc, AppItem, LauncherProfile, ChangelogItem } from '@/lib/ipc';

function MainLauncherContent() {
  const searchParams = useSearchParams();
  const queryToken = searchParams.get('token');

  const [token, setToken] = useState<string | null>(null);
  const [user, setUser] = useState<LauncherProfile | null>(null);
  const [apps, setApps] = useState<AppItem[]>([]);
  const [changelogs, setChangelogs] = useState<ChangelogItem[]>([]);
  const [activeLaunchApp, setActiveLaunchApp] = useState<AppItem | null>(null);

  // When auth succeeds with verified profile and token
  const handleAuthSuccess = async (verifiedUser: LauncherProfile, authToken: string) => {
    setUser(verifiedUser);
    setToken(authToken);

    // Fetch changelogs from backend API
    try {
      let logs = await ipc.getChangelogs(undefined, authToken);
      if (!Array.isArray(logs) || logs.length === 0) {
        logs = await ipc.getChangelogs('cs2', authToken);
      }
      if (Array.isArray(logs) && logs.length > 0) {
        setChangelogs(logs);
      } else {
        setChangelogs([]);
      }
    } catch (e) {
      console.warn('Could not load dynamic changelogs', e);
      setChangelogs([]);
    }

    // Fetch apps
    try {
      const fetchedApps = await ipc.getApps();
      if (Array.isArray(fetchedApps)) {
        setApps(fetchedApps);
      } else {
        setApps([]);
      }
    } catch (e) {
      console.warn('Could not load apps list', e);
      setApps([]);
    }
  };

  const handleLogout = () => {
    if (typeof window !== 'undefined') {
      localStorage.removeItem('launcher_token');
    }
    setUser(null);
    setToken(null);
  };

  if (!user || !token) {
    return (
      <AuthScreen 
        initialToken={queryToken} 
        onSuccess={handleAuthSuccess} 
      />
    );
  }

  return (
    <>
      <LauncherRedesign
        apps={apps}
        user={user}
        token={token}
        changelogs={changelogs}
        onlineCount={4892}
        ping={16}
        onLaunch={(app) => setActiveLaunchApp(app)}
        onLogout={handleLogout}
      />

      <LaunchModal
        app={activeLaunchApp}
        onClose={() => setActiveLaunchApp(null)}
        onLog={(msg) => console.log(msg)}
      />
    </>
  );
}

export default function Home() {
  return (
    <Suspense fallback={
      <div className="flex h-screen w-screen items-center justify-center bg-[#0d0d0d] text-white">
        <span className="text-xs font-mono text-[#666]">INITIALIZING WEAVE...</span>
      </div>
    }>
      <MainLauncherContent />
    </Suspense>
  );
}
