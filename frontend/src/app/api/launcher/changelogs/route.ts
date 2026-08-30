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

    const productId = request.nextUrl.searchParams.get('product_id');

    // 2. Strict backend URL: GET /api/launcher/changelogs?product_id=...
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const url = new URL(`${baseUrl}/api/launcher/changelogs`);

    if (productId) {
      url.searchParams.set('product_id', productId);
    }

    // 3. Strict header: X-Launcher-Token: <launcher_token>
    const headers: Record<string, string> = {
      'Accept': 'application/json',
    };

    if (token) {
      headers['X-Launcher-Token'] = token;
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
