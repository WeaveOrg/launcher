'use client';

import React, { useState } from 'react';
import { 
  Play, 
  ShieldCheck, 
  Flame, 
  Crosshair, 
  Zap, 
  Shield, 
  Eye, 
  Box, 
  Users, 
  Star, 
  Search,
  Filter,
  CheckCircle2,
  RefreshCw
} from 'lucide-react';
import { AppItem } from '@/lib/ipc';

interface LibraryViewProps {
  apps: AppItem[];
  onLaunch: (app: AppItem) => void;
  onRefresh: () => void;
  isLoading: boolean;
}

export const LibraryView: React.FC<LibraryViewProps> = ({
  apps,
  onLaunch,
  onRefresh,
  isLoading
}) => {
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedCategory, setSelectedCategory] = useState('ALL');

  const getIcon = (iconName: string) => {
    switch (iconName) {
      case 'Crosshair': return <Crosshair className="w-5 h-5" />;
      case 'Flame': return <Flame className="w-5 h-5" />;
      case 'Zap': return <Zap className="w-5 h-5" />;
      case 'Shield': return <Shield className="w-5 h-5" />;
      case 'Eye': return <Eye className="w-5 h-5" />;
      case 'Box': return <Box className="w-5 h-5" />;
      default: return <ShieldCheck className="w-5 h-5" />;
    }
  };

  const filteredApps = apps.filter((app) => {
    const matchesSearch = app.name.toLowerCase().includes(searchQuery.toLowerCase()) ||
      app.category.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesCat = selectedCategory === 'ALL' || app.category.includes(selectedCategory);
    return matchesSearch && matchesCat;
  });

  return (
    <div className="flex-1 flex flex-col overflow-hidden p-6 space-y-6">
      {/* Top Controls Bar */}
      <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
        <div>
          <h1 className="text-2xl font-black tracking-tight text-white flex items-center gap-2">
            Target Library
            <span className="text-xs font-mono font-normal text-purple-400 px-2 py-0.5 rounded-full bg-purple-950/80 border border-purple-500/30">
              {apps.length} Modules Available
            </span>
          </h1>
          <p className="text-xs text-slate-400 mt-0.5">
            Select a software target to initialize driver injection and cloud profile sync.
          </p>
        </div>

        <div className="flex items-center gap-3 w-full sm:w-auto">
          {/* Search bar */}
          <div className="relative flex-1 sm:w-64">
            <Search className="w-4 h-4 absolute left-3 top-1/2 -translate-y-1/2 text-slate-400" />
            <input
              type="text"
              placeholder="Search target or category..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              className="w-full bg-[#13141f] border border-white/10 rounded-xl pl-9 pr-3 py-2 text-xs text-slate-200 placeholder:text-slate-500 focus:outline-none focus:border-purple-500/50 transition"
            />
          </div>

          <button
            onClick={onRefresh}
            className="p-2.5 bg-[#13141f] hover:bg-white/10 border border-white/10 text-slate-300 rounded-xl transition"
            title="Refresh Manifests"
          >
            <RefreshCw className={`w-4 h-4 ${isLoading ? 'animate-spin text-purple-400' : ''}`} />
          </button>
        </div>
      </div>

      {/* Grid of Apps */}
      <div className="flex-1 overflow-y-auto pr-1 grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-5">
        {filteredApps.map((app) => {
          const isUndetected = app.status === 'Undetected';
          return (
            <div
              key={app.id}
              className="group relative rounded-2xl bg-[#11121c] border border-white/5 hover:border-purple-500/40 transition-all duration-300 flex flex-col overflow-hidden shadow-lg hover:shadow-purple-500/10 hover:-translate-y-1"
            >
              {/* Banner Image with gradient overlay */}
              <div className="h-36 relative w-full overflow-hidden bg-slate-900">
                <img
                  src={app.banner}
                  alt={app.name}
                  className="w-full h-full object-cover transition-transform duration-500 group-hover:scale-105 opacity-80 group-hover:opacity-95"
                />
                <div className="absolute inset-0 bg-gradient-to-t from-[#11121c] via-[#11121c]/40 to-transparent" />
                
                {/* Status Badge */}
                <div className="absolute top-3 right-3 flex items-center gap-1.5 px-2.5 py-1 rounded-full bg-black/60 backdrop-blur-md border border-white/10 text-[11px] font-mono font-medium text-slate-200">
                  <span
                    className="w-2 h-2 rounded-full"
                    style={{ backgroundColor: app.statusColor }}
                  />
                  <span>{app.status}</span>
                </div>

                {/* Category Tag */}
                <div className="absolute bottom-3 left-3 flex items-center gap-2">
                  <div className="w-8 h-8 rounded-xl bg-purple-600/80 backdrop-blur-md flex items-center justify-center text-white shadow-md">
                    {getIcon(app.icon)}
                  </div>
                  <div>
                    <h3 className="font-bold text-white text-base leading-none">{app.name}</h3>
                    <span className="text-[11px] text-purple-300 font-mono">{app.version}</span>
                  </div>
                </div>
              </div>

              {/* Card Body */}
              <div className="p-4 flex-1 flex flex-col justify-between space-y-4">
                <div>
                  <p className="text-xs text-slate-400 line-clamp-2 leading-relaxed">
                    {app.description}
                  </p>

                  {/* Feature Pills */}
                  <div className="flex flex-wrap gap-1.5 mt-3">
                    {app.features.slice(0, 3).map((feat, idx) => (
                      <span
                        key={idx}
                        className="text-[10px] px-2 py-0.5 rounded-md bg-purple-950/40 border border-purple-500/20 text-purple-300 font-mono"
                      >
                        {feat}
                      </span>
                    ))}
                    {app.features.length > 3 && (
                      <span className="text-[10px] px-1.5 py-0.5 rounded-md bg-white/5 text-slate-400 font-mono">
                        +{app.features.length - 3}
                      </span>
                    )}
                  </div>
                </div>

                {/* Footer Info & Launch Button */}
                <div className="pt-3 border-t border-white/5 flex items-center justify-between">
                  <div className="flex items-center gap-3 text-slate-400 text-xs font-mono">
                    <div className="flex items-center gap-1">
                      <Users className="w-3.5 h-3.5 text-purple-400" />
                      <span>{app.activeUsers}</span>
                    </div>
                    <div className="flex items-center gap-1">
                      <Star className="w-3.5 h-3.5 text-amber-400 fill-amber-400" />
                      <span>{app.rating}</span>
                    </div>
                  </div>

                  <button
                    onClick={() => onLaunch(app)}
                    disabled={!isUndetected}
                    className={`flex items-center gap-1.5 px-4 py-2 rounded-xl text-xs font-bold transition shadow-md ${
                      isUndetected
                        ? 'bg-gradient-to-r from-purple-600 to-indigo-600 hover:from-purple-500 hover:to-indigo-500 text-white shadow-purple-600/25 hover:shadow-purple-600/40 cursor-pointer active:scale-95'
                        : 'bg-slate-800 text-slate-500 cursor-not-allowed border border-white/5'
                    }`}
                  >
                    <Play className="w-3.5 h-3.5 fill-current" />
                    <span>{isUndetected ? 'LAUNCH' : 'UPDATING'}</span>
                  </button>
                </div>
              </div>
            </div>
          );
        })}
      </div>
    </div>
  );
};
