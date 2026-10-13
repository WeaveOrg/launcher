import { isIP } from 'net';
import { NextRequest, NextResponse } from 'next/server';

const CDN_RU = 'https://cdn.orion-security.su';
const CDN_DEFAULT = 'https://cdn.orion-security.pro';

// The rightmost X-Forwarded-For entry is the one our edge proxy appended.
// The leftmost entry is controlled by the client.
function clientIp(request: NextRequest): string | null {
  const forwarded = request.headers.get('x-forwarded-for');
  if (!forwarded) return null;

  const ip = forwarded.split(',').pop()?.trim() ?? '';
  return isIP(ip) ? ip : null;
}

async function countryAuthority(ip: string): Promise<string | null> {
  const token = process.env.IPINFO_TOKEN;
  if (!token) return null;

  try {
    const res = await fetch(`https://api.ipinfo.io/lite/${ip}`, {
      headers: { Authorization: `Bearer ${token}` },
      cache: 'no-store',
      signal: AbortSignal.timeout(3000),
    });
    if (!res.ok) return null;

    const data = (await res.json()) as { country_code?: unknown };
    if (typeof data.country_code !== 'string') return null;
    return data.country_code === 'RU' ? CDN_RU : CDN_DEFAULT;
  } catch {
    return null;
  }
}

export async function GET(request: NextRequest) {
  const ip = clientIp(request);
  const authority = ip ? await countryAuthority(ip) : null;

  // Only the chosen CDN is returned. The IP and country stay on the server.
  return NextResponse.json({ authority }, { headers: { 'Cache-Control': 'no-store' } });
}
