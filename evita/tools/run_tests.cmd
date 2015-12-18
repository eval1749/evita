@if "%_echo%"=="" echo off
setlocal
set outdir=..\out.gn\debug
set options=--single-process-tests --test-launcher-retry-limit=0

set which=%1
set pattern=%2
if "%which%"=="" goto usage


set filter=
set target=%outdir%\evita_%which%_tests.exe

if not exist %target% goto not_fond
if not "%pattern%"=="" set filter=--gtest_filter=%pattern%

:// We should use single process for running test, because initialization
:// of V8 takes 2 to 3 seconds for each process.
%target% %options% %filter%
exit/b

:not_found
echo No such tests: %which%
exit/b

:usage
echo Usage: %~nx0 which [pattern]
echo which is one of
for /f "usebackq delims=_ tokens=2" %%x in (`dir/b %outdir%\evita_*_tests.exe`) do (
  echo %space% %space% %%x
)
exit/b
