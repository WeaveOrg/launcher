'use client';

import React from 'react';
import { Newspaper, Calendar, User, ArrowUpRight } from 'lucide-react';
import { NewsItem } from '@/lib/ipc';

interface NewsFeedProps {
  news: NewsItem[];
}

export const NewsFeed: React.FC<NewsFeedProps> = ({ news }) => {
  return (
    <div className="flex-1 flex flex-col overflow-y-auto p-6 space-y-6 select-none">
      <div>
        <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
          Updates & Changelogs
          <span className="text-xs font-mono font-normal text-indigo-400 px-2 py-0.5 rounded-full bg-indigo-950/80 border border-indigo-500/30">
            Feed
          </span>
        </h1>
        <p className="text-xs text-slate-400 mt-0.5">
          Real-time security updates, patch notes, and engine announcements from the Weave core team.
        </p>
      </div>

      <div className="space-y-4">
        {news.map((item) => (
          <div
            key={item.id}
            className="p-5 rounded-2xl bg-[#11121c] border border-white/5 hover:border-purple-500/30 transition duration-300 space-y-3"
          >
            <div className="flex items-center justify-between">
              <span className={`text-[10px] font-mono font-bold px-2.5 py-1 rounded-md border ${item.tagColor}`}>
                {item.tag}
              </span>
              <div className="flex items-center gap-4 text-xs text-slate-500 font-mono">
                <span className="flex items-center gap-1">
                  <Calendar className="w-3.5 h-3.5" />
                  {item.date}
                </span>
                <span className="flex items-center gap-1">
                  <User className="w-3.5 h-3.5 text-purple-400" />
                  {item.author}
                </span>
              </div>
            </div>

            <h2 className="text-base font-bold text-white leading-snug">{item.title}</h2>
            <p className="text-xs text-slate-300 leading-relaxed">{item.content}</p>
          </div>
        ))}
      </div>
    </div>
  );
};
