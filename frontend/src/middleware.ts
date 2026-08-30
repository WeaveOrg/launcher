import { NextResponse } from 'next/server';
import type { NextRequest } from 'next/server';

export function middleware(request: NextRequest) {
  const token = request.nextUrl.searchParams.get('token') || request.nextUrl.searchParams.get('launcher_token');
  const response = NextResponse.next();

  if (token) {
    // Automatically set cookie for all client & server requests
    response.cookies.set('launcher_token', token, {
      path: '/',
      sameSite: 'lax',
      maxAge: 60 * 60 * 24 * 30, // 30 days
      httpOnly: false, // accessible to client js as well
    });
  }

  return response;
}

export const config = {
  matcher: [
    /*
     * Match all request paths except for static files
     */
    '/((?!_next/static|_next/image|favicon.ico).*)',
  ],
};
