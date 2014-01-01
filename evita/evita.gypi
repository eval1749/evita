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
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/gin/gin.gyp:gin',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'text',
      ],
      'sources': [
        'precomp.cpp',

        'dom/console.cc',
        'dom/buffer.cc',
        'dom/editor.cc',
        'dom/editor_window.cc',
        'dom/lock.cc',
        'text/range.cc',
        'dom/script_command.cc',
        'dom/script_controller.cc',
        'dom/script_thread.cc',
        'dom/window.cc',

        'gc/collectable.cc',
        'gc/collector.cc',
        'gc/visitable.cc',

        'v8_glue/constructor_template.cc',
        'v8_glue/converter.cc',
        'v8_glue/isolate_holder.cc',
        'v8_glue/per_isolate_data.cc',
        'v8_glue/scriptable.cc',
        'v8_glue/v8_console_buffer.cc',
        'v8_glue/wrapper_info.cc',
      ], # sources
    }, # dom
    {
      'target_name': 'text',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/regex/regex.gyp:regex_lib',
      ], # dependencies
      'sources': [
        'precomp.cpp',
        'core/buffer.cc',
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
        'vi_Gateway.cpp',
        'vi_IoManager.cpp',
        'vi_Listener.cpp',
        'vi_Page.cpp',
        'vi_Pane.cpp',
        'vi_Selection.cpp',
        'vi_Style.cpp',
        'vi_TextEditWindow.cpp',

        'view/view_delegate_impl.cc',

        'widgets/root_widget.cc',
        'widgets/widget.cc',
      ], # sources
    }, # ui
  ], # targets
}
