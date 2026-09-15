'use client';

import { AlertTriangle, Download, ExternalLink } from 'lucide-react';

const dashboardUrl = 'https://weave.su/dashboard/products';

interface LegacyLauncherNoticeProps {
  latestVersion?: string;
  downloadedVersion?: string;
}

export function LegacyLauncherNotice({ latestVersion, downloadedVersion }: LegacyLauncherNoticeProps) {
  const currentLabel = downloadedVersion || 'an older build';
  const latestLabel = latestVersion || 'the current build';

  return (
    <main
      className="relative flex h-screen w-screen items-center justify-center overflow-hidden bg-[#0d0d0d] px-6 py-8 text-[#ddd]"
      aria-labelledby="legacy-launcher-title"
    >
      <div className="pointer-events-none absolute inset-0 bg-[radial-gradient(circle_at_top,rgba(255,140,0,0.14),transparent_42%)]" />
      <section className="relative w-full max-w-lg rounded-2xl border border-[#3c2a18] bg-[#121212] p-7 shadow-[0_24px_80px_rgba(0,0,0,0.45)] sm:p-9">
        <div className="mb-6 flex size-12 items-center justify-center rounded-xl border border-[#ff8c00]/35 bg-[#ff8c00]/10 text-[#ff9d2e] shadow-[0_0_28px_rgba(255,140,0,0.14)]">
          <AlertTriangle className="size-6" aria-hidden="true" />
        </div>

        <p className="text-xs font-bold uppercase tracking-[0.18em] text-[#ff9d2e]">Update Required</p>
        <h1 id="legacy-launcher-title" className="mt-2 text-balance text-2xl font-bold tracking-tight text-white sm:text-3xl">
          This Launcher Version Is No Longer Supported
        </h1>
        <p className="mt-3 text-sm leading-6 text-[#aaa]">
          Download the latest launcher from your dashboard to continue using Weave. Launching is unavailable until the update is installed.
        </p>

        <dl className="mt-6 grid gap-3 rounded-xl border border-[#292929] bg-[#0d0d0d] p-4 text-sm sm:grid-cols-2">
          <div>
            <dt className="text-[11px] font-semibold uppercase tracking-wider text-[#6d6d6d]">Issued version</dt>
            <dd className="mt-1 break-all font-mono text-[#c4c4c4]">{currentLabel}</dd>
          </div>
          <div>
            <dt className="text-[11px] font-semibold uppercase tracking-wider text-[#6d6d6d]">Latest version</dt>
            <dd className="mt-1 break-all font-mono text-white">{latestLabel}</dd>
          </div>
        </dl>

        <a
          href={dashboardUrl}
          target="_blank"
          rel="noreferrer"
          className="mt-7 flex w-full items-center justify-center gap-2 rounded-xl bg-[#ff8c00] px-5 py-3 text-sm font-extrabold text-black transition-[background-color,transform] hover:bg-[#ffa02b] focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-[#ffb35c] focus-visible:ring-offset-2 focus-visible:ring-offset-[#121212] active:scale-[0.99] motion-reduce:transform-none motion-reduce:transition-none"
          aria-label="Open Weave dashboard products page to download the latest launcher"
        >
          <Download className="size-4" aria-hidden="true" />
          Download Latest Launcher
          <ExternalLink className="size-3.5" aria-hidden="true" />
        </a>
      </section>
    </main>
  );
}
