# Copyright (c) 2013-2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'interface_idl_files': [
      'clipboard/DataTransfer.idl',
      'clipboard/DataTransferItem.idl',
      'clipboard/DataTransferItemList.idl',
      'Editor.idl',
      'events/Event.idl',
      'events/EventTarget.idl',
      'events/ViewEventTarget.idl',
      'forms/ButtonControl.idl',
      'forms/CheckboxControl.idl',
      'forms/FormControl.idl',
      'forms/FormWindow.idl',
      'forms/LabelControl.idl',
      'forms/RadioButtonControl.idl',
      'forms/TextFieldControl.idl',
      'forms/TextFieldSelection.idl',
      'os/AbstractFile.idl',
      'os/File.idl',
      'text/modes/Mode.idl',
      'windows/DocumentWindow.idl',
      'windows/EditorWindow.idl',
      'windows/Point.idl',
      'windows/Rect.idl',
      'windows/Selection.idl',
      'windows/TableSelection.idl',
      'windows/Window.idl',
    ], # idl_files

    'dictionary_idl_files': [
      'events/EventInit.idl',
      'forms/FormWindowInit.idl',
      'os/MoveFileOptions.idl',
    ], # dictionary_idl_files

    'generated_glue_files': [
      '<(glue_dir)/AbstractFile.cc',
      '<(glue_dir)/AbstractFile.h',
      '<(glue_dir)/ButtonControl.cc',
      '<(glue_dir)/ButtonControl.h',
      '<(glue_dir)/CheckboxControl.cc',
      '<(glue_dir)/CheckboxControl.h',
      '<(glue_dir)/DataTransfer.cc',
      '<(glue_dir)/DataTransfer.h',
      '<(glue_dir)/DataTransferItem.cc',
      '<(glue_dir)/DataTransferItem.h',
      '<(glue_dir)/DataTransferItemList.cc',
      '<(glue_dir)/DataTransferItemList.h',
      '<(glue_dir)/DocumentWindow.cc',
      '<(glue_dir)/DocumentWindow.h',
      '<(glue_dir)/Editor.cc',
      '<(glue_dir)/Editor.h',
      '<(glue_dir)/EditorWindow.cc',
      '<(glue_dir)/EditorWindow.h',
      '<(glue_dir)/Event.cc',
      '<(glue_dir)/Event.h',
      '<(glue_dir)/EventInit.cc',
      '<(glue_dir)/EventInit.h',
      '<(glue_dir)/FormControl.cc',
      '<(glue_dir)/FormControl.h',
      '<(glue_dir)/FormWindow.cc',
      '<(glue_dir)/FormWindow.h',
      '<(glue_dir)/FormWindowInit.cc',
      '<(glue_dir)/FormWindowInit.h',
      '<(glue_dir)/EventTarget.cc',
      '<(glue_dir)/EventTarget.h',
      '<(glue_dir)/File.cc',
      '<(glue_dir)/File.h',
      '<(glue_dir)/LabelControl.cc',
      '<(glue_dir)/LabelControl.h',
      '<(glue_dir)/Mode.cc',
      '<(glue_dir)/Mode.h',
      '<(glue_dir)/MoveFileOptions.cc',
      '<(glue_dir)/MoveFileOptions.h',
      '<(glue_dir)/RadioButtonControl.cc',
      '<(glue_dir)/RadioButtonControl.h',
      '<(glue_dir)/Selection.cc',
      '<(glue_dir)/Selection.h',
      '<(glue_dir)/TableSelection.cc',
      '<(glue_dir)/TableSelection.h',
      '<(glue_dir)/TextFieldControl.cc',
      '<(glue_dir)/TextFieldControl.h',
      '<(glue_dir)/TextFieldSelection.cc',
      '<(glue_dir)/TextFieldSelection.h',
      '<(glue_dir)/ViewEventTarget.cc',
      '<(glue_dir)/ViewEventTarget.h',
      '<(glue_dir)/Window.cc',
      '<(glue_dir)/Window.h',
    ],

    'generated_js_externs_files': [
      '<(js_externs_dir)/AbstractFile_externs.js',
      '<(js_externs_dir)/ButtonControl_externs.js',
      '<(js_externs_dir)/CheckboxControl_externs.js',
      '<(js_externs_dir)/DataTransfer_externs.js',
      '<(js_externs_dir)/DataTransferItem_externs.js',
      '<(js_externs_dir)/DataTransferItemList_externs.js',
      '<(js_externs_dir)/DocumentWindow_externs.js',
      '<(js_externs_dir)/Editor_externs.js',
      '<(js_externs_dir)/EditorWindow_externs.js',
      '<(js_externs_dir)/File_externs.js',
      '<(js_externs_dir)/FormControl_externs.js',
      '<(js_externs_dir)/FormWindow_externs.js',
      '<(js_externs_dir)/FormWindowInit_externs.js',
      '<(js_externs_dir)/LabelControl_externs.js',
      '<(js_externs_dir)/Mode_externs.js',
      '<(js_externs_dir)/Point_externs.js',
      '<(js_externs_dir)/Rect_externs.js',
      '<(js_externs_dir)/RadioButtonControl_externs.js',
      '<(js_externs_dir)/Selection_externs.js',
      '<(js_externs_dir)/TableSelection_externs.js',
      '<(js_externs_dir)/TextFieldControl_externs.js',
      '<(js_externs_dir)/TextFieldSelection_externs.js',
      '<(js_externs_dir)/ViewEventTarget_externs.js',
      '<(js_externs_dir)/Window_externs.js',
    ], # generated_js_externs_files

    'static_js_externs_files': [
      'errors_externs.js',
      'types_externs.js',
      'bracket_externs.js',
      'clipboard/clipboard_externs.js',
      'commander_externs.js',
      'commands/find_and_replace_externs.js',
      'commands/spell_checker_externs.js',
      'console_externs.js',
      'text/document_externs.js',
      'events/document_event_externs.js',
      'events/focus_event_externs.js',
      'events/form_event_externs.js',
      'events/keyboard_event_externs.js',
      'events/mouse_event_externs.js',
      'events/wheel_event_externs.js',
      'events/ui_event_externs.js',
      'events/window_event_externs.js',

      'encodings/text_decoder_externs.js',
      'encodings/text_encoder_externs.js',

      'forms/form_externs.js',

      'file_path_externs.js',
      'js_console_externs.js',
      'key_names_externs.js',
      'text/modes/modes_externs.js',
      'text/mutation_observer_externs.js',
      'os/os_externs.js',
      'os/process_externs.js',
      'text/range_externs.js',
      'text/regexp_externs.js',
      'windows/document_state_externs.js',
      'css/style_externs.js',
      'windows/table_window_externs.js',
      'windows/tab_data_externs.js',
      'text/text_position_externs.js',
      'windows/text_selection_externs.js',
      'windows/text_window_externs.js',
      'timer_externs.js',
      'jslib/editors_externs.js',
      'jslib/windows_externs.js',
    ], # static_js_externs_files

    # Computed variables

    'glue_dir': '<(SHARED_INTERMEDIATE_DIR)/evita/bindings',
    'interface_idl_file_list': '<|(interface_idl_file.txt <@(interface_idl_files))',
    'idl_files': '<@(interface_idl_files) <@(dictionary_idl_files)',
    'idl_file_list': '<|(idl_file.txt <@(idl_files))',
    'idl_work_dir': '<(SHARED_INTERMEDIATE_DIR)/evita',
    'js_externs_dir': '<(SHARED_INTERMEDIATE_DIR)/evita/js_externs',
    'static_file_list': '<|(static_js_externs.txt <@(static_js_externs_files))',

    'conditions': [
        ['"<(GENERATOR)"=="ninja"', {
          'write_file_only_if_changed': 1,
        },{
          'write_file_only_if_changed': 1,
        }],
      ], # conditions
  }, # variables

  'targets': [
    {
      'target_name': 'dom_aggregate_js_extern_files',
      'type': 'none',
      'dependencies': [
        'dom_generated_js_extern_files',
      ],
      'actions': [{
        'action_name': 'dom_aggregate_js_extern_files',
        'inputs': [
          '<(DEPTH)/tools/idl_to_js_externs/aggregate_js_externs.py',
          '<(idl_file_list)',
          '<@(generated_js_externs_files)',
          '<@(static_js_externs_files)',
        ], # inputs
        'outputs': [
          '<(js_externs_dir)/evita_js_externs.js',
        ],
        'action': [
          'python',
          '<(DEPTH)/tools/idl_to_js_externs/aggregate_js_externs.py',
          '<(js_externs_dir)/evita_js_externs.js',
          '<(DEPTH)/tools/idl_to_js_externs/header.js',

          '<(js_externs_dir)', '<(idl_file_list)',
          '<(DEPTH)/evita', '<(static_file_list)',
        ], # action
      }], # actions
    },

    # DOM bindings library
    {
      'target_name': 'dom_aggregate_glue_files',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/gin/gin.gyp:gin',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom_generated_glue_files',
        'v8_glue',
      ], # dependencies
      'include_dirs+': [ '<(SHARED_INTERMEDIATE_DIR)/evita/bindings' ],
      'sources': [
        '../precomp.cpp',
        '<@(generated_glue_files)',
      ], # sources
    },

    # Generate glue C++ source code and header file from IDL file.
    {
      'target_name': 'dom_generated_glue_files',
      'type': 'none',
      'dependencies': [
        'dom_interfaces_info',
      ],
      'sources': [ '<@(idl_files)' ],
      'rules': [{
        'rule_name': 'dom_idl_to_glue_file',
        'extension': 'idl',
        'msvs_external_rule': 1,
        'inputs': [
          'bindings/idl_to_glue.py',
          'bindings/dictionary.cc',
          'bindings/dictionary.h',
          'bindings/interface.cc',
          'bindings/interface.h',
          '<(idl_work_dir)/InterfacesInfo.pickle',
        ], # inputs
        'outputs': [
          '<(glue_dir)/<(RULE_INPUT_ROOT).cc',
          '<(glue_dir)/<(RULE_INPUT_ROOT).h',
        ],
        'action': [
          'python',
          '<(DEPTH)/evita/dom/bindings/idl_to_glue.py',
          '--output-dir', '<(glue_dir)',
          '--interfaces-info', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<@(write_file_only_if_changed)',
          '<(RULE_INPUT_PATH)',
        ], # action
      }], # rules
    }, # 'dom_generated_glue_files'

    # Generate JavaScript Externs from IDL files.
    {
      'target_name': 'dom_generated_js_extern_files',
      'type': 'none',
      'dependencies': [
        'dom_interfaces_info',
      ],
      'sources': [ '<@(idl_files)' ],
      'rules': [{
        'rule_name': 'dom_idl_to_js_externs',
        'extension': 'idl',
        'msvs_external_rule': 1,
        'inputs': [
          '<(DEPTH)/tools/idl_to_js_externs/idl_to_js_externs.py',
          '<(idl_work_dir)/InterfacesInfo.pickle',
        ], # inputs
        'outputs': [
          '<(js_externs_dir)/<(RULE_INPUT_ROOT)_externs.js',

        ],
        'action': [
          'python',
          '<(DEPTH)/tools/idl_to_js_externs/idl_to_js_externs.py',
          '--output-dir', '<(js_externs_dir)',
          '--interfaces-info', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<@(write_file_only_if_changed)',
          '<(RULE_INPUT_PATH)',
        ], # action
      }], # rules
    }, # 'dom_generated_js_extern_files'

    # Interfaces information for IDL compiler
    {
      'target_name': 'dom_interfaces_info',
      'type': 'none',
      'actions': [{
        'action_name': 'compute_dom_interfaces_info',
        'inputs': [
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
          '<@(interface_idl_files)',
        ],
        'outputs': [
          '<(idl_work_dir)/InterfacesInfo.pickle',

        ],
        'action': [
          'python',
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
          '--idl-files-list', '<(interface_idl_file_list)',
          '--interfaces-info-file', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<(write_file_only_if_changed)',
        ], # action
        'message': 'Computing global information about IDL files',
      }], # actions
    }, # dom_interfaces_info
  ], # targets
}
