import type { Metadata } from 'next';
import './globals.css';
import ZoomLock from '@/components/ZoomLock';

export const metadata: Metadata = {
  title: 'Weave Launcher - Modern Native Game & Modding Client',
  description: 'Powered by Saucer C++ & Next.js',
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className="dark">
      <body className="h-screen w-screen select-none overflow-hidden bg-ink-0 text-fg-0 antialiased">
        <ZoomLock />
        {children}
      </body>
    </html>
  );
}
