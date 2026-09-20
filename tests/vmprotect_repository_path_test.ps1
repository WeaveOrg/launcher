$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binary = Join-Path $repoRoot 'tools\VMProtect_Con.exe'
$cmake = Get-Content -Raw (Join-Path $repoRoot 'loader\CMakeLists.txt')

if (-not (Test-Path -LiteralPath $binary)) {
    throw "Repository-bundled VMProtect console is missing: $binary"
}

if ($cmake -notmatch '\$\{CMAKE_SOURCE_DIR\}/tools/VMProtect_Con\.exe') {
    throw 'CMake does not default to the repository-bundled VMProtect console.'
}

Write-Host 'Repository-bundled VMProtect path is configured.'
