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
   4055,
             # warning C4100: 'identifier' : unreferenced formal parameter
             4100,
             # warning C4127: conditional expression is constant
             4127,
             # warning C4131: 'function' : uses old-style declarator
             4131,
             # warning C4152: non standard extension, function/data ptr
             # conversion in expression
             4152,
    4189,
    4244,
             # warning C4245: 'conversion' : conversion from 'type1' to
             # 'type2', signed/unsigned mismatch
             4245,
             # warning C4255: 'function' : no function prototype given:
             # converting '()' to '(void)'
             4255,
             # warning C4310: cast truncates constant value
             4310, # Level 3
             # warning C4512: 'class' : assignment operator could not be
             # generated
             4512,
             # warning C4530: C++ exception handler used, but unwind semantics
             # are not enabled. Specify /EHsc
             4530, # Level 1

             # C4541: 'identifier' used on polymorphic type 'type' with /GR-;
             # unpredictable behavior may result
             4541, # Level

             # warning C4625: derived class' : copy constructor could not be
             # generated because a base class copy constructor is inaccessible
             4625,
             # warning C4668: 'symbol' is not defined as a preprocessor macro,
             # replacing with '0' for 'directives'
             4668,
    4706,
             # warning C4820: 'bytes' bytes padding added after construct
             # 'member_name'
             4820,
             # warning C4996: 'function': was declared deprecated
             4996, # Level 3
            ] # msvs_disabled_warnings
      }], # OS=="win"
    ], # target_conditions
  }, # target_defaults
}
