import { NextRequest, NextResponse } from 'next/server';

const BACKEND_BASE_URL = 
  process.env.BACKEND_API_URL || 
  process.env.API_BASE_URL || 
  process.env.NEXT_PUBLIC_BACKEND_URL || 
  'http://localhost:4000';

export async function GET(request: NextRequest) {
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
    const productId = request.nextUrl.searchParams.get('product_id');

    // 2. Build target URL with product_id and token query params
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const url = new URL(`${baseUrl}/api/launcher/changelogs`);

    if (productId) {
      url.searchParams.set('product_id', productId);
    }
    if (token) {
      url.searchParams.set('token', token);
      url.searchParams.set('launcher_token', token);
    }

    // 3. Send headers as well
    const headers: Record<string, string> = {
      'Accept': 'application/json',
    };

    if (token) {
      headers['X-Launcher-Token'] = token;
      headers['Authorization'] = `Bearer ${token}`;
    }

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
    console.error('[Proxy Error /api/launcher/changelogs]:', error);
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
