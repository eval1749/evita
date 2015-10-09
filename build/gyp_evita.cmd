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

: We use "icudt.dll".
: set defines=%defines% -D icu_use_data_file_flag=0

: We don't use V8's external startup file.
set defines=%defines% -D v8_use_external_startup_data=0 -D win_fastlink=1

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

: Note: We need to have
: "C:\Program Files (x86)\Windows Kits\8.1\Bin\SetEnv.Cmd"'
: it is used by vs.SetupScript() in GenerateEnvironmentFiles()


set PYTHONPATH=%cd%\build;%PATH%

: Please try --no-parallel, when gyp failures.
.\tools\gyp\gyp ^
    --format=%format% ^
    --generator-output=%outdir% ^
    --include build\common.gypi ^
    --no-circular-check ^
    %defines% ^
  build\evita_all.gyp

endlocal
exit/b

: See /tools/gyp/pylib/gyp/MSVSVersion.py
: -G msvs_version=2013
: GYP_MSVS_VERSION=2013|2013e|...
: GYP_MSVS_OVERRIDE_PATH

: tools/gyp debug options:
: --debug=general --debug=includes --debug=variables

