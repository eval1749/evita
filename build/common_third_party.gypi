# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'conditions': [
    ['OS=="win"', {
      'target_defaults': {
        'msvs_settings': {
          'VCCLCompilerTool': {
            'conditions': [
              ['component=="shared_library"', {
                'ExceptionHandling': '0',
              }, {
                'ExceptionHandling': '0',
              }],
            ], # conditions
           }, # VCCLCompilerTool
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'kernel32.lib',
              'advapi32.lib',
            ],
           }, # VCLinkerTool
        }, # msvs_settings
        'msvs_disabled_warnings': [
           # for ICU

           # warning C4100: 'identifier' : unreferenced formal parameter
           4100,
           # warning C4152: non standard extension, function/data ptr
           # conversion in expression
           4152,
           # warning C4255: 'function' : no function prototype given:
           # converting '()' to '(void)'
           4255,
           # warning C4310: cast truncates constant value
           4310, # Level 3
           # warning C4530: C++ exception handler used, but unwind semantics
           # are not enabled. Specify /EHsc
           4530, # Level 1
           # warning C4625: derived class' : copy constructor could not be
           # generated because a base class copy constructor is inaccessible
           4625,
           # warning C4668: 'symbol' is not defined as a preprocessor macro,
           # replacing with '0' for 'directives'
           4668,
           # warning C4820: 'bytes' bytes padding added after construct
           # 'member_name'
           4820,
          ] # msvs_disabled_warnings
       } # target_defaults
    }], # OS=="win"
  ], # construct
}