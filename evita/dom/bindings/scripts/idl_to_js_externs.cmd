@if "%_echo%"=="" echo off
setlocal

set my_dir=%~dp0
set int_dir=../out/Debug/gen/evita
set out_dir=%int_dir%\js_externs
set parser_dir=%root%src\third_party\blink_idl_parser

set iface_file=%int_dir%/InterfacesInfo.pickle

python -s %my_dir%/idl_to_js_externs.py --output-dir %out_dir% ^
    --interfaces-info %iface_file% ^
    %1

endlocal

