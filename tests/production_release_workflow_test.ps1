$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$workflowPath = Join-Path $repoRoot '.github\workflows\production-release.yml'

if (-not (Test-Path -LiteralPath $workflowPath)) {
    throw "Production release workflow is missing: $workflowPath"
}

$workflow = Get-Content -Raw $workflowPath
$requiredPatterns = @(
    'branches:\s*\[master\]',
    'workflow_dispatch:',
    'environment:\s*PRODUCTION',
    'AWS_ACCESS_KEY_ID',
    'AWS_SECRET_ACCESS_KEY',
    'AWS_REGION',
    'S3_BUCKET',
    'S3_ENDPOINT',
    'Visual Studio 18 2026',
    'loader\.exe',
    'loader\.dll',
    'aws s3 cp'
)

foreach ($pattern in $requiredPatterns) {
    if ($workflow -notmatch $pattern) {
        throw "Production release workflow is missing required pattern: $pattern"
    }
}

if ($workflow -match 'Visual Studio 17 2022') {
    throw 'Production release workflow still requests the unavailable Visual Studio 2022 generator.'
}

Write-Host 'Production release workflow contract is valid.'
