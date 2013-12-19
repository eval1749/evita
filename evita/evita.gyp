# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'targets': [
    {
      'target_name': 'evita',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/regex/regex.gyp:*',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/evita',
      ], # include_dirs

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

      # Precompiled header
      # See gyp/pylib/gyp/msvs_settings.py for details
      'msvs_precompiled_header': 'precomp.h',
      'msvs_precompiled_source': 'precomp.cpp',

      'sources': [
        'precomp.cpp',
        'ap_input_history.cpp',
        #'ap_lisp_thread.cpp',
        #'ap_listener_buffer.cpp',
        'ap_main.cpp',
        'cm_CmdProc.cpp',
        'cm_commands.cpp',
        'ctrl_StatusBar.cpp',
        'ctrl_TabBand.cpp',
        'ctrl_TitleBar.cpp',
        'ed_Buffer.cpp',
        'ed_BufferCore.cpp',
        'ed_BufferFind.cpp',
        'ed_Interval.cpp',
        'ed_Mode.cpp',
        'ed_Range.cpp',
        'ed_Undo.cpp',
        #'evita.rc',
        'ExactStringMatch.cpp',
        'gfx_base.cpp',
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
        'vcsupport.cpp',
        'vi_Application.cpp',
        'vi_Buffer.cpp',
        'vi_BufferListPane.cpp',
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
        'z_debug.cpp',

        'base/timer/timer.cpp',
        'gfx/rect.cpp',
        'widgets/container_widget.cpp',
        'widgets/naitive_window.cpp',
        'widgets/widget.cpp',

        '../charset/CharsetDecoder.cpp',
        '../charset/CharsetDetector.cpp',
      ], # sources
    }, # evita
  ], # targets
}
