@if "%_echo%"=="" echo off
call ../setenv.cmd
if "%outdir%"=="" goto no_outdir

setlocal
set options=--single-process-tests --test-launcher-retry-limit=0

set config=%1
if "%config%"=="" goto usage
if "%config%"=="debug" (shift && goto has_config)
if "%config%"=="release" (shift && goto has_config)
if "%config%"=="offical" (shift && goto has_config)
set config=debug
:has_config
set which=%1
set pattern=%2
if "%which%"=="" goto usage

if "%which%"=="d" set which=dom
if "%which%"=="l" set which=layout
if "%which%"=="t" set which=text
if "%which%"=="v" set which=visuals

set exedir=%outdir%\%config%
set filter=
set target=%exedir%\evita_%which%_tests.exe

if not exist %target% goto not_found
if not "%pattern%"=="" set filter=--gtest_filter=%pattern%

:// We should use single process for running test, because initialization
:// of V8 takes 2 to 3 seconds for each process.
%target% %options% %filter%
exit/b

:not_found
echo No such tests: %which%
echo Avaliable tests are:
for /f "usebackq delims=_ tokens=2" %%x in (`dir/b %exedir%\evita_*_tests.exe`) do (
  echo %space% %space% %%x
)
exit/b

:no_outdir
echo.
echo No OUTDIR
echo.
exit/b 1

:usage
echo Usage: %~nx0 [config] which [pattern]
exit/b
