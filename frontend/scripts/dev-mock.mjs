// Starts `next dev` with the mock API enabled (see src/lib/mock.ts).
// Kept as a script so it works on Windows without cross-env.
import { spawn } from 'node:child_process';

const child = spawn(
  process.platform === 'win32' ? 'npx.cmd' : 'npx',
  ['next', 'dev', '-p', '3000'],
  { stdio: 'inherit', shell: process.platform === 'win32', env: { ...process.env, NEXT_PUBLIC_MOCK_API: '1' } },
);
child.on('exit', (code) => process.exit(code ?? 0));
