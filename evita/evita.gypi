# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    'dom/dom.gypi',
    'text/encodings/encodings.gypi',
   ], # includes

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
      'target_name': 'text',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/regex/regex.gyp:regex_lib',
        'encodings',
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
        'text/line_number_cache.cc',
        'text/marker.cc',
        'text/marker_set.cc',
        'text/marker_set_observer.cc',
        'text/range.cc',
        'text/range_base.cc',
        'text/range_base.h',
        'text/range_set.cc',
        'text/range_set_base.cc',
        'text/range_set_base.h',
        'text/undo_stack.cc',
        'text/undo_step.cc',
        'z_debug.cpp',
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
        'editor/switch_set.cc',
        'editor/switch_set.h',
        'gfx/bitmap.cc',
        'gfx/color_f.cc',
        'gfx/font_face.cc',
        'gfx/graphics.cc',
        'gfx/point_f.cc',
        'gfx/rect_f.cc',
        'gfx/size_f.cc',
        'gfx/stroke_style.cc',
        'gfx/text_format.cc',
        'gfx/text_layout.cc',
        'gfx_base.cpp',
        'metrics/counter.cc',
        'metrics/time_scope.cc',
        'views/frame_list.cc',
        'views/frame_observer.cc',
        'views/tab_data_set.cc',
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
        'views/forms/file_dialog_box.cc',
        'views/forms/form_control_controller.cc',
        'views/forms/form_control_set.cc',
        'views/forms/form_window.cc',
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
        'ui/events/mouse_click_tracker.cc',

        'ui/controls/button_control.cc',
        'ui/controls/checkbox_control.cc',
        'ui/controls/control.cc',
        'ui/controls/control_controller.cc',
        'ui/controls/label_control.cc',
        'ui/controls/radio_button_control.cc',
        'ui/controls/scroll_bar.cc',
        'ui/controls/scroll_bar_observer.cc',
        'ui/controls/status_bar.cc',
        'ui/controls/table_control.cc',
        'ui/controls/table_control_observer.cc',
        'ui/controls/text_field_control.cc',

        'ui/caret.cc',
        'ui/root_widget.cc',
        'ui/system_metrics.cc',
        'ui/system_metrics_observer.cc',
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

        'views/event_source.cc',
        'views/icon_cache.cc',
        'views/switches.h',
        'views/switches.cc',
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
