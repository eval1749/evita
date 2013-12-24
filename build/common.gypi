# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'variables': {
      'variables': {
        'clang': 0,
        'component%': 'shared_library', # or 'static_library'
        'target_arch%': 'ia32', # 'ia32', 'x64'
      }, # variables
      'clang': '<(clang)',
      'component%': '<(component)',
      'target_arch%': '<(target_arch)',
    }, # variables
    'clang%': '<(clang)',
    'component%': '<(component)',
    'target_arch%': '<(target_arch)',

    'chromium_code': 0,

    # .gyp files or targets should set |evita_code| to 1 if they build
    # Evita-specific code, as opposed to external code. This variable is
    # used to control such things as the set of warnings to enable, and
    # whether warnings are treated as errors.
    'evita_code%': 0,

    # Relative path to icu.gyp from this file.
    'icu_gyp_path%': '<(DEPTH)/third_party/icu/icu.gyp',
    'os_posix': 0,
    'target_arch%': '<(target_arch)',
    'v8_enable_i18n_support': 1,
    'v8_optimized_debug%': '(<v8_optimized_debug)',
  }, # variables

  'conditions': [
    ['chromium_code==1', {
      'includes': [ 'common_chromium.gypi' ]
    }],
    ['evita_code==1', {
      'includes': [ 'common_evita.gypi' ]
    }],
    ['chromium_code==0 and evita_code==0', {
      'includes': [ 'common_third_party.gypi' ]
    }],
    ['OS=="win"', {
      'default_configuration': 'Debug',

      'target_defaults': {
        'msvs_cygwin_dirs': ['<(DEPTH)/third_party/cygwin'],
        'msvs_cygwin_shell': 0,

        'configurations': {
          'common_base': {
            'abstract': 1,

            'msvs_configuration_attributes': {
              'OutputDirectory': '<(DEPTH)\\..\$(ConfigurationName)',
              'IntermediateDirectory': '<(DEPTH)\\..\$(ConfigurationName)\\$(ProjectName)',
              'CharacterSet': '1',
            },

            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalDependencies': [
                  'kernel32.lib',
                ],
                'SubSytem': 1,
              }, # VCLinkerTool
            }, # msvs_settings
          }, # common_base

          'x86_base': {
            'abstract': 1,
            'msvs_settings': {
              'VCLinkerTool': {
                'AdditionalOptions': [
                  '/safeseh',
                  #'/dynamicbase',
                  #'/nxcompat',
                ], # AdditionalOptions
                'TargetMachine': '1',
              },
            }, # msvs_settings
          }, # x86_base

          'debug_base': {
            'abstract': 1,
            'msvs_settings': {
              'VCCLCompilerTool': {
                'Optimization': '0', # /Od
                'PreprocessorDefinitions': ['_DEBUG'],
                'AdditionalOptions': [
                  '/GF-', # Enables string pooling.
                  '/GS', # Buffer security check
                  '/Gm-', # Enables minimal rebuild.
                  '/Gy-', # Enables function-level linking.
                  '/MTd', # Creates a debug multithreaded executable file using LIBCMTD.lib.
                  '/Oy-', # Omits frame pointer (x86 only).
                  '/Zi', # Generates complete debugging information.
                ],
                'conditions': [
                  ['evita_code==1', {
                    'AdditionalOptions': [
                      '/RTC1', # Enables run-time error checking.
                    ],
                  }],
                ], # conditions
              }, # VCCLCompilerTool
              'VCLinkerTool': {
                'GenerateDebugInformation': 'true',
                'GenerateMapFile': 'true',
              }, # VCLinkerTool
            },
          }, # debug_base

          'release_base': {
            'abstract': 1,
            'msvs_settings': {
              # Note: GYP add /FS(Forces writes to the program database (PDB)
              # file to be serialized through MSPDBSRV.EXE.) for MSVS=|2013|
              # or # |2013e|.
              'VCCLCompilerTool': {
                'EnableIntrinsicFunctions': 'true', # /Gy
                'EnableFiberSafeOptimizations': 'true', # /GT
                'FavorSizeOrSpeed': '2', # /Os Favors small code.
                'PreprocessorDefinitions': ['NDEBUG'],
                'Optimization': '3',
                'AdditionalOptions': [
                  '/GF', # Enables string pooling.
                  '/GL', # Enables whole program optimization.
                  '/GS-', # Buffer security check
                  '/Gm-', # Enables minimal rebuild.
                  '/MT', # Creates a multithreaded executable file using LIBCMT.lib.
                  '/Oi', # Generates intrinsic functions.
                  '/Oy', # Omits frame pointer (x86 only).
                ],
                'WholeProgramOptimization': 'true', # /Ox
              }, # VCCLCompilerTool
              'VCLinkerTool': {
                'EnableCOMDATFolding': '2',
                'GenerateMapFile': 'true',
                'OptimizeReferences': '2',
              }, # VCLinkerTool
            },
          }, # release_base

          # Concrete configurations
          'Debug' : {
            'inherit_from': ['common_base', 'x86_base', 'debug_base'],
          },

          'Release' : {
            'inherit_from': ['common_base', 'x86_base', 'release_base'],
          },
        }, # configurations
      }, # target_defaults
    }], # OS="win"
  ], # conditions
}
