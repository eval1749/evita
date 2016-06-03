@if "%_echo%"=="" echo off
setlocal
echo.
echo.
echo //////////////////////////////////////////////////////////////////////
echo //
echo // Current directory is %CD%
echo // Generating *.ninja into %outdir%
echo //
echo // DEPOT_TOOLS_WIN_TOOLCHAIN=%DEPOT_TOOLS_WIN_TOOLCHAIN%
echo // GYP_MSVS_VERSION=%GYP_MSVS_VERSION%
echo //

if exist setenv.cmd (
  call setenv.cmd
  goto called_setenv
)
if exist ..\setenv.cmd  (
  call ..\setenv.cmd
  goto called_setenv
)
echo No setenv.cmd to set OUTDIR
exit/b 1

:called_setenv
if "%outdir%"=="" goto no_outdir

: v8 should have  -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_SHARE

if exist src cd src
set common_args= is_win_fastlink=true v8_use_external_startup_data=false
call gn gen %outdir%\Debug --args="is_component_build=true is_debug=true %common_args%"
call gn gen %outdir%\Release --args="is_component_build=false is_debug=false %common_args%"
call gn gen %outdir%\Official --args="is_component_build=false is_debug=false is_official_build=true %common_args%"
endlocal
exit /b

:no_outdir
echo.
echo You should set OUTDIR environment variable, e.g. OUTDIR=..\out
echo.
exit/b 1
