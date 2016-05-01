@if "%_echo%"=="" echo off
call ..\setenv.cmd
if "%outdir%"=="" goto no_outdir

start devenv.exe /debugexe %outdir%\Debug\evita.exe --multiple --v=0
exit/b

:no_outdir
echo.
echo You should set OUTDIR environment variable, e.g. OUTDIR=..\out
echo.
exit/b 1
