# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'includes': [
    'dom/dom.gypi',
    'text/encodings/encodings.gypi',
    'ui/ui.gypi',
   ], # includes

  'targets': [
    {
      'target_name': 'evita',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
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
        'text/scoped_undo_group.cc',
        'text/scoped_undo_group.h',
        'text/selection.cc',
        'text/selection.h',
        'text/selection_change_observer.cc',
        'text/selection_change_observer.h',
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
        '<(DEPTH)/common/common.gyp:common',
        'gfx',
        'io',
        'views',
      ],
      'sources': [
        'precomp.cpp',

        'ctrl_TitleBar.cpp',
        'editor/application.cc',
        'editor/application_proxy.cc',
        'editor/application_proxy.h',
        'editor/modal_message_loop_scope.cc',
        'editor/dom_lock.cc',
        'editor/switch_set.cc',
        'editor/switch_set.h',
        'metrics/counter.cc',
        'metrics/sampling.cc',
        'metrics/sampling.h',
        'metrics/time_scope.cc',
        'views/frame_list.cc',
        'views/frame_observer.cc',
        'views/tabs/tab.cc',
        'views/tabs/tab.h',
        'views/tabs/tab_data_set.cc',
        'views/tabs/tab_content.cc',
        'views/tabs/tab_content.h',
        'views/tabs/tab_content_observer.cc',
        'views/tabs/tab_content_observer.h',
        'views/tabs/tab_strip.cc',
        'views/tabs/tab_strip_animator.cc',
        'views/tabs/tab_strip_animator.h',
        'views/tabs/tab_strip_delegate.cc',
        'views/text/render_cell.cc',
        'views/text/render_font.cc',
        'views/text/render_font.h',
        'views/text/render_font_set.cc',
        'views/text/render_font_set.h',
        'views/text/render_selection.cc',
        'views/text/render_selection.h',
        'views/text/render_style.cc',
        'views/text/render_text_block.cc',
        'views/text/render_text_line.cc',
        'views/text/screen_text_block.cc',
        'views/text/text_formatter.cc',
        'views/text/text_renderer.cc',
        'views/text_window.cc',
        'views/text_window.h',
        'vi_EditPane.cpp',
        'views/forms/file_dialog_box.cc',
        'views/forms/form_control_controller.cc',
        'views/forms/form_control_set.cc',
        'views/forms/form_window.cc',
        'vi_Frame.cpp',
      ], # sources
    }, # editor
    {
      'target_name': 'gfx',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        'precomp.cpp',

        'gfx/bitmap.cc',
        'gfx/color_f.cc',
        'gfx/font_face.cc',
        'gfx/canvas.cc',
        'gfx/canvas_observer.cc',
        'gfx/canvas_observer.h',
        'gfx/dx_device.cc',
        'gfx/dx_device.h',
        'gfx/point_f.cc',
        'gfx/rect_conversions.cc',
        'gfx/rect_conversions.h',
        'gfx/rect_f.cc',
        'gfx/size_f.cc',
        'gfx/stroke_style.cc',
        'gfx/swap_chain.cc',
        'gfx/swap_chain.h',
        'gfx/text_format.cc',
        'gfx/text_layout.cc',
        'gfx_base.cpp',
      ], # sources
    }, # gfx
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
        'v8_glue/v8_platform.cc',
        'v8_glue/v8_platform.h',
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

        'views/content_observer.cc',
        'views/content_observer.h',
        'views/content_window.cc',
        'views/message_view.cc',
        'views/metrics_view.cc',
        'views/metrics_view.h',
        'views/table_view_model.cc',
        'views/table_view_model.h',
        'views/table_window.cc',
        'views/table_window.h',
      ], # sources
    },
  ], # targets
}
