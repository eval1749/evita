@if "%_echo%"=="" echo off
setlocal
set out_dir=..\out\Debug\gen\evita
python evita\dom\bindings\idl_to_glue.py ^
    --output-dir=%out_dir%\bindings ^
    --interfaces-info %out_dir%\InterfacesInfo.pickle ^
    %1
endlocal
