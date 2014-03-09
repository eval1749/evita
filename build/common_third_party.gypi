# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'target_defaults': {
    'target_conditions': [
      ['OS=="win" and chromium_code==0 and evita_code==0', {
          'defines': [
            'NOMINMAX',
            # See SDK version in include/shared/sdkddkver.h
            '_WIN32_WINNT=0x0602', # _WIN32_WINNT_WIN8
            '_WINDOWS',
            'WIN32',
            'WIN32_LEAN_AND_MEAN',
            'WINVER=0x0602', # _WIN32_WINNT_WIN8
          ], # defines
          'msvs_settings': {
            'VCCLCompilerTool': {
              'conditions': [
                ['component=="shared_library"', {
                  'ExceptionHandling': '1',
                }, {
                  'ExceptionHandling': '0',
                }],
              ], # conditions
              'AdditionalOptions': ['/MP'],
              'WarningLevel': '3', # /Wall
              'WarnAsError': 'false', # no /WX
            }, # VCCLCompilerTool
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'kernel32.lib',
                'advapi32.lib',
              ],
             }, # VCLinkerTool
          }, # msvs_settings
          'msvs_disabled_warnings': [
            # L2 C4244: 'conversion' conversion from 'type1' to 'type2', possible
            # loss of data
            4244,
            # L1 C4251: 'identifier' : class 'type' needs to have dll-interface to
            # be used by clients of class 'type2'
            4251,
            # L1 C4530: C++ exception handler used, but unwind semantics are not
            # enabled. Specify /EHsc
            4530,
            # L3 C4800: 'type' : forcing value to bool 'true' or 'false'
            # (performance warning)
            4800,
            # L3 C4996: 'function': was declared deprecated
            4996,
          ] # msvs_disabled_warnings
      }], # OS=="win"
    ], # target_conditions
  }, # target_defaults
}
