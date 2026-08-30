import { NextRequest, NextResponse } from 'next/server';

const BACKEND_BASE_URL = 
  process.env.BACKEND_API_URL || 
  process.env.API_BASE_URL || 
  process.env.NEXT_PUBLIC_BACKEND_URL || 
  'http://localhost:4000';

export async function POST(request: NextRequest) {
  try {
    const body = await request.json().catch(() => ({}));
    const baseUrl = BACKEND_BASE_URL.replace(/\/+$/, '');
    const targetUrl = `${baseUrl}/api/v1/auth/login`;

    const headers: Record<string, string> = {
      'Content-Type': 'application/json',
      'Accept': 'application/json',
    };

    const token = request.headers.get('x-launcher-token') || request.headers.get('authorization');
    if (token) headers['X-Launcher-Token'] = token;

    const response = await fetch(targetUrl, {
      method: 'POST',
      headers,
      body: JSON.stringify(body),
      cache: 'no-store',
    });

    const data = await response.json().catch(() => null);

    return NextResponse.json(data || {}, { status: response.status });
  } catch (error: any) {
    console.error('[Proxy Error /api/v1/auth/login]:', error);
    return NextResponse.json(
      { success: false, message: 'Backend auth server unreachable', error: error?.message },
      { status: 502 }
    );
  }
}
