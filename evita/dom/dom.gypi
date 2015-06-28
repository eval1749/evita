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
      'text/document_file_load.js',
      'text/document_file_save.js',
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

      'lexers/lexer.js',
      #'lexers/cmd_lexer.js',
      'lexers/clike_lexer.js',
          'lexers/cpp_lexer.js',
          'lexers/csharp_lexer.js',
          'lexers/idl_lexer.js',
          'lexers/java_lexer.js',
          'lexers/javascript_lexer.js',
      'lexers/config_lexer.js',
      'lexers/python_lexer.js',
      'lexers/xml_lexer.js',
          'lexers/html_lexer.js',

      'text/modes/mode.js',
      'text/modes/modes.js',
      'os/file.js',
      'os/os.js',
      'os/process.js',
      'text/range.js',
      'text/regexp.js',
      'css/style.js',
      'windows/table_window.js',
      'text/text_position.js',
      'windows/document_state.js',
      'windows/point.js',
      'windows/rect.js',
      'windows/tab_data.js',
      'windows/text_selection.js',
      'windows/text_window.js',
      'windows/window.js',


      'jslib/editors.js',
      'jslib/ordered_set.js',
      'jslib/stringify.js',
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

      'jslib/install_stringify.js',
    ], # js_lib_files

    'js_lib_files_file': '<|(js_lib_files.txt <@(js_defs_files) <@(js_lib_files))',
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

        'events/composition_event.cc',
        'events/composition_event.h',
        'events/composition_span.cc',
        'events/composition_span.h',
        'events/document_event.cc',
        'events/event.cc',
        'events/event_target.cc',
        'events/focus_event.cc',
        'events/form_event.cc',
        'events/keyboard_event.cc',
        'events/mouse_event.cc',
        'events/wheel_event.cc',
        'events/ui_event.cc',
        'events/view_event_handler_impl.cc',
        'events/view_event_target.cc',
        'events/view_event_target_set.cc',
        'events/window_event.cc',

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

        'text/mutation_observer.cc',
        'text/mutation_observer_controller.cc',
        'text/mutation_record.cc',

        'os/abstract_file.cc',
        'os/file.cc',
        'os/process.cc',

        'windows/point.cc',
        'windows/rect.cc',
        'promise_resolver.cc',
        'text/range.cc',
        'text/regexp.cc',
        'windows/selection.cc',
        'script_host.cc',
        'script_thread.cc',
        'css/style.cc',
        'windows/table_window.cc',
        'windows/table_selection.cc',
        'windows/text_selection.cc',
        'windows/text_window.cc',
        'timers/one_shot_timer.cc',
        'timers/one_shot_timer.h',
        'timers/repeating_timer.cc',
        'timers/repeating_timer.h',
        'timers/timer.cc',
        'timers/timer.h',
        'time_stamp.cc',
        'windows/window.cc',
        'windows/window_ostream.cc',
        'windows/window_set.cc',

        'public/io_context_id.cc',
        'public/io_delegate.cc',
        'public/io_error.cc',
        'public/float_point.cc',
        'public/float_rect.cc',
        'public/switch_value.cc',
        'public/switch_value.h',
        'public/tab_data.cc',
        'public/tab_data.h',
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
        'dom_aggregate_js_extern_files',
      ],
      'actions': [
        {
          'action_name': 'closure',
          'inputs': [
            '<(DEPTH)/evita/dom/closure_compiler_workaround.js',
            '<(js_externs_dir)/evita_js_externs.js',
            '<@(js_defs_files)',
            '<@(js_lib_files)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/evita/checked.js', ],
          'action': [
            'python',
            '<(DEPTH)/tools/razzle/closure_compiler.py',
            '--js_output_file=<@(_outputs)',
            '--flagfile=<(js_lib_files_file)',
            '--extern',
            '<(js_externs_dir)/evita_js_externs.js',
          ], # action
        }, # js2c
      ], # actions
    }, # dom_jslib_js2c
    {
      'target_name': 'dom_jslib',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'dom_jslib_js2c',
      ],
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/evita/dom_jslib.cc',
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
            '<(DEPTH)/evita/dom/closure_compiler_workaround.js',
            '<@(js_defs_files)',
            '<@(js_lib_files)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/evita/dom_jslib.cc' ],
          'action': [
            'python',
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<@(_outputs)',
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<(DEPTH)/evita/dom/closure_compiler_workaround.js',
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
        '<(SHARED_INTERMEDIATE_DIR)/evita/dom_unicode_lib.cc',
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
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/evita/dom_unicode_lib.cc', ],
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

