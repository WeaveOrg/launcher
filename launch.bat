@echo off
title Weave Launcher
echo Starting Weave Backend Loader Server...
start "" /B node backend_server\server.js

echo Starting Weave Desktop Interface...
start "" "http://localhost:3001"

echo Weave Launcher is running.
