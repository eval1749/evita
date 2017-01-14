# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github.git': 'https://github.com',

  'base_revision': 'bdf7c72757f08a7893e482474ff32414b99898a5',
  'build_revision': '15635a431921e51b15dbfcacc6716e87348681bb',
  'buildtools_revision': '0ef801087682b271e9ace93cfa93e9d3dea98079',
  'ced_revision': 'e57cdc44bd541d10669312a6fdc59fc4bf52d2b9',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'gmock_revision': '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566
  'grit_revision': 'a04acb22442f2e0760534441e17fe831817ff116',
  'gtest_revision': '6f8a66431cb592dad629028a50b3dd418a408c87',
  'gyp_revision': 'e7079f0e0e14108ab0dba58728ff219637458563',
  'icu_revision': '9cd2828740572ba6f694b9365236a8356fd06147',
  'idl_parser_revision': 'fce24a3e157ff80b88a514fb469dacf82358f1f0',
  'instrumented_libraries_revision': '45f5814b1543e41ea0be54c771e3840ea52cca4a',
  'jinja2_revision': 'b61a2c009a579593a259c1b300e0ad02bf48fd78',
  'markupsafe_revision': '484a5661041cac13bfc688a26ec5434b05d18961',
  'modp_b64_revision': '28e3fbba4cb4ec3ffd85b53d0a3904525d08f5a6',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': 'e66da27ea326c183115d399432a393dbc8e492cf',
  'v8_revision': '17c1b84066cacf7ea797b04c678748600ef6ec36', # 5.7.451
  'zlib_revision': '8086cc8849617783f1b05be03cb893bb23977186',

  # github
  'autopep8_revision': '9eb1121f357077c7d71fc770e25d3678f906a401',
  'pep8_revision': '4dc42d842274ba27d2724e76eb83ff69e7db226f',
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

  'src/third_party/ced/src':
    Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + Var('ced_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + Var('cygwin_revision'),

  'src/tools/grit':
    Var('chromium_git') + '/chromium/src/tools/grit' + '@' + Var('grit_revision'),

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + Var('gyp_revision'),

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + Var('icu_revision'),

  'src/tools/idl_parser':
    Var('chromium_git') + '/chromium/src/tools/idl_parser' + '@' + Var('idl_parser_revision'),

  'src/third_party/instrumented_libraries':
    Var('chromium_git') + '/chromium/src/third_party/instrumented_libraries.git' + '@' + Var('instrumented_libraries_revision'),

  'src/third_party/jinja2':
    Var('chromium_git') + '/chromium/src/third_party/jinja2.git' + '@' + Var('jinja2_revision'),

  'src/third_party/markupsafe':
    Var('chromium_git') + '/chromium/src/third_party/markupsafe.git' + '@' + Var('markupsafe_revision'),

  'src/third_party/modp_b64':
    Var('chromium_git') + '/chromium/src/third_party/modp_b64.git' + '@' + Var('modp_b64_revision'),

  'src/third_party/ply':
    Var('chromium_git') + '/chromium/src/third_party/ply' + '@' +  Var('ply_revision'),

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),

  'src/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' +  Var('zlib_revision'),

  # From github
  'src/third_party/autopep8':
    Var('github.git') + '/hhatto/autopep8.git' + '@' + Var('autopep8_revision'),

  'src/third_party/pep8':
    Var('github.git') + '/PyCQA/pep8.git' + '@' + Var('pep8_revision'),

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
