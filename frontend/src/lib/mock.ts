// Fixtures for running the UI without the native launcher or backend.
// Enabled with NEXT_PUBLIC_MOCK_API=1 (see `npm run dev:mock`). Used by the
// proxy API routes and the IPC bridge; never bundled into production behaviour
// unless the flag is set at build time.
import type { AppItem, ChangelogItem, LauncherProfile } from './ipc';

export const MOCK_ENABLED = process.env.NEXT_PUBLIC_MOCK_API === '1';

export const mockProfile: LauncherProfile = {
  id: 'usr_mock_1',
  username: 'macan',
  avatar: '',
  channel: 'stable',
  beta_access: true,
  latest_launcher_version: '1.4.2',
  launcher_downloaded_version: '1.4.2',
  launcher_update_required: false,
};

export const mockChangelogs: ChangelogItem[] = [
  {
    id: 'cl_3',
    product_id: '6a943ac671805d202d5fc1e0',
    version: '1.4.2',
    title: 'Stability',
    content: 'Fixed a crash on map change. Reduced loader download timeout errors on slow connections. Minor UI fixes in the settings panel.',
    created_at: '2026-09-18T14:00:00Z',
  },
  {
    id: 'cl_2',
    product_id: '6a943ac671805d202d5fc1e0',
    version: '1.4.1',
    title: 'Beta channel',
    content: 'Accounts with beta access can now switch channels from the launcher. Improved proxy detection and HTTP failure reporting.',
    created_at: '2026-09-10T10:30:00Z',
  },
  {
    id: 'cl_1',
    product_id: '6a943ac671805d202d5fc1e0',
    version: '1.4.0',
    title: 'New launcher',
    content: 'Launcher rewritten on Saucer + Next.js. Faster startup, smaller binary, live changelog feed.',
    created_at: '2026-08-28T09:00:00Z',
  },
];

export const mockApps: AppItem[] = [
  {
    id: '6a943ac671805d202d5fc1e0',
    name: 'Weave CS2',
    subtitle: 'Counter-Strike 2',
    category: 'FPS',
    status: 'Undetected',
    statusColor: '#22c55e',
    version: '1.4.2',
    lastUpdate: '2026-09-18',
    processName: 'cs2.exe',
    banner: '',
    icon: 'Crosshair',
    rating: 4.8,
    activeUsers: 1240,
    features: ['Aimbot', 'ESP', 'Radar', 'Skin changer', 'Misc'],
    description: 'Full-featured internal for CS2 with stream-proof overlay.',
  },
  {
    id: 'prod_rust',
    name: 'Weave Rust',
    subtitle: 'Rust',
    category: 'Survival',
    status: 'Updating',
    statusColor: '#f59e0b',
    version: '0.9.7',
    lastUpdate: '2026-09-15',
    processName: 'RustClient.exe',
    banner: '',
    icon: 'Flame',
    rating: 4.5,
    activeUsers: 310,
    features: ['ESP', 'Recoil', 'Misc'],
    description: 'Updating for the September patch.',
  },
  {
    id: 'prod_apex',
    name: 'Weave Apex',
    subtitle: 'Apex Legends',
    category: 'FPS',
    status: 'Maintenance',
    statusColor: '#ef4444',
    version: '2.1.0',
    lastUpdate: '2026-09-01',
    processName: 'r5apex.exe',
    banner: '',
    icon: 'Zap',
    rating: 4.2,
    activeUsers: 0,
    features: ['Aimbot', 'ESP'],
    description: 'Temporarily down for maintenance.',
  },
];

// Simulates the native loader pipeline. `?mock_launch=` on the page URL picks
// the outcome: `error` fails at the download stage, `hold` stays in progress
// forever (for screenshots), anything else succeeds.
export function mockParam(name: string): string | null {
  if (typeof window === 'undefined') return null;
  return new URLSearchParams(window.location.search).get(name);
}

export async function mockLaunch(
  app: AppItem,
  onProgress: (stage: string, progress: number, isFinal: boolean) => void,
): Promise<{ success: boolean; message: string }> {
  const mode = mockParam('mock_launch');
  const fail = mode === 'error';
  const hold = mode === 'hold';
  const stages: Array<[string, number]> = [
    ['Connecting to CDN...', 5],
    ['Resolving proxy', 15],
    ['Downloading loader.dll (1.2 MB / 3.4 MB)', 35],
    ['Downloading loader.dll (3.4 MB / 3.4 MB)', 60],
    ['Verifying signature', 75],
    ['Waiting for ' + app.processName, 85],
    ['Mapping module', 95],
  ];
  for (const [stage, progress] of stages) {
    onProgress(stage, progress, false);
    await new Promise((r) => setTimeout(r, 450));
    if (fail && progress === 35) {
      return { success: false, message: 'Failed Loading Library (1) [7]: SSL handshake failed [native/WSA: 10054]' };
    }
    if (hold && progress === 60) {
      await new Promise(() => {}); // never resolves
    }
  }
  onProgress('Done', 100, true);
  return { success: true, message: `[Weave Loader] Backend payload loaded for ${app.name} (${app.processName}).` };
}
