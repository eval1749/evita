@if "%_echo%"== "" echo off
setlocal

set externs_dir=evita\dom

python tools\razzle\closure_compiler.py ^
  %* ^
  --extern ^
  %externs_dir%\enums.js ^
  %externs_dir%\events\event_enums.js ^
  %externs_dir%\strings_en_US.js ^
  %externs_dir%\unicode_enums.js ^
  %externs_dir%\types_externs.js ^
  %externs_dir%\events\event_externs.js ^
  %externs_dir%\events\event_target_externs.js ^
  %externs_dir%\events\focus_event_externs.js ^
  %externs_dir%\events\ui_event_externs.js ^
  %externs_dir%\editor_externs.js ^
  %externs_dir%\editor_window_externs.js ^
  %externs_dir%\document_externs.js ^
  %externs_dir%\file_path_externs.js ^
  %externs_dir%\key_names_externs.js ^
  %externs_dir%\range_externs.js ^
  %externs_dir%\runtime_externs.js ^
  %externs_dir%\text_window_externs.js ^
  %externs_dir%\window_externs.js

endlocal
