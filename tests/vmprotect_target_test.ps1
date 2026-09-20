$ErrorActionPreference = 'Stop'

$buildDir = Join-Path $PSScriptRoot '..\build'
$result = & cmake --build $buildDir --config Release --target protect_loader 2>&1
if ($LASTEXITCODE -ne 0) {
    $result | Write-Host
    throw "CMake target protect_loader is unavailable or failed."
}

$output = Join-Path $buildDir 'loader\Release\loader.dll'
if (-not (Test-Path -LiteralPath $output)) {
    throw "Expected VMProtect output was not created: $output"
}

Write-Host "VMProtect output exists: $output"
