@if "%_echo%"=="" echo off
call ../setenv.cmd
if "%outdir%"=="" goto no_outdir

setlocal
setlocal enabledelayedexpansion

set tests=dom layout text
set test_log=%TEMP%\evita_all_tests.txt
echo. > %test_log%

echo OURDIR=%outdir% tests=%tests%

set start=%TIME%

for %%x in (%tests%) do (
  echo Running %%x...
  set test_exe=%OUTDIR%\evita_%%x_tests.exe
  if not exist !test_exe! goto no_test_exe

  !test_exe! ^
      --single-process-tests ^
      --test-launcher-retry-limit=1 >> %test_log% 2>&1
)

grep -E -e "CRASHED|FAIL" %test_log%

set end=%TIME%

echo.
echo Start %start%
echo End %end%

endlocal
exit/b

:no_outdir
echo.
echo No OUTDIR
echo.
exit/b 1

:no_test_exe
echo.
echo No test exe %test_exe%
echo.
echo Available test exes are:
dir/b %OUTDIR%\*.exe
exit/b 1
