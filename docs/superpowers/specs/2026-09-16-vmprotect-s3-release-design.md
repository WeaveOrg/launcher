# VMProtect and S3 release pipeline

## Goal

On every push to `master`, build the Release configuration, protect the
payload DLL with the repository's VMProtect console executable, create the
final launcher name `loader.exe`, and upload both release files to the root
of the configured S3 bucket.

## Build flow

The repository stores `tools/VMProtect_Con.exe`. CMake uses that executable by
default and keeps `VMPROTECT_CON` as an override for local installations. The
Release build produces `payload_loader.dll`; the `protect_loader` target runs
VMProtect and writes `loader.dll` beside it. The workflow copies
`weave_launcher.exe` to `loader.exe` without changing the CMake target name.

## GitHub Actions

The workflow runs on `push` to `master` and on manual dispatch, uses a Windows
runner, configures the `PRODUCTION` environment, builds Release, verifies both
files exist, and uploads them as `loader.exe` and `loader.dll` to S3 object
keys at the bucket root. Credentials are read from the environment's existing
GitHub Secrets/Variables. An optional endpoint supports S3-compatible storage;
when it is empty, the AWS CLI uses standard AWS S3.

Expected names are `AWS_ACCESS_KEY_ID`, `AWS_SECRET_ACCESS_KEY`, `AWS_REGION`,
`S3_BUCKET`, and optional `S3_ENDPOINT`.

## Failure handling

The workflow stops if configuration, compilation, VMProtect, file validation,
or either upload fails. It does not make files public or delete existing
objects. Uploading to the same root keys replaces those objects according to
the bucket's normal versioning/overwrite policy.

## Verification

The existing local regression script invokes `protect_loader` and checks that
`loader.dll` is created. Local Release build verification checks the same
pipeline that CI will execute. The workflow validates both final artifacts
before calling the S3 CLI.
