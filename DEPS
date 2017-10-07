# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github_git': 'https://github.com',

  'base_revision': '234faf4fdde9599ad4a436b252592f2662af379d',
  'build_revision': '58ec82342fa6dc7fc4a08020913559b3ffa2918f',
  'buildtools_revision': 'f6d165d9d842ddd29056c127a5f3a3c5d8e0d2e3',
  'ced_revision': '910cca22d881b02cbc8950fa02ccbcdcfb782456',
  'googletest_revision': 'ca102b1f9d1f4c8a8c6f7a87b3e80d0af4b8789f',
  'grit_revision': '10da9fa0d05589fa8f4f941a847e55df5e63aa72',
  'gyp_revision': 'd61a9397e668fa9843c4aa7da9e79460fe590bfb',
  'icu_revision': '08cb956852a5ccdba7f9c941728bb833529ba3c6',
  'idl_parser_revision': '4e235f57fc7d38ba3cd450c81f0e8e0e7fa4e164',
  'instrumented_libraries_revision': '644afd349826cb68204226a16c38bde13abe9c3c',
  'jinja2_revision': 'd34383206fa42d52faa10bb9931d6d538f3a57e0',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'modp_b64_revision': '28e3fbba4cb4ec3ffd85b53d0a3904525d08f5a6',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': 'c63631a8d5c94930c4ed192154f9d4375a2547a8',
  'tools_clang_revision': '66be66dcb7169e736d56d6fc2b53a7d6dac87ac4',
  'tools_win_revision': '8b3e4fdd40af25b4adb1cbe13bca32f59971c127',
  'v8_revision': '4f88c0ee8f69161db1534dbc3a5e566275639abe', # 6.3.273
  'zlib_revision': 'e7afdfe128e01ca480a28f757b571957befdd962',

  # github
  'autopep8_revision': '93fff80de7dd0819b36281bc0868e5b17c81fb7f', # 1.3.2
  'pep8_revision': '8053c7c1d5597b062c58c5991231b62e11b435e9', # 2.2.0
}

deps = {
  # From chromium_git
  'src/base':
    Var('chromium_git') + '/chromium/src/base' + '@' +  Var('base_revision'),

  'src/build':
    Var('chromium_git') + '/chromium/src/build' + '@' +  Var('build_revision'),

  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/testing':
    Var('chromium_git') + '/chromium/src/testing' + '@' + Var('testing_revision'),

  'src/third_party/ced/src':
    Var('chromium_git') + '/external/github.com/google/compact_enc_det.git' + '@' + Var('ced_revision'),

 'src/third_party/googletest/src':
    Var('chromium_git') + '/external/github.com/google/googletest.git' + '@' + Var('googletest_revision'),

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

  'src/tools/clang':
    Var('chromium_git') + '/chromium/src/tools/clang' + '@' +  Var('tools_clang_revision'),

  'src/tools/win':
    Var('chromium_git') + '/chromium/src/tools/win' + '@' +  Var('tools_win_revision'),

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),

  'src/third_party/zlib':
    Var('chromium_git') + '/chromium/src/third_party/zlib' + '@' +  Var('zlib_revision'),

  # From github
  'src/third_party/autopep8':
    Var('github_git') + '/hhatto/autopep8.git' + '@' + Var('autopep8_revision'),

  'src/third_party/pep8':
    Var('github_git') + '/PyCQA/pep8.git' + '@' + Var('pep8_revision'),
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
    # Pull clang if needed or requested via GYP_DEFINES.
    # Note: On Win, this should run after win_toolchain, as it may use it.
    'name': 'clang',
    'pattern': '.',
    'action': ['python', 'src/tools/clang/scripts/update.py', '--if-needed'],
  },
  {
    'pattern': '.',
    'action': ['src\\evita\\build\\gn_evita.cmd']
  },
]

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'src/buildtools',
]
