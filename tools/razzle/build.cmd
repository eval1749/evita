@if "%_echo%"=="" echo off
setlocal
setlocal enabledelayedexpansion
set config=debug
for %%x in (%*) do (
  if "%%x"=="release" (
    set config=release_x64
  ) else (
    set targets=!targets! %%x
  )
)
if "%targets%"=="" set targets=evita
set start=%TIME%
ninja -C ..\out\%config% %targets%
set end=%TIME%
echo Start %start%
echo End   %end%
endlocal
