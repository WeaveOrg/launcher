'use client';

import { useEffect } from 'react';

// Blocks browser zoom (ctrl+wheel, ctrl +/-/0, pinch) so the launcher UI
// keeps a fixed scale inside the WebView2 window.
export default function ZoomLock() {
  useEffect(() => {
    const onWheel = (e: WheelEvent) => {
      if (e.ctrlKey || e.metaKey) {
        e.preventDefault();
      }
    };

    const onKeyDown = (e: KeyboardEvent) => {
      if (!(e.ctrlKey || e.metaKey)) return;
      if (['+', '=', '-', '_', '0'].includes(e.key)) {
        e.preventDefault();
      }
    };

    const onGesture = (e: Event) => e.preventDefault();

    window.addEventListener('wheel', onWheel, { passive: false });
    window.addEventListener('keydown', onKeyDown);
    window.addEventListener('gesturestart', onGesture);
    window.addEventListener('gesturechange', onGesture);
    window.addEventListener('gestureend', onGesture);

    return () => {
      window.removeEventListener('wheel', onWheel);
      window.removeEventListener('keydown', onKeyDown);
      window.removeEventListener('gesturestart', onGesture);
      window.removeEventListener('gesturechange', onGesture);
      window.removeEventListener('gestureend', onGesture);
    };
  }, []);

  return null;
}
