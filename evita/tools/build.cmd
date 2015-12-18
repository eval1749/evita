@if "%_echo%"=="" echo off
setlocal
setlocal enabledelayedexpansion
set config=debug
:// if "%cd:~-11,-4%"=="release" set config=release_x64
for %%x in (%*) do (
  set targets=!targets! %%x
)
if "%targets%"=="" set targets=evita
: set/a num_jobs=NUMBER_OF_PROCESSORS / 2
set/a num_jobs=NUMBER_OF_PROCESSORS
set start=%TIME%
ninja -j %num_jobs% -C ..\out.gn\%config% %targets%
set end=%TIME%
echo Start %start%
echo End   %end%
endlocal
