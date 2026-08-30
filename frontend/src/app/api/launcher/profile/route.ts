import { NextRequest, NextResponse } from 'next/server';

const BACKEND_BASE_URL = 
  process.env.BACKEND_API_URL || 
  process.env.API_BASE_URL || 
  process.env.NEXT_PUBLIC_BACKEND_URL || 
  'http://localhost:4000';

export async function GET(request: NextRequest) {
  try {
    // 1. Extract token from query param, cookies, or header
    const token = 
      request.nextUrl.searchParams.get('token') ||
      request.nextUrl.searchParams.get('launcher_token') ||
      request.cookies.get('launcher_token')?.value ||
      request.headers.get('x-launcher-token') || 
      request.headers.get('authorization')?.replace(/^Bearer\s+/i, '') ||
      '';

    if (!token) {
      return NextResponse.json(
        { 
          error: 'Missing launcher token', 
          message: 'Please provide ?token=<launcher_token> parameter.' 
        },
        { status: 401 }
      );
    }

    // 2. Strict backend URL: GET /api/launcher/profile
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const targetUrl = `${baseUrl}/api/launcher/profile`;

    // 3. Strict header: X-Launcher-Token: <launcher_token>
    const headers: Record<string, string> = {
      'Accept': 'application/json',
      'X-Launcher-Token': token,
    };

    const response = await fetch(targetUrl, {
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
          targetUrl,
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
