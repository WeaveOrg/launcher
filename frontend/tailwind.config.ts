import type { Config } from "tailwindcss";

const config: Config = {
  content: [
    "./src/pages/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/components/**/*.{js,ts,jsx,tsx,mdx}",
    "./src/app/**/*.{js,ts,jsx,tsx,mdx}",
  ],
  theme: {
    extend: {
      // Tokens live in globals.css; keep the two in sync.
      colors: {
        background: "var(--background)",
        foreground: "var(--foreground)",
        ink: {
          0: "rgb(var(--ink-0) / <alpha-value>)",
          1: "rgb(var(--ink-1) / <alpha-value>)",
          2: "rgb(var(--ink-2) / <alpha-value>)",
          3: "rgb(var(--ink-3) / <alpha-value>)",
        },
        line: {
          DEFAULT: "rgb(var(--line) / <alpha-value>)",
          strong: "rgb(var(--line-strong) / <alpha-value>)",
        },
        fg: {
          0: "rgb(var(--fg-0) / <alpha-value>)",
          1: "rgb(var(--fg-1) / <alpha-value>)",
          2: "rgb(var(--fg-2) / <alpha-value>)",
        },
        accent: {
          DEFAULT: "rgb(var(--accent) / <alpha-value>)",
          hover: "rgb(var(--accent-hover) / <alpha-value>)",
          fg: "rgb(var(--accent-fg) / <alpha-value>)",
        },
        ok: "rgb(var(--ok) / <alpha-value>)",
        warn: "rgb(var(--warn) / <alpha-value>)",
        danger: "rgb(var(--danger) / <alpha-value>)",
      },
      fontFamily: {
        sans: ['"Segoe UI Variable Text"', '"Segoe UI"', 'Inter', 'system-ui', 'sans-serif'],
        mono: ['"Cascadia Mono"', 'Consolas', '"JetBrains Mono"', 'monospace'],
      },
    },
  },
  plugins: [],
};
export default config;
