@if "%_echo%"=="" echo off
setlocal
setlocal enabledelayedexpansion
call ..\setenv.cmd
if "%outdir%"=="" goto no_outdir

set config=%1
if "%config%"=="debug" (shift && goto end_config)
if "%config%"=="release" (shift && goto end_config)
if "%config%"=="official" (shift && goto end_config)
set config=debug
:end_config

set targets=
for %%x in (%1 %2 %3 %4 %5 %6 %7 %8 %9) do (
  set targets=!targets! %%x
)
if "%targets%"=="" set targets=evita

: set/a num_jobs=NUMBER_OF_PROCESSORS / 2
set/a num_jobs=NUMBER_OF_PROCESSORS

echo %config% %targets% num_jobs=%num_jobs%
set start=%TIME%
ninja -j %num_jobs% -C ..\out\%config% %targets%
set end=%TIME%

echo.
echo Start %start%
echo End   %end%
endlocal
exit/b

:no_outdir
echo.
echo You should set OUTDIR environment variable, e.g. OUTDIR=..\out
echo.
exit/b 1
