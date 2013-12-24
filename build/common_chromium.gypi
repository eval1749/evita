# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  # Variables for loading base/base.gyp
  'variables': {
    'android_webview_build': 0,
    'chromeos': 0,
    'chromium_code%': 0,
    'desktop_linux': 0,
    'google_tv': 0,
    'gtest_target_type': 'executable',
    'nacl_win64_defines': [],
    'order_profiling': 0,
    'os_bsd': 0,
    'test_isolation_mode': 'noop',
    'toolkit_uses_gtk': 0,
    'use_aura': 0,
    'use_glib': 0,
    'use_system_nspr': 0,
    'use_ozone': 0,
    'use_x11': 0,
    'win_use_allocator_shim': 0,
  }, # variables

  'target_defaults': {
    'variables': { 'nacl_untrusted_build%': 0, },
    'target_conditions': [
      ['chromium_code==1', {
        'includes': [ 'filename_rules.gypi' ],
        'variables': {
          'toolkit_views': 0,
          'use_ash': 0,
          'use_pango': 0,
          'use_ozone_evdev': 0,
          'ozone_platform_dri': 0,
         }, # variables
      }], # chromium_code==1
      ['chromium_code==1 and OS=="win"', {
        'defines': [
          '_ATL_NO_OPENGL',
          '_CRT_RAND_S',
          '_WIN32_WINNT=0x0602',
          '_WINDOWS',
          'CERT_CHAIN_PARA_HAS_EXTRA_FIELDS',
          'NOMINMAX',
          'PSAPI_VERSION=1',
          'WIN32',
          'WIN32_LEAN_AND_MEAN',
          'WINVER=0x0602',

          'ICU_UTIL_DATA_IMPL=ICU_UTIL_DATA_SHARED'

          '_CRT_SECURE_NO_DEPRECATE',
          '_SCL_SECURE_NO_DEPRECATE',
          # This define is required to pull in the new Win8 interfaces from
          # system headers like ShObjIdl.h.
          'NTDDI_VERSION=0x06020000',
          # This is required for ATL to use XP-safe versions of its functions.
          '_USING_V110_SDK71_',
        ], # defines
        'msvs_settings': {
          'VCCLCompilerTool': {
            'AdditionalOptions': ['/MP'],
            'MinimalRebuild': 'false',
            'BufferSecurityCheck': 'true',
            'EnableFunctionLevelLinking': 'true', # /Gy
            'RuntimeTypeInfo': 'false', # /GR-
            'WarningLevel': '4', # /Wall
            'WarnAsError': 'true', # /WX
            'DebugInformationFormat': '3',
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
              'advapi32.lib',
              'dbghelp.lib',
              'dnsapi.lib',
              'msimg32.lib',
              'ole32.lib',
              'oleaut32.lib',
              'psapi.lib',
              'shell32.lib',
              'shlwapi.lib',
              'user32.lib',
              'usp10.lib',
              'version.lib',
              'wininet.lib',
              'winmm.lib',
              'ws2_32.lib',
             ],
           }, # VCLinkerTool
        }, # msvs_settings
        'msvs_disabled_warnings': [
          4351, 4355, 4396, 4503, 4819,
          # These warnings are level 4.
          4100, 4121, 4125, 4127, 4130, 4131, 4189, 4201, 4238, 4244, 4245,
          4310, 4428, 4481, 4505, 4510, 4512, 4530, 4610, 4611, 4701, 4702,
          4706,
          # C4996: 'function': was declared deprecated
          4996, # GetVersionEx
          # C4251: 'identifier' : class 'type' needs to have dll-interface to
          # be used by clients of class 'type2'
          4251,
         ],
    }], # OS=="win"
    ], # target_conditions
  }, # target_defaults
}
