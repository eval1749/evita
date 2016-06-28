# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github.git': 'https://github.com',

  'autopep8_revision': '9eb1121f357077c7d71fc770e25d3678f906a401',
  'base_revision': '8e7c163b4f6283a278aaf4dc4617625aa4fd6af7',
  'build_revision': '95bb77009ecdeaa992b031b947142fbf7da57ed6',
  'buildtools_revision': '56eaae134648135663c4aa1ed82278572b5f35ef',
  'cygwin_revision': '4fbd5b9e6664b991c06dd3a157c8f8e67fcbff48',
  'gmock_revision': '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566
  'grit_revision': '3a6166d8a1226a793d9978ea3d44925eb59ed43e',
  'gtest_revision': '6f8a66431cb592dad629028a50b3dd418a408c87',
  'gyp_revision': '35eafcd939515d51d19556c543f9cf97faf75ee6',
  'icu_revision': 'ffa4b6704cf5cc9fec9485731f24a05b2ba94bca',
  'idl_parser_revision': 'fce24a3e157ff80b88a514fb469dacf82358f1f0',
  'pep8_revision': '4dc42d842274ba27d2724e76eb83ff69e7db226f',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': '1bde8f72bcdf5710333550f977667c22817714cd',
  'v8_revision': 'bbd1605bb0829a0a4b46f93c4587f33c24cc1174', # 5.3.330
  'zlib_revision': '6c1421668fbeb42b16796b6bcd41639cea2edc1c',
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
