# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github.git': 'https://github.com',

  'autopep8_revision': '9eb1121f357077c7d71fc770e25d3678f906a401',
  'base_revision': '2efc4ae606a38db4b7c02efb6abe4f9af00902b7',
  'build_revision': '5e4ffb5c8928fe5afacd1b1b0f2bb732cdc0d77c',
  'buildtools_revision': '5fd66957f08bb752dca714a591c84587c9d70762',
  'ced_revision': '9012c0ab648025dd0f8df14294bf5d6d73793ac9',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'gmock_revision': '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566
  'grit_revision': 'b3fb946d9e758d7e5789c3dc8df4eb54337f7312',
  'gtest_revision': '6f8a66431cb592dad629028a50b3dd418a408c87',
  'gyp_revision': '702ac58e477214c635d9b541932e75a95d349352',
  'icu_revision': 'b0bd3ee50bc2e768d7a17cbc60d87f517f024dbe',
  'idl_parser_revision': 'fce24a3e157ff80b88a514fb469dacf82358f1f0',
  'pep8_revision': '4dc42d842274ba27d2724e76eb83ff69e7db226f',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': '1310674f3e89bce7647aab8d7523039e2810bbcf',
  'v8_revision': '58bce67f5fd1615ba493fb0f95ace2a99c7ef649', # 5.5.313
  'zlib_revision': '40925c7211b26f85071c98a16c3d8a43d70c7c67',
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
