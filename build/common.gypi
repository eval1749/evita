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
    'host_arch%': 'x64',
    'target_arch%': '<(target_arch)',
    # For hunspell
    'gcc_version': '48',

    # Relative path to icu.gyp from this file.
    'icu_gyp_path%': '<(DEPTH)/third_party/icu/icu.gyp',
    'icu_use_data_file_flag%': 0,
    'os_posix': 0,
    'target_arch%': '<(target_arch)',
    'use_custom_libcxx%': 0,
    'v8_enable_i18n_support': 1,
    'v8_optimized_debug%': '(<v8_optimized_debug)',
  }, # variables

  'includes': [
    'common_chromium.gypi',
    'common_evita.gypi',
    'common_third_party.gypi',
  ], # includes

  'target_defaults': {
    'variables': {
      'chromium_code%': '<(chromium_code)',
      'evita_code%': '<(evita_code)',
     }, # variables

     # Choose MSVCRT flavor: MT:multithreaded
     'conditions': [
       ['OS=="win" and component=="shared_library"', {
          'variables': {
            'win_release_RuntimeLibrary%': 2, # /MD
            'win_debug_RuntimeLibrary%': 3, # /MDd
          }
       }, {
          'variables': {
            'win_release_RuntimeLibrary%': 0, # /MT
            'win_debug_RuntimeLibrary%': 1, # /MTd
          }
      }], # OS=="win" and component=="shared_library"
      ['component=="shared_library"', {
        'defines': ['COMPONENT_BUILD'],
      }] # component=="shared_library"
    ], # conditions

    'msvs_cygwin_dirs': ['<(DEPTH)/third_party/cygwin'],
    'msvs_cygwin_shell': 0,

    'default_configuration': 'Debug',
    'configurations': {
      'Common_Base': {
        'abstract': 1,

        'msvs_configuration_attributes': {
          'OutputDirectory': '<(DEPTH)\\..\$(ConfigurationName)',
          'IntermediateDirectory': '<(DEPTH)\\..\$(ConfigurationName)\\$(ProjectName)',
          'CharacterSet': '1',
        },
      }, # Common_Base

      'x86_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': [
              '/arch:SSE2',
            ],
          }, # VCCLCompilerTool
          'VCLinkerTool': {
            'AdditionalOptions': [
              '/safeseh',
              #'/dynamicbase',
              '/nxcompat',
            ], # AdditionalOptions
            'TargetMachine': '1',
          },
        }, # msvs_settings
      }, # x86_Base

      'x64_Base': {
        'abstract': 1,
        'msvs_configuration_platform': 'x64',
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalOptions': [
              # safeseh is not compatible with x64
              '/dynamicbase',
              '/ignore:4199',
              '/ignore:4221',
              '/nxcompat',
            ],
            'TargetMachine': '17', # x86 - 64
          }, # VCLinkerTool
        },
      }, # x64_Base

      'Debug_Base': {
        'abstract': 1,
        'msvs_settings': {
          'VCCLCompilerTool': {
            'BufferSecurityCheck': 'true',
            'DebugInformationFormat': '3',
            'MinimalRebuild': 'false',
            'Optimization': '0', # /Od
            'PreprocessorDefinitions': ['_DEBUG'],
            'RuntimeLibrary': '<(win_debug_RuntimeLibrary)',
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
      }, # Debug_Base

      'Release_Base': {
        'abstract': 1,
        'msvs_settings': {
          # Note: GYP add /FS(Forces writes to the program database (PDB)
          # file to be serialized through MSPDBSRV.EXE.) for MSVS=|2013|
          # or # |2013e|.
          'VCCLCompilerTool': {
            'BufferSecurityCheck': 'false', # /GS-
            'DebugInformationFormat': '3', # /Zi
            'EnableFiberSafeOptimizations': 'true', # /GT
            'EnableFunctionLevelLinking': 'true', # /Gy
            'EnableIntrinsicFunctions': 'true', # /Oi
            'EnableFiberSafeOptimizations': 'true', # /GT
            'FavorSizeOrSpeed': '2', # /Os Favors small code.
            'InlineFunctionExpansion': '2', # /Ob2
            'MinimalRebuild': 'false', # /Gm
            'OmitFramePointers': 'true', # /Oy
            'Optimization': '3', # /Ox
            'PreprocessorDefinitions': ['NDEBUG'],
            'RuntimeLibrary': '<(win_release_RuntimeLibrary)',
            'StringPooling': 'true', # /GF
            'WholeProgramOptimization': 'true', # /GL
          }, # VCCLCompilerTool
          'VCLibrarianTool': {
            'LinkTimeCodeGeneration': 'true', # /LTCG
          }, # VCLibrarianTool
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'dbghelp.lib',
              'psapi.lib',
            ], # AdditionalDependencies
            'EnableCOMDATFolding': 2,
            'GenerateMapFile': 'true',
            'LargeAddressAware': 2,
            'LinkIncremental':  1,
            'LinkTimeCodeGeneration': 1, # /LTCG
            'OptimizeReferences': 2,
            'RandomizedBaseAddress': 1,
          }, # VCLinkerTool
        },
      }, # Release_Base

      # Concrete configurations
      'Debug' : {
        'inherit_from': ['Common_Base', 'x86_Base', 'Debug_Base'],
      },

      'Release' : {
        'inherit_from': ['Common_Base', 'x86_Base', 'Release_Base'],
      },
      'Debug_x64' : {
        'inherit_from': ['Common_Base', 'x64_Base', 'Debug_Base'],
      },
      'Release_x64' : {
        'inherit_from': ['Common_Base', 'x64_Base', 'Release_Base'],
      },
    }, # configurations
  }, # target_defaults

  'conditions': [
    ['OS=="win"', {
      'target_defaults': {
        'msvs_settings': {
          'VCLinkerTool': {
            'AdditionalDependencies': [
              'kernel32.lib',
            ], # AdditionalDependencies
            'FixBaseAddress': '1',
            'GenerateDebugInformation': 'true',
            'ImportLibrary': '$(OutDir)\\lib\\$(TargetName).lib',
            'MapFileName': '$(OutDir)\\$(TargetName).map',
            'SubSystem': 1, # /SUBSYSTEM:CONSOLE
          }, # VCLinkerTool
        }, # msvs_settings
      }, # 'target_defaults'
    }], # OS=="win"
  ], # 'conditions',
}
