# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',

  'buildtools_revision':  '68e3c238a5ab347436762cb929316aa55ca72563',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'gmock_revision': '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566
  'gtest_revision': '6f8a66431cb592dad629028a50b3dd418a408c87',
  'gyp_revision': '70ee80e82bacf2d7816a56f792bb33587b04c338',
  'icu_revision': '42c58d4e49f2250039f0e98d43e0b76e8f5ca024',
  'v8_revision': '65013c1d17092a8cad8cd9518d3e877ae96927ff', # 4.9.201
}

deps = {
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + Var('cygwin_revision'),

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + Var('gmock_revision'), 

  'src/testing/gtest':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + Var('gtest_revision'),

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + Var('gyp_revision'),

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + Var('icu_revision'),
}

hooks = [
  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']
  },

  {
    'pattern': '.',
    'action': ['src\\evita\\build\\gn_evita.cmd']
  },
]
