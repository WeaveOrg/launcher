'use client';

import React, { useState } from 'react';
import { ipc, AppItem, LauncherProfile, ChangelogItem } from '../lib/ipc';
import { Play, User } from 'lucide-react';
import { WeaveMark } from './WeaveMark';
import { TitleBar, WindowShell } from './WindowChrome';

interface LauncherRedesignProps {
  apps: AppItem[];
  user: LauncherProfile;
  token: string;
  changelogs: ChangelogItem[];
  onLaunch: (app: AppItem) => void;
  onLogout: () => void;
  onChannelChanged?: (channel: 'stable' | 'beta') => void | Promise<void>;
}

function formatDate(iso?: string) {
  if (!iso) return '';
  const d = new Date(iso);
  if (Number.isNaN(d.getTime())) return '';
  return d.toLocaleDateString('en-US', { month: 'short', day: 'numeric', year: 'numeric' });
}

export const LauncherRedesign: React.FC<LauncherRedesignProps> = ({
  apps,
  user,
  token,
  changelogs,
  onLaunch,
  onChannelChanged,
}) => {
  const safeApps = Array.isArray(apps) ? apps : [];
  // Backend returns entries oldest-first; the timeline expects the newest
  // release on top.
  const safeChangelogs = (Array.isArray(changelogs) ? [...changelogs] : []).sort(
    (a, b) => new Date(b.created_at).getTime() - new Date(a.created_at).getTime()
  );

  // Release channel. The toggle is only rendered when the account holds beta
  // access; the switch persists server-side and takes effect on the next launch
  // (the loader reads its product id at start).
  const [channel, setChannel] = useState<'stable' | 'beta'>(
    user.channel === 'beta' ? 'beta' : 'stable'
  );
  const [switching, setSwitching] = useState(false);

  const changeChannel = async (next: 'stable' | 'beta') => {
    if (next === channel || switching) return;
    setSwitching(true);
    const prev = channel;
    setChannel(next); // optimistic
    const ok = await ipc.setChannel(next, token);
    if (!ok) {
      setChannel(prev); // revert on failure
    } else {
      // Refresh the product list so the id injected on the next Launch matches
      // the new channel; without this Launch would still use the stale id.
      await onChannelChanged?.(next);
    }
    setSwitching(false);
  };

  const activeApp: AppItem | null = safeApps.length > 0 ? safeApps[0] : null;
  const canLaunch = Boolean(activeApp) && activeApp?.status !== 'Maintenance' && activeApp?.status !== 'Frozen';

  return (
    <WindowShell>
      <TitleBar
        trailing={
          <div className="flex items-center gap-2 rounded-full border border-line bg-ink-2 py-1 pl-1.5 pr-3">
            {user.avatar ? (
              <img
                src={user.avatar}
                alt=""
                className="size-5 rounded-full border border-line-strong object-cover"
                onError={(e) => { e.currentTarget.style.display = 'none'; }}
              />
            ) : (
              <span className="flex size-5 items-center justify-center rounded-full bg-ink-3 text-fg-1">
                <User className="size-3" aria-hidden="true" />
              </span>
            )}
            <span className="text-xs font-semibold text-fg-0">{user.username}</span>
          </div>
        }
      >
        <WeaveMark width={30} className="shrink-0 text-accent" />
        <span className="truncate text-sm font-semibold text-fg-0">{activeApp?.name ?? 'Weave'}</span>
      </TitleBar>

      {/* MAIN: changelog timeline */}
      <main className="flex-1 overflow-y-auto px-6 pb-4 pt-4">
        <div className="mx-auto flex w-full max-w-3xl flex-col gap-3">
          <h2 className="text-sm font-semibold text-fg-0">Changelog</h2>

          {safeChangelogs.length > 0 ? (
            <ol className="relative flex flex-col gap-4 before:absolute before:bottom-2 before:left-[5px] before:top-2 before:w-px before:bg-line">
              {safeChangelogs.map((item, idx) => {
                const latest = idx === 0;
                return (
                  <li key={item.id || idx} className="relative pl-6">
                    <span
                      aria-hidden="true"
                      className={`absolute left-0 top-[5px] size-[11px] rounded-full border-2 ${
                        latest
                          ? 'border-accent bg-accent shadow-[0_0_8px_rgba(255,140,0,0.55)]'
                          : 'border-line-strong bg-ink-0'
                      }`}
                    />
                    <div className="flex items-baseline gap-2">
                      <span className={`font-mono text-[13px] font-semibold ${latest ? 'text-fg-0' : 'text-fg-1'}`}>
                        {item.version || `Update #${safeChangelogs.length - idx}`}
                      </span>
                      {item.title && (
                        <span className={`text-[13px] font-medium ${latest ? 'text-fg-0' : 'text-fg-1'}`}>
                          {item.title}
                        </span>
                      )}
                      {latest && (
                        <span className="rounded-full bg-accent/15 px-1.5 py-px text-[10px] font-semibold uppercase tracking-wide text-accent">
                          Latest
                        </span>
                      )}
                      <span className="ml-auto shrink-0 text-[11px] text-fg-2">{formatDate(item.created_at) || 'Recent'}</span>
                    </div>
                    <p
                      className={`mt-1.5 whitespace-pre-line text-xs leading-relaxed ${
                        latest
                          ? 'rounded-lg border border-line bg-ink-1 px-3 py-2.5 text-fg-1'
                          : 'text-fg-2'
                      }`}
                    >
                      {item.content}
                    </p>
                  </li>
                );
              })}
            </ol>
          ) : (
            <div className="rounded-lg border border-dashed border-line py-8 text-center text-xs text-fg-2">
              No changelog entries yet.
            </div>
          )}
        </div>
      </main>

      {/* FOOTER: status, channel, launch */}
      <footer className="flex h-[72px] shrink-0 items-center gap-4 border-t border-line bg-ink-1 px-6">
        {activeApp?.status === 'Frozen' && (
          <span className="rounded-full border border-amber-500/30 bg-amber-500/10 px-3 py-1 text-[11px] font-semibold text-amber-300">
            Subscription frozen · cannot launch
          </span>
        )}
        {/* Release channel toggle — shown only when the account has beta access */}
        {user.beta_access && (
          <div
            role="radiogroup"
            aria-label="Release channel"
            className="no-drag ml-auto flex items-center rounded-full border border-line bg-ink-2 p-0.5"
          >
            {(['stable', 'beta'] as const).map((ch) => {
              const selected = channel === ch;
              return (
                <button
                  key={ch}
                  type="button"
                  role="radio"
                  aria-checked={selected}
                  onClick={() => changeChannel(ch)}
                  disabled={switching}
                  className={`rounded-full px-3 py-1 text-[11px] font-semibold capitalize transition disabled:opacity-50 focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent ${
                    selected
                      ? ch === 'beta'
                        ? 'bg-accent text-accent-fg'
                        : 'bg-ink-3 text-fg-0'
                      : 'text-fg-2 hover:text-fg-0'
                  }`}
                >
                  {ch}
                </button>
              );
            })}
          </div>
        )}

        <button
          type="button"
          onClick={() => { if (activeApp) onLaunch(activeApp); }}
          disabled={!canLaunch}
          className={`${user.beta_access ? '' : 'ml-auto'} flex h-10 items-center gap-2 rounded-full bg-accent px-7 text-[13px] font-bold uppercase tracking-wide text-accent-fg shadow-[0_0_18px_rgba(255,140,0,0.25)] transition hover:bg-accent-hover active:scale-[0.97] disabled:cursor-not-allowed disabled:opacity-40 disabled:shadow-none focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent focus-visible:ring-offset-2 focus-visible:ring-offset-ink-1`}
        >
          <Play className="size-3.5 fill-current" aria-hidden="true" />
          Launch
        </button>
      </footer>
    </WindowShell>
  );
};
