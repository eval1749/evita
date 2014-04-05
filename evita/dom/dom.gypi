# Copyright (c) 2013-2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    'dom_idl.gypi',
  ],

  'variables': {
     'js_externs_dir': '<(SHARED_INTERMEDIATE_DIR)/evita',

    'js_defs_files': [
      'enums.js',
      'events/event_enums.js',
      'strings_en_US.js',
      'unicode_enums.js',
      'windows/window_enums.js',
     ],
    'js_lib_files': [
      'polyfill.js',
      'key_names.js',

      'bracket.js',
      'commander.js',
      'text/document.js',
      'text/document_file_io.js',
      'editor.js',
      'windows/editor_window.js',
      'encodings/text_decoder.js',
      'encodings/text_encoder.js',
      'errors.js',
      'file_path.js',

      'forms/button_control.js',
      'forms/checkbox_control.js',
      'forms/form.js',
      'forms/form_control.js',
      'forms/form_window.js',
      'forms/label_control.js',
      'forms/radio_button_control.js',
      'forms/text_field_control.js',
      'forms/text_field_selection.js',

      'text/modes/modes.js',
      'os/file.js',
      'os/process.js',
      'text/range.js',
      'text/regexp.js',
      'css/style.js',
      'windows/table_window.js',
      'text/text_position.js',
      'windows/document_state.js',
      'windows/tab_data.js',
      'windows/text_selection.js',
      'windows/text_window.js',
      'windows/window.js',

      'jslib/editors.js',
      'jslib/windows.js',

      'console.js',
      'commands/document_list.js',
      'commands/dynamic_abbrev.js',
      'commands/js_console.js',
      'commands/find_and_replace.js',
      'commands/find_and_replace_commands.js',
      'commands/spell_checker.js',
      'commands/text_window_commands.js',
      'commands/window_commands.js',
    ], # js_lib_files
   }, # variables

  'targets': [
    {
      'target_name': 'dom',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom_aggregate_glue_files',
        'dom_jslib',
        'dom_unicode_lib_icu',
        'dom_v8_strings_cc',
        'text',
        'v8_glue',
      ],
      'include_dirs+': [ '<(SHARED_INTERMEDIATE_DIR)/' ],
      'sources': [
        '../precomp.cpp',

        'text/buffer.cc',
        'converter.cc',
        'text/document.cc',
        'text/document_set.cc',
        'windows/document_window.cc',
        'editor.cc',
        'windows/editor_window.cc',

        'clipboard/clipboard.cc',
        'clipboard/data_transfer.cc',
        'clipboard/data_transfer_data.cc',
        'clipboard/data_transfer_item.cc',
        'clipboard/data_transfer_item_list.cc',

        'encodings/text_decoder.cc',
        'encodings/text_encoder.cc',

        'events/document_event.cc',
        'events/document_event_init.cc',
        'events/event.cc',
        'events/event_init.cc',
        'events/event_target.cc',
        'events/focus_event.cc',
        'events/focus_event_init.cc',
        'events/form_event.cc',
        'events/form_event_init.cc',
        'events/keyboard_event.cc',
        'events/keyboard_event_init.cc',
        'events/mouse_event.cc',
        'events/mouse_event_init.cc',
        'events/wheel_event.cc',
        'events/wheel_event_init.cc',
        'events/ui_event.cc',
        'events/ui_event_init.cc',
        'events/view_event_handler_impl.cc',
        'events/view_event_target.cc',
        'events/view_event_target_set.cc',
        'events/window_event.cc',
        'events/window_event_init.cc',

        'forms/button_control.cc',
        'forms/checkbox_control.cc',
        'forms/form.cc',
        'forms/form_control.cc',
        'forms/form_observer.cc',
        'forms/form_window.cc',
        'forms/label_control.cc',
        'forms/radio_button_control.cc',
        'forms/text_field_control.cc',
        'forms/text_field_selection.cc',

        'file_path.cc',
        'global.cc',
        'dictionary.cc',
        'lock.cc',

        'text/modes/mode.cc',
        'text/modes/config_mode.cc',
        'text/modes/cxx_mode.cc',
        'text/modes/haskell_mode.cc',
        'text/modes/java_mode.cc',
        'text/modes/javascript_mode.cc',
        'text/modes/lisp_mode.cc',
        'text/modes/mason_mode.cc',
        'text/modes/perl_mode.cc',
        'text/modes/plain_text_mode.cc',
        'text/modes/python_mode.cc',
        'text/modes/xml_mode.cc',

        'text/mutation_observer.cc',
        'text/mutation_observer_controller.cc',
        'text/mutation_observer_init.cc',
        'text/mutation_record.cc',

        'os/abstract_file.cc',
        'os/file.cc',
        'os/process.cc',

        'windows/point.cc',
        'promise_resolver.cc',
        'text/range.cc',
        'text/regexp.cc',
        'text/regexp_init.cc',
        'windows/selection.cc',
        'script_host.cc',
        'script_thread.cc',
        'css/style.cc',
        'windows/table_window.cc',
        'windows/table_selection.cc',
        'windows/text_selection.cc',
        'windows/text_window.cc',
        'timer.cc',
        'time_stamp.cc',
        'windows/window.cc',
        'windows/window_ostream.cc',
        'windows/window_set.cc',

        'public/io_context_id.cc',
        'public/io_delegate.cc',
        'public/float_point.cc',
        'public/view_event_handler.cc',

        '<(SHARED_INTERMEDIATE_DIR)/v8_strings.cc',

        '../gc/collectable.cc',
        '../gc/collector.cc',
        '../gc/visitable.cc',
      ], # sources
    }, # dom
    {
      'target_name': 'check_jslib',
      'type': 'none',
      'dependencies': [
        'dom_generated_js_extern_files',
      ], # dependencies
      'rules': [
        {
          'rule_name': 'jscomp',
          'extension': 'js',
          'inputs': [
             '<(DEPTH)/tools/razzle/closure_compiler.py',
             '<@(js_defs_files)',
             '<(js_externs_dir)/evita_js_externs.js',
           ], # inputs
          'outputs': [
              '<(SHARED_INTERMEDIATE_DIR)/<(RULE_INPUT_ROOT).min.js',
           ], # outputs
          'action': [
            'python',
            '<(DEPTH)/tools/razzle/closure_compiler.py',
            '--js_output_file=<@(_outputs)',
            '<(RULE_INPUT_PATH)',
            '--extern',
            '<@(js_defs_files)',
            '<(js_externs_dir)/evita_js_externs.js',
          ], # action
        } # closure_compiler
      ], # 'rules'
      'sources': [ '<@(js_lib_files)', ],
    },
    {
      'target_name': 'dom_jslib',
      'type': 'static_library',
      'dependencies': [ 'dom_jslib_js2c', ], 
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/dom_jslib.cc',
      ], # sources
    },
    {
      'target_name': 'dom_jslib_js2c',
      'type': 'none',
      'actions': [
        {
          'action_name': 'js2c',
          'inputs': [
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<@(js_defs_files)',
            '<@(js_lib_files)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/dom_jslib.cc' ],
          'action': [
            'python',
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<@(_outputs)',
            '<@(js_defs_files)',
            '<@(js_lib_files)',
          ], # action
        }, # js2c
      ], # actions
    }, # dom_jslib_js2c
    {
      'target_name': 'dom_unicode_lib_icu',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(icu_gyp_path):icuuc',
      ], 
      'include_dirs+': [ '<(DEPTH)/third_party/icu/source/common' ],
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'sources': [
        'unicode_icu.cc',
      ], # sources
    },
    {
      'target_name': 'dom_unicode_lib',
      'type': 'static_library',
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'dependencies': [
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom_unicode_lib_cc'
      ],
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/dom_unicode_lib.cc',
      ], # sources
    }, # dom_unicode_lib
    {
      # Because of Chromium icudata doesn't contain unames.icu. So, we create
      # our own UCD data table.
      # See third_party/icu/patches/data.build.patch
      'target_name': 'dom_unicode_lib_cc',
      'type': 'none',
      'variables': {
        'unicode_data_txt': '<(DEPTH)/third_party/unicode/UnicodeData.txt',
      }, # variables
      'actions': [
        {
          'action_name': 'make_dom_unicode_lib_cc',
          'inputs': [
            'make_unicode_lib_cc.py',
            '<(unicode_data_txt)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/dom_unicode_lib.cc', ],
          'action': [
            'python',
            '<(DEPTH)/evita/dom/make_unicode_lib_cc.py',
            '<@(_outputs)',
            '<(unicode_data_txt)',
          ], # action
        },
      ], # actions
    },
    {
      'target_name': 'dom_v8_strings_cc',
      'type': 'none',
      'actions': [
        {
          'action_name': 'make_v8_strings_cc',
          'inputs': [
            'make_v8_strings.py',
            'v8_strings.in',
          ], # inputs
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings.cc',
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings.h',
          ],
          'action': [
            'python',
            '<(DEPTH)/evita/dom/make_v8_strings.py',
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings',
            '<(DEPTH)/evita/dom/v8_strings.in',
          ], # action
        },
      ], # actions
    },
  ], # targets
}
