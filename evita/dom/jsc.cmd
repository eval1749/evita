@if "%_echo%"== "" echo off
setlocal

set externs_dir=evita\dom

python tools\razzle\closure_compiler.py ^
  --js_output_file=nul ^
  %* ^
  --extern ^
  %externs_dir%\enums.js ^
  %externs_dir%\events\event_enums.js ^
  %externs_dir%\strings_en_US.js ^
  %externs_dir%\unicode_enums.js ^
  %externs_dir%\types_externs.js ^
  %externs_dir%\bracket_externs.js ^
  %externs_dir%\console_externs.js ^
  %externs_dir%\document_externs.js ^
  %externs_dir%\document_window_externs.js ^
  %externs_dir%\events\event_externs.js ^
  %externs_dir%\events\event_target_externs.js ^
  %externs_dir%\events\focus_event_externs.js ^
  %externs_dir%\events\form_event_externs.js ^
  %externs_dir%\events\keyboard_event_externs.js ^
  %externs_dir%\events\mouse_event_externs.js ^
  %externs_dir%\events\ui_event_externs.js ^
  %externs_dir%\forms\form_externs.js ^
  %externs_dir%\forms\form_control_externs.js ^
  %externs_dir%\forms\text_field_control_externs.js ^
  %externs_dir%\editor_externs.js ^
  %externs_dir%\editor_window_externs.js ^
  %externs_dir%\file_path_externs.js ^
  %externs_dir%\js_console_externs.js ^
  %externs_dir%\key_names_externs.js ^
  %externs_dir%\range_externs.js ^
  %externs_dir%\selection_externs.js ^
  %externs_dir%\style_externs.js ^
  %externs_dir%\table_selection_externs.js ^
  %externs_dir%\table_window_externs.js ^
  %externs_dir%\text_position_externs.js ^
  %externs_dir%\text_selection_externs.js ^
  %externs_dir%\text_window_externs.js ^
  %externs_dir%\timer_externs.js ^
  %externs_dir%\window_externs.js ^
  %externs_dir%\jslib\editors_externs.js ^
  %externs_dir%\jslib\windows_externs.js

endlocal
