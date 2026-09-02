/** @type {import('next').NextConfig} */
const nextConfig = {
  output: 'standalone',
  reactStrictMode: false,
  // Unique build ID per deployment to invalidate cached chunks
  generateBuildId: async () => {
    return `weave-build-${Date.now()}`;
  },
  async headers() {
    return [
      {
        source: '/(.*)',
        headers: [
          {
            key: 'Cache-Control',
            value: 'no-cache, no-store, max-age=0, must-revalidate',
          },
        ],
      },
    ];
  },
  async rewrites() {
    return [
      {
        source: '/api/:path*',
        destination: 'http://backend:4000/api/:path*', // Proxy API requests
      },
      {
        source: '/ws',
        destination: 'http://backend:4000/ws', // Proxy WebSocket connections
      },
    ];
  },
};

export default nextConfig;
