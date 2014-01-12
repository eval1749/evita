# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'target_defaults': {
    'include_dirs+' : [
      # TODO(yosi): We should not have |<(DEPTH)/evita| in |include_dirs|.
      '<(DEPTH)/evita',
    ], # include_dirs
  }, # target_defaults

  'targets': [
    {
      'target_name': 'evita',
      'type': 'executable',
      'dependencies': [
        'ui',
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
        'dom_jslib',
        'dom_unicode_lib_icu',
        'text',
        'v8_glue',
      ],
      'sources': [
        'precomp.cpp',

        'dom/buffer.cc',
        'dom/converter.cc',
        'dom/document.cc',
        'dom/editor.cc',
        'dom/editor_window.cc',
        'dom/events/event.cc',
        'dom/events/event_target.cc',
        'dom/file_path.cc',
        'dom/lock.cc',
        'dom/range.cc',
        'dom/selection.cc',
        'dom/script_command.cc',
        'dom/script_controller.cc',
        'dom/script_thread.cc',
        'dom/text_window.cc',
        'dom/time_stamp.cc',
        'dom/v8_console_buffer.cc',
        'dom/window.cc',

        'gc/collectable.cc',
        'gc/collector.cc',
        'gc/visitable.cc',
      ], # sources
    }, # dom
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
      'variables': {
        'library_files': [
          'dom/polyfill.js',
          'dom/unicode.js',
          'dom/types.js',
          'dom/key_names.js',
          'dom/runtime.js',
          'dom/messages_en_US.js',

          'dom/console.js',
          'dom/document.js',
          'dom/editor.js',
          'dom/editor_window.js',
          'dom/file_path.js',
          'dom/range.js',
          'dom/selection.js',
          'dom/key_bindings.js',
        ],
      }, # variables
      'actions': [
        {
          'action_name': 'js2c',
          'inputs': [
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<@(library_files)',
          ], # inputs
          'outputs': [ '<(SHARED_INTERMEDIATE_DIR)/dom_jslib.cc' ],
          'action': [
            'python',
            '<(DEPTH)/evita/dom/make_get_jslib.py',
            '<@(_outputs)',
            '<@(library_files)',
          ], # action
        }, # js2c
      ], # actions
    }, # dom_jslib_js2c
    {
      'target_name': 'dom_unicode_lib_icu',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base_i18n',
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
      'dependencies': [ 'dom_unicode_lib_cc' ],
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
      'target_name': 'text',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/regex/regex.gyp:regex_lib',
      ], # dependencies
      'sources': [
        'precomp.cpp',
        'text/buffer.cc',
        'text/range.cc',
        'core/style.cc',

        'ed_BufferCore.cpp',
        'ed_BufferFind.cpp',
        'ed_Interval.cpp',
        'ed_Mode.cpp',
        'ed_Undo.cpp',
        'li_util.cpp',
        'mode_Config.cpp',
        'mode_Cxx.cpp',
        'mode_Haskell.cpp',
        'mode_Lisp.cpp',
        'mode_Mason.cpp',
        'mode_Perl.cpp',
        'mode_PlainText.cpp',
        'mode_Python.cpp',
        'mode_Xml.cpp',
        'RegexMatch.cpp',
        'z_debug.cpp',

        '../charset/CharsetDecoder.cpp',
        '../charset/CharsetDetector.cpp',
      ], # sources
    }, # text
    {
      'target_name': 'ui',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'dom',
      ],
      'sources': [
        'precomp.cpp',

        'ap_input_history.cpp',
        'cm_CmdProc.cpp',
        'cm_commands.cpp',
        'content/buffer_list_window.cc',
        'content/content_window.cc',
        'ctrl_StatusBar.cpp',
        'ctrl_TabBand.cpp',
        'ctrl_TitleBar.cpp',
        'editor/application.cc',
        'editor/dialog_box.cc',
        'editor/dom_lock.cc',
        'ExactStringMatch.cpp',
        'gfx_base.cpp',
        'vi_Caret.cpp',
        'vi_DialogBox.cpp',
        'vi_EditPane.cpp',
        'vi_FileDialogBox.cpp',
        'vi_FileIo.cpp',
        'vi_FindDialogBox.cpp',
        'vi_Frame.cpp',
        'vi_IoManager.cpp',
        'vi_Page.cpp',
        'vi_Pane.cpp',
        'vi_Selection.cpp',
        'vi_Style.cpp',
        'vi_TextEditWindow.cpp',

        'view/view_delegate_impl.cc',
        'view/window.cc',
        'view/window_set.cc',

        'widgets/root_widget.cc',
        'widgets/widget.cc',
      ], # sources
    }, # ui
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
        'v8_glue/constructor_template.cc',
        'v8_glue/converter.cc',
        'v8_glue/function_template_builder.cc',
        'v8_glue/isolate_holder.cc',
        'v8_glue/per_isolate_data.cc',
        'v8_glue/scriptable.cc',
        'v8_glue/script_callback.cc',
        'v8_glue/wrapper_info.cc',
      ], # sources
    }, # v8_glue
  ], # targets
}
