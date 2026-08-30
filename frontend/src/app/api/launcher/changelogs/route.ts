import { NextRequest, NextResponse } from 'next/server';

const BACKEND_BASE_URL = 
  process.env.BACKEND_API_URL || 
  process.env.API_BASE_URL || 
  process.env.NEXT_PUBLIC_BACKEND_URL || 
  'http://localhost:4000';

export async function GET(request: NextRequest) {
  try {
    const token = request.headers.get('x-launcher-token') || request.headers.get('authorization') || '';
    const productId = request.nextUrl.searchParams.get('product_id');

    // Normalize trailing slash from base url
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const url = new URL(`${baseUrl}/api/launcher/changelogs`);

    if (productId) {
      url.searchParams.set('product_id', productId);
    }

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

    const data = await response.json().catch(() => null);

    if (!response.ok) {
      return NextResponse.json(
        data || { error: 'Failed to fetch changelogs from backend' },
        { status: response.status }
      );
    }

    return NextResponse.json(data, { status: 200 });
  } catch (error: any) {
    console.error('[Proxy Error /api/launcher/changelogs]:', error);
    return NextResponse.json(
      { error: 'Backend server is unreachable', message: error?.message },
      { status: 502 }
    );
  }
}
