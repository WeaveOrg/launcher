import type { Metadata } from 'next';
import './globals.css';

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
      <body className="bg-[#0a0a0f] text-slate-100 antialiased select-none overflow-hidden h-screen w-screen border border-purple-500/20 rounded-xl">
        {children}
      </body>
    </html>
  );
}
