# VMProtect and S3 Release Pipeline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Package `loader.exe` and protected `loader.dll` on `master` and upload both files to the root of the configured S3 bucket.

**Architecture:** Keep the existing CMake `payload_loader` target and its `protect_loader` custom target. Store the VMProtect console binary under `tools`, make CMake use that repository-relative path by default, and let a Windows GitHub Actions job build Release, package the two final names, validate them, and upload them with the AWS CLI.

**Tech Stack:** CMake 3.20+, Visual Studio/MSVC on `windows-latest`, VMProtect Console 3.8.1, GitHub Actions, AWS CLI.

**Spec:** `docs/superpowers/specs/2026-09-16-vmprotect-s3-release-design.md`

## Global Constraints

- The workflow runs on `push` to `master` and on manual dispatch.
- GitHub Actions uses the `PRODUCTION` environment.
- The protected DLL object key is `loader.dll` at the S3 bucket root.
- The launcher object key is `loader.exe` at the S3 bucket root.
- Credentials are read from GitHub Secrets/Variables and never written to repository files.
- `S3_ENDPOINT` is optional; when empty, the AWS CLI uses standard AWS S3.
- Existing user changes remain untouched.

### Task 1: Bundle VMProtect and make CMake produce the protected DLL

**Files:**
- Create: `tools/VMProtect_Con.exe` by copying the installed console binary from `E:\vmp\VMProtect_Con.exe`.
- Modify: `loader/CMakeLists.txt` in the VMProtect path configuration.
- Test: `tests/vmprotect_target_test.ps1`.

**Interfaces:**
- Consumes: CMake target `payload_loader`.
- Produces: `protect_loader` target and `build/loader/Release/loader.dll`.

- [ ] **Step 1: Confirm the regression test is red before the repository-bundled path change**

Run:

```powershell
& tests/vmprotect_target_test.ps1
```

Expected: failure because `protect_loader.vcxproj` is not present in the current CMake generation.

- [ ] **Step 2: Copy the VMProtect console binary into the repository tools directory**

Run:

```powershell
New-Item -ItemType Directory -Force tools | Out-Null
Copy-Item -LiteralPath E:\vmp\VMProtect_Con.exe -Destination tools\VMProtect_Con.exe -Force
```

Verify that `tools/VMProtect_Con.exe` exists and has the same byte length as the source binary.

- [ ] **Step 3: Change the CMake default to the repository-relative VMProtect path**

In `loader/CMakeLists.txt`, use:

```cmake
set(VMPROTECT_CON "${CMAKE_SOURCE_DIR}/tools/VMProtect_Con.exe" CACHE FILEPATH
    "Path to the VMProtect console executable")
```

Keep the existing existence check, custom command, and `protect_loader ALL` dependency so a normal Release build invokes VMProtect and fails if protection fails.

- [ ] **Step 4: Reconfigure and run the regression test**

Run:

```powershell
cmake -S . -B build
& tests/vmprotect_target_test.ps1
```

Expected: exit code `0` and an existing `build/loader/Release/loader.dll`.

- [ ] **Step 5: Commit the local build integration**

```powershell
git add -- tools/VMProtect_Con.exe loader/CMakeLists.txt tests/vmprotect_target_test.ps1
git commit -m "build: bundle VMProtect protection step"
```

### Task 2: Add the production GitHub Actions workflow

**Files:**
- Create: `.github/workflows/production-release.yml`.

**Interfaces:**
- Consumes: `PRODUCTION` environment values `AWS_REGION`, `S3_BUCKET`, optional `S3_ENDPOINT`, and secrets `AWS_ACCESS_KEY_ID`, `AWS_SECRET_ACCESS_KEY`.
- Produces: S3 objects `loader.exe` and `loader.dll` at the bucket root.

- [ ] **Step 1: Add a workflow triggered only for production release events**

Use this event/job shape:

```yaml
name: Production release

on:
  push:
    branches: [master]
  workflow_dispatch:

permissions:
  contents: read

jobs:
  release:
    runs-on: windows-latest
    environment: PRODUCTION
```

- [ ] **Step 2: Configure, build, and package the Release artifacts**

The job must checkout the repository, configure CMake for x64 Visual Studio, run `cmake --build build --config Release --parallel 2`, copy `build/Release/weave_launcher.exe` to `build/Release/loader.exe`, and verify both `loader.exe` and `build/loader/Release/loader.dll` exist before upload.

- [ ] **Step 3: Configure AWS credentials without exposing them**

Use `aws-actions/configure-aws-credentials@v4` with:

```yaml
with:
  aws-access-key-id: ${{ secrets.AWS_ACCESS_KEY_ID }}
  aws-secret-access-key: ${{ secrets.AWS_SECRET_ACCESS_KEY }}
  aws-region: ${{ vars.AWS_REGION }}
```

Pass `S3_BUCKET` and optional `S3_ENDPOINT` through the job environment. Do not echo credential variables or include them in command arguments.

- [ ] **Step 4: Upload both files to the S3 root**

Use PowerShell to append `--endpoint-url $env:S3_ENDPOINT` only when the optional endpoint is non-empty, then run:

```powershell
aws s3 cp build\Release\loader.exe "s3://$env:S3_BUCKET/loader.exe"
aws s3 cp build\loader\Release\loader.dll "s3://$env:S3_BUCKET/loader.dll"
```

The job must fail on either upload failure.

- [ ] **Step 5: Commit the workflow**

```powershell
git add -- .github/workflows/production-release.yml
git commit -m "ci: publish protected loaders to S3"
```

### Task 3: Verify the complete local packaging path

**Files:**
- Modify: none.
- Test: generated files under `build/loader/Release` and `build/Release`.

**Interfaces:**
- Consumes: the CMake and workflow changes from Tasks 1 and 2.
- Produces: verified local artifacts with the exact CI names.

- [ ] **Step 1: Force the protection step to execute through the normal build**

Remove only the generated file `build/loader/Release/loader.dll`, then run:

```powershell
cmake --build build --config Release --parallel 4
```

Expected output includes `Protecting payload_loader.dll with VMProtect`, and the command exits with code `0`.

- [ ] **Step 2: Create and validate the local `loader.exe` package name**

Run:

```powershell
Copy-Item -LiteralPath build\Release\weave_launcher.exe -Destination build\Release\loader.exe -Force
Get-Item build\Release\loader.exe, build\loader\Release\loader.dll | Select-Object FullName,Length
```

Expected: both files exist and have non-zero sizes.

- [ ] **Step 3: Review the final diff and workflow references**

Run:

```powershell
git diff --check
rg -n "PRODUCTION|AWS_REGION|S3_BUCKET|S3_ENDPOINT|loader\.exe|loader\.dll|VMProtect_Con" .github loader tools
```

Confirm that no credentials, local `E:\vmp` path, or unrelated files were added to the workflow.
