@if "%_echo%"=="" echo off
: Create *.ninja files under out/
setlocal

: See /tools/gyp/pylib/gyp/MSVSVersion.py
: -G msvs_version=2013
: GYP_MSVS_VERSION=2013|2013e|...
: GYP_MSVS_OVERRIDE_PATH

.\tools\gyp\gyp ^
  --debug=general --debug=includes --debug=variables ^
    --format=ninja ^
    --generator-output=.. ^
    --include build\common.gypi ^
  build\all.gyp

endlocal
exit/b
