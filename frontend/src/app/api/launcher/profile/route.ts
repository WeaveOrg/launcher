import { NextRequest, NextResponse } from 'next/server';
import { MOCK_ENABLED, mockProfile } from '@/lib/mock';

const BACKEND_BASE_URL = 
  process.env.BACKEND_API_URL || 
  process.env.API_BASE_URL || 
  process.env.NEXT_PUBLIC_BACKEND_URL || 
  'http://localhost:4000';

export async function GET(request: NextRequest) {
  if (MOCK_ENABLED) {
    // `?token=stale` renders the mandatory-update gate; `?token=slow` holds
    // the response so the splash screen can be inspected.
    const token = request.nextUrl.searchParams.get('token');
    if (token === 'slow') {
      await new Promise((r) => setTimeout(r, 60_000));
    }
    const stale = token === 'stale';
    return NextResponse.json(
      stale
        ? { ...mockProfile, launcher_downloaded_version: '1.3.0', launcher_update_required: true }
        : mockProfile,
    );
  }
  try {
    // 1. Extract token from query param, cookies, or header
    const rawToken = 
      request.nextUrl.searchParams.get('token') ||
      request.nextUrl.searchParams.get('launcher_token') ||
      request.cookies.get('launcher_token')?.value ||
      request.headers.get('x-launcher-token') || 
      request.headers.get('authorization')?.replace(/^Bearer\s+/i, '') ||
      '';

    const token = rawToken.trim().replace(/^['"]|['"]$/g, '');

    if (!token) {
      return NextResponse.json(
        { 
          error: 'Missing launcher token', 
          message: 'Please provide ?token=<launcher_token> parameter.' 
        },
        { status: 401 }
      );
    }

    // 2. Build target URL with token in query params
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const url = new URL(`${baseUrl}/api/launcher/profile`);
    url.searchParams.set('token', token);
    url.searchParams.set('launcher_token', token);

    // 3. Send headers as well (covers all backend implementations)
    const headers: Record<string, string> = {
      'Accept': 'application/json',
      'X-Launcher-Token': token,
      'Authorization': `Bearer ${token}`,
    };

    const response = await fetch(url.toString(), {
      method: 'GET',
      headers,
      cache: 'no-store',
    });

    const responseText = await response.text();
    let data: any = null;
    try {
      data = JSON.parse(responseText);
    } catch {
      data = null;
    }

    if (!response.ok) {
      return NextResponse.json(
        data || { 
          error: 'Backend returned non-OK status', 
          status: response.status,
          targetUrl: url.toString(),
          details: responseText || response.statusText 
        },
        { status: response.status }
      );
    }

    return NextResponse.json(data || responseText, { status: 200 });
  } catch (error: any) {
    console.error('[Proxy Error /api/launcher/profile]:', error);
    return NextResponse.json(
      { 
        error: 'Backend server is unreachable', 
        backendUrl: BACKEND_BASE_URL,
        message: error?.message 
      },
      { status: 502 }
    );
  }
}
