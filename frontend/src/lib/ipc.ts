// IPC Bridge for Weave Launcher
// Communicates with Saucer C++ native engine and/or the Backend Loader Server

export interface AppItem {
  id: string;
  name: string;
  subtitle: string;
  category: string;
  status: 'Undetected' | 'Updating' | 'Maintenance' | 'Testing';
  statusColor: string;
  version: string;
  lastUpdate: string;
  processName: string;
  banner: string;
  icon: string;
  rating: number;
  activeUsers: number;
  features: string[];
  description: string;
}

export interface UserProfile {
  id: string;
  username: string;
  plan: string;
  expiresAt: string;
  avatar: string;
  hwidBound: boolean;
  allowedApps: string[];
}

export interface SystemSpecs {
  os: string;
  cpu: string;
  gpu: string;
  ramGb: number;
  hwid: string;
  antivirusDetected: string;
  secureBoot: boolean;
}

export interface NewsItem {
  id: number;
  title: string;
  tag: string;
  tagColor: string;
  date: string;
  author: string;
  content: string;
}

export interface LauncherProfile {
  id: string;
  username: string;
  avatar: string;
}

export interface ChangelogItem {
  id: string;
  product_id: string;
  version: string;
  title: string;
  content: string;
  created_at: string;
}

declare global {
  interface Window {
    saucer?: {
      call: (funcName: string, ...args: any[]) => Promise<any>;
    };
    weaveNative?: {
      minimizeWindow: () => void;
      maximizeWindow: () => void;
      closeWindow: () => void;
      getSystemInfo: () => string; // returns JSON
      launchProcess: (appId: string, processName: string, args: string) => Promise<string>;
      checkProcessRunning: (processName: string) => boolean;
      getHwid: () => string;
    };
  }
}

const BACKEND_URL = process.env.NEXT_PUBLIC_BACKEND_URL || 'http://localhost:4000';

class WeaveIPCBridge {
  private ws: WebSocket | null = null;
  private wsListeners: ((data: any) => void)[] = [];

  constructor() {
    if (typeof window !== 'undefined') {
      this.initWebSocket();
    }
  }

  private initWebSocket() {
    try {
      const wsUrl = BACKEND_URL.replace(/^http/, 'ws') + '/ws';
      this.ws = new WebSocket(wsUrl);

      this.ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          this.wsListeners.forEach((listener) => listener(data));
        } catch (e) {
          console.error('Error parsing WS message', e);
        }
      };

