'use client';

import React, { useState, useEffect, Suspense } from 'react';
import { useSearchParams } from 'next/navigation';
import { LaunchModal } from '@/components/LaunchModal';
import { LauncherRedesign } from '@/components/LauncherRedesign';
import { LegacyLauncherNotice } from '@/components/LegacyLauncherNotice';
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

    // Stale launchers must go directly to the site download page. Avoid
    // loading game data that cannot be used while this mandatory gate is shown.
    if (verifiedUser.launcher_update_required) {
      return;
    }

    // Fetch changelogs for product 6a943ac671805d202d5fc1e0 from backend API
    try {
      const logs = await ipc.getChangelogs('6a943ac671805d202d5fc1e0', authToken);
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
    await refreshApps();
  };

  // Re-fetch the product list. The ids it returns are channel-aware (the
  // backend hands beta accounts the beta build's id), so this must run after
  // the channel changes — otherwise Launch would inject the stale stable id.
  const refreshApps = async () => {
    try {
      const fetchedApps = await ipc.getApps();
      setApps(Array.isArray(fetchedApps) ? fetchedApps : []);
    } catch (e) {
      console.warn('Could not load apps list', e);
      setApps([]);
    }
  };

  // Called by the launcher's channel toggle once the switch is persisted.
  const handleChannelChanged = async (channel: 'stable' | 'beta') => {
    setUser((prev) => (prev ? { ...prev, channel } : prev));
    await refreshApps();
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

  if (user.launcher_update_required) {
    return (
      <LegacyLauncherNotice
        latestVersion={user.latest_launcher_version}
        downloadedVersion={user.launcher_downloaded_version}
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
        onChannelChanged={handleChannelChanged}
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
    <Suspense fallback={
      <div className="flex h-screen w-screen items-center justify-center bg-[#0d0d0d] text-white">
        <span className="text-xs font-mono text-[#666]">INITIALIZING WEAVE...</span>
      </div>
    }>
      <MainLauncherContent />
    </Suspense>
  );
}
