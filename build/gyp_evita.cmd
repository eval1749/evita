@if "%_echo%"=="" echo off
: Create *.ninja files under out/
setlocal

set format=ninja
set mode=debug
set outdir=default

if exist src cd src

if "%cd:~-11,-4%"=="release" set mode=release

for %%x in (%*) do (
  if "%%x"=="msvs" set format=%%x
  if "%%x"=="ninja" set format=%%x
  if "%%x"=="release" set mode=%%x
)

if "%format%"=="msvs" (
  if "%outdir%"=="default" set outdir=..\msvs
)

if "%format%"=="ninja" (
  : Output goes %outdir%\out
  if "%outdir%"=="default" set outdir=..
)

if "%mode%"=="release" (
  set defines=-D component=static_library -D target_arch=x64 %defines%
)

: Check settings
if "%format%"=="default" (
  echo Bad format "%format%"
  goto :EOF
)

if "%outdir%"=="default" (
  echo Bad outdir "%outdir%"
  goto :EOF
)

if not "%defines%"=="" (
  echo Defines: %defines%
)

: Generate files

: Note: Since, base.gyp and base/allocator/allocator.gyp have circular
: dependency. Although, we don't want to use |--no-circular-check|.
: We modify allocator.gyp to remove circular dependency.
: See http://crbug.com/35878.

: Note: There are cycles in .gyp files:
: Cycle: base\allocator\allocator.gyp -> base\base.gyp -> base\allocator\allocator.gyp
: Cycle: base\base.gyp -> base\allocator\allocator.gyp -> base\base.gyp

.\tools\gyp\gyp ^
    --format=%format% ^
    --generator-output=%outdir% ^
    --include build\common.gypi ^
    --no-circular-check ^
    %defines% ^
  build\all.gyp

endlocal
exit/b

: See /tools/gyp/pylib/gyp/MSVSVersion.py
: -G msvs_version=2013
: GYP_MSVS_VERSION=2013|2013e|...
: GYP_MSVS_OVERRIDE_PATH

: tools/gyp debug options:
: --debug=general --debug=includes --debug=variables

