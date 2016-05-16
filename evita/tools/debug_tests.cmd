@if "%_echo%"=="" echo off
setlocal
call ..\setenv.cmd
if "%outdir%"=="" goto no_outdir

set options=--single-process-tests --gtest_break_on_failure --gtest_throw_on_failure

set config=%1
if "%config%"=="" goto usage
if "%config%"=="debug" (shift && goto has_config)
if "%config%"=="release" (shift && goto has_config)
if "%config%"=="official" (shift && goto has_config)
set config=debug
:has_config
set which=%1
set pattern=%2
if "%which%"=="" goto usage

if "%which%"=="d" set which=dom
if "%which%"=="g" set which=geometry
if "%which%"=="l" set which=layout
if "%which%"=="t" set which=text
if "%which%"=="v" set which=visuals

set exedir=%outdir%\%config%
set target=%exedir%\evita_%which%_tests.exe
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
for /f "usebackq delims=_ tokens=2" %%x in (`dir/b %exedir%\evita_*_tests.exe`) do (
  echo %space% %space% %%x
)
exit/b


:no_outdir
echo.
echo You should set OUTDIR environment variable, e.g. OUTDIR=..\out
echo.
exit/b 1
