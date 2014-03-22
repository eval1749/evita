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
  %externs_dir%\commander_externs.js ^
  %externs_dir%\commands\spell_checker_externs.js ^
  %externs_dir%\console_externs.js ^
  %externs_dir%\css\style_externs.js ^
  %externs_dir%\encoding\text_decoder_externs.js ^
  %externs_dir%\encoding\text_encoder_externs.js ^
  %externs_dir%\errors_externs.js ^
  %externs_dir%\events\document_event_externs.js ^
  %externs_dir%\events\event_externs.js ^
  %externs_dir%\events\event_target_externs.js ^
  %externs_dir%\events\focus_event_externs.js ^
  %externs_dir%\events\form_event_externs.js ^
  %externs_dir%\events\keyboard_event_externs.js ^
  %externs_dir%\events\mouse_event_externs.js ^
  %externs_dir%\events\ui_event_externs.js ^
  %externs_dir%\events\wheel_event_externs.js ^
  %externs_dir%\forms\button_control_externs.js ^
  %externs_dir%\forms\checkbox_control_externs.js ^
  %externs_dir%\forms\form_externs.js ^
  %externs_dir%\forms\form_control_externs.js ^
  %externs_dir%\forms\form_window_externs.js ^
  %externs_dir%\forms\radio_button_control_externs.js ^
  %externs_dir%\forms\text_field_control_externs.js ^
  %externs_dir%\editor_externs.js ^
  %externs_dir%\file_path_externs.js ^
  %externs_dir%\js_console_externs.js ^
  %externs_dir%\key_names_externs.js ^
  %externs_dir%\os\file_externs.js ^
  %externs_dir%\os\process_externs.js ^
  %externs_dir%\timer_externs.js ^
  %externs_dir%\text\document_externs.js ^
  %externs_dir%\text\modes\modes_externs.js ^
  %externs_dir%\text\mutation_observer_externs.js ^
  %externs_dir%\text\range_externs.js ^
  %externs_dir%\text\regexp_externs.js ^
  %externs_dir%\text\text_position_externs.js ^
  %externs_dir%\windows\document_state_externs.js ^
  %externs_dir%\windows\document_window_externs.js ^
  %externs_dir%\windows\editor_window_externs.js ^
  %externs_dir%\windows\point_externs.js ^
  %externs_dir%\windows\selection_externs.js ^
  %externs_dir%\windows\table_selection_externs.js ^
  %externs_dir%\windows\table_window_externs.js ^
  %externs_dir%\windows\tab_data_externs.js ^
  %externs_dir%\windows\text_selection_externs.js ^
  %externs_dir%\windows\text_window_externs.js ^
  %externs_dir%\windows\window_externs.js ^
  %externs_dir%\jslib\editors_externs.js ^
  %externs_dir%\jslib\windows_externs.js

endlocal
