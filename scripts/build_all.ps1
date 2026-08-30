Write-Host ">>> [1/2] Building Next.js Frontend Static Distribution..." -ForegroundColor Cyan
Set-Location -Path "$PSScriptRoot\..\frontend"
npm run build

Write-Host "`n>>> [2/2] Building C++ Native Launcher Core..." -ForegroundColor Cyan
Set-Location -Path "$PSScriptRoot\.."
cmake -B build -G "Ninja" -DCMAKE_CXX_COMPILER=g++
cmake --build build

Write-Host "`n>>> [BUILD COMPLETE] Weave Launcher built successfully!" -ForegroundColor Green
