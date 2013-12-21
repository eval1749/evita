# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'conditions': [ ['evita_code==1', {
  'target_defaults': {
    'include_dirs': [
      '<(DEPTH)',
    ],
  }, # target_defaults

  'conditions': [
    ['component=="shared_library"', {
      'target_defaults': {
        'defines': ['COMPONENT_BUILD'],
       }, # target_defaults
    }], # component=="shared_library"

    ['OS=="win"', {
      'target_defaults': {
        'defines': [
          # See SDK version in include/shared/sdkddkver.h
          '_WIN32_WINNT=0x0602', # _WIN32_WINNT_WIN8
          'WINVER=0x0602', # _WIN32_WINNT_WIN8
          'WIN32',
          '_WINDOWS',
        ], # defines

        # Precompiled header
        # See gyp/pylib/gyp/msvs_settings.py for details
        #'msvs_precompiled_header': '<(DEPTH)/build/precomp.h',
        #'msvs_precompiled_source': '<(DEPTH)/build/precomp.cc',
        #'sources': ['<(DEPTH)/build/precomp.cc'],

        'msvs_settings': {
          'VCCLCompilerTool': {
            'ExceptionHandling': '0',
            'WarningLevel': '4', # /Wall
            'WarnAsError': 'true', # /WX
            'AdditionalOptions': [
              '/GR-', # Enables run-time type information (RTTI).
              #'/Gr', # Uses the __fastcall calling convention (x86 only).
              '/Zc:forScope',
              '/Zc:wchar_t',
              '/analyze-',
              '/arch:SSE2',
              '/errorReport:prompt',
              '/fp:except-',
              '/fp:fast',
            ],
          }, # VCCLCompilerTool
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'kernel32.lib',
              'advapi32.lib',
              'user32.lib',
            ], # AdditionalDependencies
            # 0=not set
            # 1=/SUBSYSTEM:CONSOLE
            # 2=/SUBSYSTEM:WINDOWS
            'SubSystem': 1,
           }, # VCLinkerTool
           'target_conditions': [
              ['_type=="executable"', {
                'VCManifestTool': {
                  'EmbedManifest': 'true',
                 },
              }],
        ], # target_conditions
        }, # msvs_settings
       } # target_defaults
    }], # OS=="win"
  ], # conditions
}]]}
