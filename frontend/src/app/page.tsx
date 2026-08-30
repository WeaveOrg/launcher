'use client';

import React, { useState, useEffect } from 'react';
import { LaunchModal } from '@/components/LaunchModal';
import { LauncherRedesign } from '@/components/LauncherRedesign';
import { AuthScreen } from '@/components/AuthScreen';
import { ipc, AppItem, UserProfile, SystemSpecs, NewsItem } from '@/lib/ipc';

export default function Home() {
  const [apps, setApps] = useState<AppItem[]>([]);
  const [user, setUser] = useState<UserProfile | null>(null);
  
  const [onlineCount, setOnlineCount] = useState(4892);
  const [ping, setPing] = useState(16);
  const [activeLaunchApp, setActiveLaunchApp] = useState<AppItem | null>(null);

  const loadData = async () => {
    try {
      const fetchedApps = await ipc.getApps();
      setApps(fetchedApps);
    } catch (e) {
      console.error('Error loading data', e);
    }
  };

  useEffect(() => {
    loadData();

    // Remove auto-login so user MUST authenticate manually
    // Real-time WebSocket listener
    const unsubscribe = ipc.onLiveEvent((event) => {
      if (event.type === 'PULSE' && event.payload) {
        setOnlineCount(event.payload.onlineUsers || 4892);
        setPing(event.payload.pingMs || 15);
      }
    });

    return () => {
      unsubscribe();
    };
  }, []);

  if (!user) {
    return <AuthScreen onSuccess={setUser} />;
  }

  return (
    <>
      <LauncherRedesign
        apps={apps}
        user={user}
        onlineCount={onlineCount}
        ping={ping}
        onLaunch={(app) => setActiveLaunchApp(app)}
        onOpenAuth={() => setUser(null)} // Or open an auth modal, but we removed it.
      />

      <LaunchModal
        app={activeLaunchApp}
        onClose={() => setActiveLaunchApp(null)}
        onLog={(msg) => console.log(msg)}
      />
    </>
  );
}
