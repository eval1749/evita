@if "%_echo%"=="" echo off
start devenv.exe /debugexe ..\out.gn\Debug\evita.exe --multiple --v=0
