@if "%_echo%"=="" echo off
setlocal
set outdir=..\out.gn\debug
set which=%1
set pattern=%2
set options=--single-process-tests --gtest_break_on_failure --gtest_throw_on_failure

if "%which%"=="" goto usage
if "%pattern%"=="" goto usage

set target=%outdir%\evita_%which%_tests.exe
if not exist %target% goto not_fond
start devenv.exe /debugexe %target% %options% --gtest_filter=%pattern%

endlocal
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