      this.ws.onclose = () => {
        // Reconnect after 3 seconds
        setTimeout(() => this.initWebSocket(), 3000);
      };
    } catch (e) {
      console.warn('WebSocket connection error', e);
    }
  }

  public onLiveEvent(callback: (data: any) => void) {
    this.wsListeners.push(callback);
    return () => {
      this.wsListeners = this.wsListeners.filter((cb) => cb !== callback);
    };
  }

  // Window Controls for Frameless Saucer / Native Window
  public startDrag() {
    import('@saucer-dev/types').then(saucer => {
      saucer.startDrag().catch(() => {});
    });
  }

  public minimize() {
    import('@saucer-dev/types').then(saucer => {
      saucer.minimize(true).catch(() => {});
    });
  }

  public maximize() {
    import('@saucer-dev/types').then(saucer => {
      saucer.maximized().then(isMax => saucer.maximize(!isMax)).catch(() => {});
    });
  }

  public close() {
    import('@saucer-dev/types').then(saucer => {
      saucer.close().catch(() => {});
    });
  }

  // HWID and System Spec retrieval
  public async getSystemSpecs(): Promise<SystemSpecs> {
    if (window.weaveNative?.getSystemInfo) {
      try {
        const info = JSON.parse(window.weaveNative.getSystemInfo());
        return info;
      } catch (e) {
        console.error('Error parsing native system info', e);
      }
    }

    // Default / Mock System Specs with real looking data
    return {
      os: 'Windows 11 Pro 64-bit (Build 26100)',
      cpu: 'AMD Ryzen 7 7800X3D 8-Core Processor @ 4.20 GHz',
      gpu: 'NVIDIA GeForce RTX 4080 (16GB GDDR6X)',
      ramGb: 32,
      hwid: window.weaveNative?.getHwid?.() || 'WEAVE-HWID-A4F8-92B1-7E03-CC91',
      antivirusDetected: 'Windows Defender (Game Mode Active)',
      secureBoot: true
    };
  }

  // Authentication with Backend Server
  public async login(licenseKey: string, username?: string): Promise<{ success: boolean; token?: string; user?: UserProfile; message?: string }> {
    const hwid = (await this.getSystemSpecs()).hwid;
    try {
      const res = await fetch(`${BACKEND_URL}/api/v1/auth/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ licenseKey, username, hwid })
      });
      const data = await res.json();
      return data;
    } catch (e: any) {
      console.warn('Backend server unreachable, using offline developer profile', e);
      return {
        success: true,
        token: 'local_dev_token_offline',
        user: {
          id: 'usr_dev',
          username: username || 'WeaveDeveloper',
          plan: 'Offline VIP Pass',
          expiresAt: '2099-12-31T23:59:59Z',
          avatar: 'https://images.unsplash.com/photo-1566492031773-4f4e44671857?w=150&auto=format&fit=crop&q=80',
          hwidBound: true,
          allowedApps: ['cs2', 'rust', 'apex', 'gtav', 'dota2', 'minecraft']
        }
      };
    }
  }

  // Fetch Catalog Apps
  public async getApps(): Promise<AppItem[]> {
    try {
      const res = await fetch(`${BACKEND_URL}/api/v1/apps`);
      const data = await res.json();
      if (data.success && data.apps) {
        return data.apps;
      }
    } catch (e) {
      console.warn('Failed to fetch apps from backend, using fallback catalog');
    }

    return [
      {
        id: 'cs2',
        name: 'Counter-Strike 2',
        subtitle: 'Next-Gen Kernel & Cloud Injection Suite',
        category: 'FPS / Competitive',
        status: 'Undetected',
        statusColor: '#10b981',
        version: 'v3.4.1',
        lastUpdate: '2026-08-28',
        processName: 'cs2.exe',
        banner: 'https://images.unsplash.com/photo-1542751371-adc38448a05e?w=800&auto=format&fit=crop&q=80',
        icon: 'Crosshair',
        rating: 4.95,
        activeUsers: 1420,
        features: ['Kernel Driver Loader', 'Dynamic Stream Overlay', 'Memory Integrity Guard', 'Cloud Profile Sync'],
        description: 'Ultra-low latency kernel injection driver with fully customized UI overlays and cloud profile synchronization.'
      },
      {
        id: 'rust',
        name: 'Rust Experimental',
        subtitle: 'Advanced Asset & Automation Loader',
        category: 'Survival / MMO',
        status: 'Undetected',
        statusColor: '#10b981',
        version: 'v2.8.0',
        lastUpdate: '2026-08-26',
        processName: 'RustClient.exe',
        banner: 'https://images.unsplash.com/photo-1518709268805-4e9042af9f23?w=800&auto=format&fit=crop&q=80',
        icon: 'Flame',
        rating: 4.88,
        activeUsers: 840,
        features: ['EAC Bypass Layer', 'Recoil Compensator DSP', 'Asset Scanner', 'Silent Loot Tracker'],
        description: 'High performance native automation layer with EAC safety sandboxing and fast memory mapping.'
      }
    ];
  }

  // Fetch News Feed
  public async getNews(): Promise<NewsItem[]> {
    try {
      const res = await fetch(`${BACKEND_URL}/api/v1/news`);
      const data = await res.json();
      if (data.success && data.news) {
        return data.news;
      }
    } catch (e) {
      console.warn('Failed to fetch news from backend');
    }

    return [
      {
        id: 1,
        title: 'Weave Launcher 2.0 Released with Saucer C++ & Next.js UI',
        tag: 'ANNOUNCEMENT',
        tagColor: 'bg-indigo-500/20 text-indigo-400 border-indigo-500/30',
        date: 'August 28, 2026',
        author: 'Weave Core Team',
        content: 'We are thrilled to unveil our brand new launcher completely rewritten in modern C++ with the lightweight Saucer WebView engine and Next.js frontend.'
      }
    ];
  }

  // Proxy API: Get Launcher User Profile
  public async getLauncherProfile(token?: string): Promise<LauncherProfile | null> {
    const launcherToken = token || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') || '' : '');
    try {
      const query = launcherToken ? `?token=${encodeURIComponent(launcherToken)}` : '';
      const res = await fetch(`/api/launcher/profile${query}`, {
        headers: {
          'X-Launcher-Token': launcherToken
        }
      });
      if (res.ok) {
        return await res.json();
      }
    } catch (e) {
      console.warn('Failed to fetch launcher profile via proxy', e);
    }
    return null;
  }

  // Proxy API: Get Changelogs
  public async getChangelogs(productId?: string, token?: string): Promise<ChangelogItem[]> {
    const launcherToken = token || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') || '' : '');
    try {
      const params = new URLSearchParams();
      if (productId) params.set('product_id', productId);
      if (launcherToken) {
        params.set('token', launcherToken);
        params.set('launcher_token', launcherToken);
      }
      const query = params.toString() ? `?${params.toString()}` : '';
      const res = await fetch(`/api/launcher/changelogs${query}`, {
        headers: {
          'X-Launcher-Token': launcherToken
        }
      });
      if (res.ok) {
        const data = await res.json();
        if (Array.isArray(data)) return data;
        if (Array.isArray(data?.changelogs)) return data.changelogs;
        if (Array.isArray(data?.data)) return data.data;
        if (Array.isArray(data?.items)) return data.items;
        return [];
      }
    } catch (e) {
      console.warn('Failed to fetch changelogs via proxy', e);
    }
    return [];
  }

  // Native Target Launcher / Injector Pipeline

  public async launchApp(
    app: AppItem,
    token: string,
    onProgress: (
      downloadStage: string,
      downloadProgress: number,
      mmapStage: string,
      mmapProgress: number,
      isFinal: boolean
    ) => void
  ): Promise<{ success: boolean; message: string }> {
    try {
      const saucer = await import('@saucer-dev/types');
      const authToken = token || (typeof window !== 'undefined' ? localStorage.getItem('launcher_token') || '' : '');

      // Trigger asynchronous background injection
      try {
        await saucer.call('start_injection', [app.id, authToken]);
      } catch {
        // Fallback if start_injection is not available
        saucer.call('fetch_and_inject', [app.id, authToken]).catch(() => {});
      }

      // Poll until is_finished is true or error
      let maxWait = 300; // up to 30 seconds
      while (maxWait-- > 0) {
        try {
          const [downloadStage, downloadProgress, mmapStage, mmapProgress, isFinished] = await Promise.all([
            saucer.call<string>('get_download_stage', []).catch(() => 'Connecting...'),
            saucer.call<number>('get_download_progress', []).catch(() => 0),
            saucer.call<string>('get_mmap_stage', []).catch(() => 'Waiting...'),
            saucer.call<number>('get_mmap_progress', []).catch(() => 0),
            saucer.call<boolean>('is_finished', []).catch(() => false)
          ]);

          onProgress(
            String(downloadStage || 'Connecting...'),
            typeof downloadProgress === 'number' ? downloadProgress : 0,
            String(mmapStage || 'Waiting...'),
            typeof mmapProgress === 'number' ? mmapProgress : 0,
            Boolean(isFinished)
          );

          if (isFinished) {
            return {
              success: true,
              message: `[Weave Loader] Backend payload loaded for ${app.name} (${app.processName}).`
            };
          }

          const combinedStage = String(downloadStage || '') + String(mmapStage || '');
          if (combinedStage.toLowerCase().includes('fail')) {
            return {
              success: false,
              message: String(downloadStage || mmapStage || 'Injection failed')
            };
          }
        } catch (e) {}

        await new Promise((r) => setTimeout(r, 60));
      }

      return {
        success: false,
        message: 'Injection timeout: target process did not respond.'
      };
    } catch (err: any) {
      console.warn('Native injection error', err);
      return {
        success: false,
        message: err?.message || 'Failed to communicate with native launcher engine.'
      };
    }
  }
}

export const ipc = new WeaveIPCBridge();
