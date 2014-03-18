@if "%_echo%"=="" echo off
setlocal

set my_dir=%~dp0
set root=%my_dir%\..\..\..
set int_dir=%root%/out/Debug/gen
set out_dir=%temp%
set parser_dir=%root%src\third_party\blink_idl_parser

set attrs_file=%my_dir%/IDLExtendedAttributes.txt
set iface_file=%int_dir%/InterfacesInfo.pickle

python -s %my_dir%/idl_to_js_externs.py --output-dir %out_dir% ^
    --idl-attributes-file %attrs_file% ^
    --interfaces-info %iface_file% ^
    %1

endlocal

