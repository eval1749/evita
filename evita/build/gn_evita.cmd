@if "%_echo%"=="" echo off
setlocal
set outdir=..\out.gn
echo.
echo.
echo //////////////////////////////////////////////////////////////////////
echo //
echo // Current directory is %CD%
echo // Generating *.ninja into %outdir%
echo //

: v8 should have  -DICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_SHARE

if exist src cd src
call gn gen %outdir%\Debug --args="is_component_build=true is_debug=true"
call gn gen %outdir%\Release --args="is_component_build=false is_debug=false"
endlocal
