Write-Host ">>> Starting Weave Backend Loader Server..." -ForegroundColor Cyan
Set-Location -Path "$PSScriptRoot\..\backend_server"
node server.js
