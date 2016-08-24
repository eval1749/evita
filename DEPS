# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github.git': 'https://github.com',

  'autopep8_revision': '9eb1121f357077c7d71fc770e25d3678f906a401',
  'base_revision': '3452bd76bb82cfe4de6efba4b368f53a7c198441',
  'build_revision': '59daf502c36f20b5c9292f4bd9af85791f8a5884',
  'buildtools_revision': 'adb8bf4e8fc92aa1717bf151b862d58e6f27c4f2',
  'ced_revision': '9012c0ab648025dd0f8df14294bf5d6d73793ac9',
  'cygwin_revision': '4fbd5b9e6664b991c06dd3a157c8f8e67fcbff48',
  'gmock_revision': '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566
  'grit_revision': '685fc61f33bd78fd86e30e4140fabb18b3ba46d9',
  'gtest_revision': '6f8a66431cb592dad629028a50b3dd418a408c87',
  'gyp_revision': '702ac58e477214c635d9b541932e75a95d349352',
  'icu_revision': '53ce631655a61aaaa42b43b4d64abe23e9b8d71f',
  'idl_parser_revision': 'fce24a3e157ff80b88a514fb469dacf82358f1f0',
  'pep8_revision': '4dc42d842274ba27d2724e76eb83ff69e7db226f',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': 'c47c1e8f7e874a5548ef7287ca3c1975586d0fa3',
  'v8_revision': '4fb8ad0d295ef2ad849c12609a3344d5fe8a8dbd', # 5.4.500
  'zlib_revision': '830b5c25b5fbe37e032ea09dd011d57042dd94df',
}

deps = {
  'src/base':
    Var('chromium_git') + '/chromium/src/base' + '@' +  Var('base_revision'),

  'src/build':
    Var('chromium_git') + '/chromium/src/build' + '@' +  Var('build_revision'),

  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + Var('testing_revision'),

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + Var('gmock_revision'),

  'src/testing/gtest':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + Var('gtest_revision'),

  'src/third_party/autopep8':
    Var('github.git') + '/hhatto/autopep8.git' + '@' + Var('autopep8_revision'),

  'src/third_party/ced/src':
    Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + Var('ced_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + Var('cygwin_revision'),

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + Var('icu_revision'),

  'src/third_party/pep8':
    Var('github.git') + '/PyCQA/pep8.git' + '@' + Var('pep8_revision'),

  'src/third_party/ply':
    Var('chromium_git') + '/chromium/src/third_party/ply' + '@' +  Var('ply_revision'),

  'src/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' +  Var('zlib_revision'),

  'src/tools/grit':
    Var('chromium_git') + '/chromium/src/tools/grit' + '@' + Var('grit_revision'),

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + Var('gyp_revision'),

  'src/tools/idl_parser':
    Var('chromium_git') + '/chromium/src/tools/idl_parser' + '@' + Var('idl_parser_revision'),

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),
}

hooks = [
  {
    # Update LASTCHANGE.
    'name': 'lastchange',
    'pattern': '.',
    'action': ['python', 'src/build/util/lastchange.py',
               '-o', 'src/build/util/LASTCHANGE'],
  },

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
    'action': ['src\\evita\\build\\gn_evita.cmd']
  },
]
