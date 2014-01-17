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
            # level 1
            # C4530: C++ exception handler used, but unwind semantics are not
            # enabled. Specify /EHsc
            4530,
            # level 2
            # C4244: 'conversion' conversion from 'type1' to 'type2', possible
            # loss of data
            4244,
            # level 3
            # C4800: 'type' : forcing value to bool 'true' or 'false'
            # (performance warning)
            4800,
            # C4996: 'function': was declared deprecated
            4996,
          ] # msvs_disabled_warnings
      }], # OS=="win"
    ], # target_conditions
  }, # target_defaults
}
