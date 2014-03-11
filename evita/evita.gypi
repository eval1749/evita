# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
    'variables': {
      'js_defs_files': [
        'dom/enums.js',
        'dom/events/event_enums.js',
        'dom/strings_en_US.js',
        'dom/unicode_enums.js',
        'dom/window_enums.js',
      ],
      'js_externs_files': [
        'dom/errors_externs.js',
        'dom/types_externs.js',
        'dom/bracket_externs.js',
        'dom/commander_externs.js',
        'dom/console_externs.js',
        'dom/document_externs.js',
        'dom/document_window_externs.js',
        'dom/events/document_event_externs.js',
        'dom/events/event_externs.js',
        'dom/events/event_target_externs.js',
        'dom/events/focus_event_externs.js',
        'dom/events/form_event_externs.js',
        'dom/events/keyboard_event_externs.js',
        'dom/events/mouse_event_externs.js',
        'dom/events/wheel_event_externs.js',
        'dom/events/ui_event_externs.js',
        'dom/events/window_event_externs.js',
        'dom/editor_externs.js',
        'dom/editor_window_externs.js',

        'dom/encoding/text_decoder_externs.js',
        'dom/encoding/text_encoder_externs.js',

        'dom/forms/button_control_externs.js',
        'dom/forms/checkbox_control_externs.js',
        'dom/forms/form_externs.js',
        'dom/forms/form_control_externs.js',
        'dom/forms/radio_button_control_externs.js',
        'dom/forms/text_field_control_externs.js',

        'dom/file_path_externs.js',
        'dom/js_console_externs.js',
        'dom/key_names_externs.js',
        'dom/modes/modes_externs.js',
        'dom/os/file_externs.js',
        'dom/os/process_externs.js',
        'dom/point_externs.js',
        'dom/range_externs.js',
        'dom/regexp_externs.js',
        'dom/selection_externs.js',
        'dom/style_externs.js',
        'dom/table_selection_externs.js',
        'dom/table_window_externs.js',
        'dom/text_position_externs.js',
        'dom/text_selection_externs.js',
        'dom/text_window_externs.js',
        'dom/timer_externs.js',
        'dom/window_externs.js',
        'dom/jslib/editors_externs.js',
        'dom/jslib/windows_externs.js',
      ],
      'js_lib_files': [
        'dom/polyfill.js',
        'dom/key_names.js',

        'dom/bracket.js',
        'dom/commander.js',
        'dom/document.js',
        'dom/editor.js',
        'dom/editor_window.js',
        'dom/encoding/text_decoder.js',
        'dom/encoding/text_encoder.js',
        'dom/errors.js',
        'dom/file_path.js',

        'dom/forms/button_control.js',
        'dom/forms/checkbox_control.js',
        'dom/forms/form.js',
        'dom/forms/form_control.js',
        'dom/forms/radio_button_control.js',
        'dom/forms/text_field_control.js',

        'dom/modes/modes.js',
        'dom/os/file.js',
        'dom/os/process.js',
        'dom/range.js',
        'dom/regexp.js',
        'dom/style.js',
        'dom/table_window.js',
        'dom/text_position.js',
        'dom/text_selection.js',
        'dom/text_window.js',
        'dom/window.js',

        'dom/jslib/editors.js',
        'dom/jslib/windows.js',

        'dom/console.js',
        'dom/commands/document_list.js',
        'dom/commands/dynamic_abbrev.js',
        'dom/commands/js_console.js',
        'dom/commands/find_and_replace_commands.js',
        'dom/commands/text_window_commands.js',
        'dom/commands/window_commands.js',
      ],
    }, # variables

  'targets': [
    {
      'target_name': 'evita',
      'type': 'executable',
      'dependencies': [
        'editor',
      ], # dependencies

      'msvs_settings': {
        'VCLinkerTool': {
          'OptimizeForWindows98': 1,
          'SubSystem': 2, # /SUBSYSTEM:WINDOWS
          'AdditionalDependencies': [
            'gdi32.lib',
            'comdlg32.lib',
            'advapi32.lib',
            'shell32.lib',
            'ole32.lib',
            'oleaut32.lib',
            'uuid.lib',
          ], # AdditionalDependencies
        }, # VCLinkTool
        'VCManifestTool': {
          'AdditionalManifestFiles': [ 'evita.exe.manifest' ],
          'EmbedManifest': 'true',
        }, # VCManifestTool
      }, # msvs_settings

      'sources': [
        'precomp.cpp',
        'ap_main.cpp',
        'evita.rc',
      ], # sources
    }, # evita

    {
      'target_name': 'dom',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom_jslib',
        'dom_unicode_lib_icu',
        'dom_v8_strings_cc',
        'text',
        'v8_glue',
      ],
      'include_dirs+': [ '<(SHARED_INTERMEDIATE_DIR)/' ],
      'sources': [
        'precomp.cpp',

        'dom/buffer.cc',
        'dom/converter.cc',
        'dom/document.cc',
        'dom/document_window.cc',
        'dom/editor.cc',
        'dom/editor_window.cc',
        'dom/encoding/text_decoder.cc',
        'dom/encoding/text_encoder.cc',

        'dom/events/document_event.cc',
        'dom/events/document_event_init.cc',
        'dom/events/event.cc',
        'dom/events/event_handler.cc',
        'dom/events/event_init.cc',
        'dom/events/event_target.cc',
        'dom/events/focus_event.cc',
        'dom/events/focus_event_init.cc',
        'dom/events/form_event.cc',
        'dom/events/form_event_init.cc',
        'dom/events/keyboard_event.cc',
        'dom/events/keyboard_event_init.cc',
        'dom/events/mouse_event.cc',
        'dom/events/mouse_event_init.cc',
        'dom/events/wheel_event.cc',
        'dom/events/wheel_event_init.cc',
        'dom/events/ui_event.cc',
        'dom/events/ui_event_init.cc',
        'dom/events/window_event.cc',
        'dom/events/window_event_init.cc',

        'dom/forms/button_control.cc',
        'dom/forms/checkbox_control.cc',
        'dom/forms/form.cc',
        'dom/forms/form_control.cc',
        'dom/forms/radio_button_control.cc',
        'dom/forms/text_field_control.cc',

        'dom/file_path.cc',
        'dom/global.cc',
        'dom/init_dict.cc',
        'dom/lock.cc',

        'dom/modes/mode.cc',
        'dom/modes/config_mode.cc',
        'dom/modes/cxx_mode.cc',
        'dom/modes/haskell_mode.cc',
        'dom/modes/java_mode.cc',
        'dom/modes/javascript_mode.cc',
        'dom/modes/lisp_mode.cc',
        'dom/modes/mason_mode.cc',
        'dom/modes/perl_mode.cc',
        'dom/modes/plain_text_mode.cc',
        'dom/modes/python_mode.cc',
        'dom/modes/xml_mode.cc',

        'dom/os/abstract_file.cc',
        'dom/os/file.cc',
        'dom/os/process.cc',

        'dom/point.cc',
        'dom/promise_deferred.cc',
        'dom/range.cc',
        'dom/regexp.cc',
        'dom/regexp_init.cc',
        'dom/selection.cc',
        'dom/script_controller.cc',
        'dom/script_thread.cc',
        'dom/style.cc',
        'dom/table_window.cc',
        'dom/table_selection.cc',
        'dom/text_selection.cc',
        'dom/text_window.cc',
        'dom/timer.cc',
        'dom/time_stamp.cc',
        'dom/window.cc',
        'dom/window_ostream.cc',

        'dom/public/io_context_id.cc',
        'dom/public/io_delegate.cc',
        'dom/public/float_point.cc',
        'dom/public/view_event_handler.cc',

        '<(SHARED_INTERMEDIATE_DIR)/v8_strings.cc',

        'gc/collectable.cc',
        'gc/collector.cc',
        'gc/visitable.cc',
      ], # sources
    }, # dom
    {
      'target_name': 'check_jslib',
      'type': 'none',
      'rules': [
        {
          'rule_name': 'jscomp',
          'extension': 'js',
          'inputs': [
             '<(DEPTH)/tools/razzle/closure_compiler.py',
             '<@(js_defs_files)',
             '<@(js_externs_files)',
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
            '<@(js_externs_files)',
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
        'dom/unicode_icu.cc',
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
            'dom/make_unicode_lib_cc.py',
            '<(unicode_data_txt)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/dom_unicode_lib.cc', ],
          'action': [
            'python',
            'dom/make_unicode_lib_cc.py',
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
          'action_name': 'make_v9_strings_cc',
          'inputs': [
            'dom/make_v8_strings.py',
            'dom/v8_strings.in',
          ], # inputs
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings.cc',
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings.h',
          ],
          'action': [
            'python',
            'dom/make_v8_strings.py',
            '<(SHARED_INTERMEDIATE_DIR)/v8_strings',
            'dom/v8_strings.in',
          ], # action
        },
      ], # actions
    },
    {
      'target_name': 'text',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/regex/regex.gyp:regex_lib',
      ], # dependencies
      'sources': [
        'precomp.cpp',

        'css/color.cc',
        'css/style.cc',
        'css/style_resolver.cc',
        'css/style_rule.cc',
        'css/style_sheet.cc',
        'css/style_sheet_observer.cc',
        'css/style_selector.cc',

        'ed_BufferCore.cpp',
        'ed_BufferFind.cpp',
        'li_util.cpp',
        'text/buffer.cc',
        'text/buffer_mutation_observer.cc',
        'text/interval.cc',
        'text/interval_set.cc',
        'text/marker.cc',
        'text/marker_set.cc',
        'text/marker_set_observer.cc',
        'text/modes/clike_lexer.cc',
        'text/modes/config_mode.cc',
        'text/modes/cxx_mode.cc',
        'text/modes/haskell_mode.cc',
        'text/modes/java_mode.cc',
        'text/modes/javascript_mode.cc',
        'text/modes/lexer.cc',
        'text/modes/lisp_mode.cc',
        'text/modes/mason_mode.cc',
        'text/modes/mode.cc',
        'text/modes/perl_mode.cc',
        'text/modes/plain_text_mode.cc',
        'text/modes/python_mode.cc',
        'text/modes/xml_mode.cc',
        'text/range.cc',
        'text/range_set.cc',
        'text/undo_stack.cc',
        'text/undo_step.cc',
        'z_debug.cpp',
        '../charset/CharsetDecoder.cpp',
        '../charset/CharsetDetector.cpp',
      ], # sources
    }, # text
    {
      'target_name': 'editor',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'io',
        'views',
      ],
      'sources': [
        'precomp.cpp',

        'ctrl_TitleBar.cpp',
        'editor/application.cc',
        'editor/modal_message_loop_scope.cc',
        'editor/dom_lock.cc',
        'gfx_base.cpp',
        'gfx/color_f.cc',
        'gfx/point_f.cc',
        'gfx/size_f.cc',
        'gfx/stroke_style.cc',
        'gfx/rect_f.cc',
        'metrics/counter.cc',
        'metrics/time_scope.cc',
        'views/frame_list.cc',
        'views/frame_observer.cc',
        'views/tab_strip.cc',
        'views/tab_strip_delegate.cc',
        'views/text/render_cell.cc',
        'views/text/render_style.cc',
        'views/text/render_text_block.cc',
        'views/text/render_text_line.cc',
        'views/text/screen_text_block.cc',
        'views/text/text_formatter.cc',
        'views/text/text_renderer.cc',
        'vi_Caret.cpp',
        'vi_EditPane.cpp',
        'views/forms/dialog_box.cc',
        'views/forms/dialog_box_set.cc',
        'views/forms/file_dialog_box.cc',
        'views/forms/find_dialog_box.cc',
        'vi_Frame.cpp',
        'vi_Pane.cpp',
        'vi_Selection.cpp',
        'vi_Style.cpp',
        'vi_TextEditWindow.cpp',
      ], # sources
    }, # editor
    {
      'target_name': 'io',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'dom',
        'text',
        'views',
      ],
      'sources': [
        'precomp.cpp',

        'io/file_io.cc',
        'io/file_io_context.cc',
        'io/io_context.cc',
        'io/io_context_utils.cc',
        'io/io_delegate_impl.cc',
        'io/io_manager.cc',
        'io/process_io_context.cc',
      ], # sources
    }, # io
    {
      'target_name': 'spellchecker',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/third_party/hunspell/hunspell.gyp:hunspell',
      ], # dependencies
      'defines': [ 'HUNSPELL_CHROME_CLIENT' ],
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'msvs_disabled_warnings': [
        # L4 warning C4127: conditional expression is constant
        # For DCHECK_XX(x)
        4127,
        # L1 warning  C4251: 'identifier' : class 'type' needs to have
        # dll-interface to be used by clients of class 'type2'
        # Example: std::unique_ptr<T>
        4251,
        # L1 warning C4350: behavior change: 'member1' called instead of
        # 'member2' An rvalue cannot be bound to a non-const reference. In
        # previous versions of Visual C++, it was possible to bind an rvalue
        # to a non-const reference in a direct initialization. This code now
        # gives a warning.
        4530,
        # L3 warning C4800: type' : forcing value to bool 'true' or 'false'
        # (performance warning)
        4800,
      ],
      'sources': [
        'spellchecker/hunspell_engine.cc',
        'spellchecker/spelling_engine.cc',
      ], # sources
    }, # spellchecker
    {
      'target_name': 'ui',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'msvs_precompiled_header': '<(DEPTH)/build/precompile.h',
      'msvs_precompiled_source': '<(DEPTH)/build/precompile.cc',
      'sources': [
        '<(DEPTH)/build/precompile.cc',

        'ui/base/selection_model.cc',
        'ui/base/table_model.cc',
        'ui/base/table_model_observer.cc',

        'ui/events/event.cc',
        'ui/events/event_ostream.cc',

        'ui/controls/scroll_bar.cc',
        'ui/controls/scroll_bar_observer.cc',
        'ui/controls/status_bar.cc',
        'ui/controls/table_control.cc',
        'ui/controls/table_control_observer.cc',

        'ui/root_widget.cc',
        'ui/widget.cc',
        'ui/widget_ostream.cc',
      ], # sources
    },
    {
      'target_name': 'v8_glue',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/gin/gin.gyp:gin',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
      ], # dependencies
      'msvs_precompiled_header': '',
      'msvs_precompiled_source': '',
      'msvs_disabled_warnings': [
        # warning C4127: conditional expression is constant
        # For DCHECK_XX(x)
        4127,
        # warning  C4251: 'identifier' : class 'type' needs to have
        # dll-interface to be used by clients of class 'type2'
        # Example: std::unique_ptr<T>
        4251,
        # warning C4350: behavior change: 'member1' called instead of
        # 'member2' An rvalue cannot be bound to a non-const reference. In
        # previous versions of Visual C++, it was possible to bind an rvalue
        # to a non-const reference in a direct initialization. This code now
        # gives a warning.
        4530,
      ],
      'sources': [
        'v8_glue/array_buffer_view.cc',
        'v8_glue/constructor_template.cc',
        'v8_glue/converter.cc',
        'v8_glue/function_template_builder.cc',
        'v8_glue/isolate_holder.cc',
        'v8_glue/per_isolate_data.cc',
        'v8_glue/runner.cc',
        'v8_glue/runner_delegate.cc',
        'v8_glue/scriptable.cc',
        'v8_glue/script_callback.cc',
        'v8_glue/wrapper_info.cc',
      ], # sources
    }, # v8_glue
    {
      'target_name': 'views',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'dom',
        'spellchecker',
        'ui',
      ],
      'sources': [
        'precomp.cpp',

        'views/icon_cache.cc',
        'views/view_delegate_impl.cc',
        'views/window.cc',
        'views/window_set.cc',

        'views/command_window.cc',
        'views/content_window.cc',
        'views/message_view.cc',
        'views/table_view_model.cc',
        'views/table_view.cc',
      ], # sources
    },
  ], # targets
}
