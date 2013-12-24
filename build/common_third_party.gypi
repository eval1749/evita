# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'target_defaults': {
    'target_conditions': [
      ['OS=="win" and chromium_code==0 and evita_code==0', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'conditions': [
                ['component=="shared_library"', {
                  'ExceptionHandling': '0',
                }, {
                  'ExceptionHandling': '0',
                }],
              ], # conditions
              'AdditionalOptions': ['/MP'],
              'MinimalRebuild': 'false',
              'BufferSecurityCheck': 'true',
              'EnableFunctionLevelLinking': 'true', # /Gy
              'RuntimeLibrary': '<(win_debug_RuntimeLibrary)',
              'RuntimeTypeInfo': 'false', # /GR-
              'WarningLevel': '3', # /Wall
              'WarnAsError': 'false', # no /WX
              'DebugInformationFormat': '3',
            }, # VCCLCompilerTool
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'kernel32.lib',
                'advapi32.lib',
              ],
             }, # VCLinkerTool
          }, # msvs_settings
          'msvs_disabled_warnings': [
            # level 1
            4530,
            4541,
            # level 2
            4244,
            # level 3
            4800,
            4996,
          ] # msvs_disabled_warnings
      }], # OS=="win"
    ], # target_conditions
  }, # target_defaults
}
