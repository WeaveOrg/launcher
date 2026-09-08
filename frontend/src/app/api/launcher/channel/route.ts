import { NextRequest, NextResponse } from 'next/server';

const BACKEND_BASE_URL =
  process.env.BACKEND_API_URL ||
  process.env.API_BASE_URL ||
  process.env.NEXT_PUBLIC_BACKEND_URL ||
  'http://localhost:4000';

// Proxy for the launcher's release-channel switch. Mirrors the profile proxy:
// the launcher token rides in query and header, the body carries {channel}.
export async function PUT(request: NextRequest) {
  try {
    const rawToken =
      request.nextUrl.searchParams.get('token') ||
      request.nextUrl.searchParams.get('launcher_token') ||
      request.cookies.get('launcher_token')?.value ||
      request.headers.get('x-launcher-token') ||
      request.headers.get('authorization')?.replace(/^Bearer\s+/i, '') ||
      '';

    const token = rawToken.trim().replace(/^['"]|['"]$/g, '');

    if (!token) {
      return NextResponse.json({ error: 'Missing launcher token' }, { status: 401 });
    }

    const body = await request.json().catch(() => ({}));

    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const url = new URL(`${baseUrl}/api/launcher/channel`);
    url.searchParams.set('token', token);
    url.searchParams.set('launcher_token', token);

    const response = await fetch(url.toString(), {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
        'X-Launcher-Token': token,
        'Authorization': `Bearer ${token}`,
      },
      body: JSON.stringify(body),
      cache: 'no-store',
    });

    const responseText = await response.text();
    let data: unknown = null;
    try {
      data = JSON.parse(responseText);
    } catch {
      data = null;
    }

    return NextResponse.json(data ?? { error: responseText || response.statusText }, {
      status: response.status,
    });
  } catch (error: unknown) {
    console.error('[Proxy Error /api/launcher/channel]:', error);
    return NextResponse.json(
      { error: 'Backend server is unreachable', backendUrl: BACKEND_BASE_URL },
      { status: 502 }
    );
  }
}
