# This file is used to manage the dependencies of the Evita src repo. It is
# used by gclient to determine what version of each dependency to check out, and
# where.

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'github.git': 'https://github.com',

  'base_revision': '33c1d10301f0154b122585e58246d7457517b767',
  'build_revision': 'a4ae1b793d57e070e6417fe9d3048dca2ab4f46c',
  'buildtools_revision': '38477c2e103caf651d113dad07cd3e4efb992209',
  'ced_revision': '910cca22d881b02cbc8950fa02ccbcdcfb782456',
  'cygwin_revision': 'c89e446b273697fadf3a10ff1007a97c0b7de6df',
  'googletest_revision': '4bab34d2084259cba67f3bfb51217c10d606e175',
  'grit_revision': '348da58de1eecd2b848493b6aef8ff15fe75ebba',
  'gyp_revision': 'eb296f67da078ec01f5e3a9ea9cdc6d26d680161',
  'icu_revision': 'dfa798fe694702b43a3debc3290761f22b1acaf8',
  'idl_parser_revision': 'faf878566a8fcee3a8522b9e732ffe70c15752f5',
  'instrumented_libraries_revision': '644afd349826cb68204226a16c38bde13abe9c3c',
  'jinja2_revision': 'd34383206fa42d52faa10bb9931d6d538f3a57e0',
  'markupsafe_revision': '8f45f5cfa0009d2a70589bcda0349b8cb2b72783',
  'modp_b64_revision': '28e3fbba4cb4ec3ffd85b53d0a3904525d08f5a6',
  'ply_revision': '4a6baf95860033d4c69d3e3087696b30c687622c',
  'testing_revision': '48bc5696ecfa2084c5ef511950c4f68589a1de38',
  'tools_win_revision': 'adcc32ce1cf28843e97eeafd539a33e34ce4bff6',
  'v8_revision': 'f9b1cd343df071f543f07539513692367af25b06', # 6.1.266
  'zlib_revision': '1782c7b1c6934f6970c4517fddc92537812cfd4f',

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

  'src/tools/win':
    Var('chromium_git') + '/chromium/src/tools/win' + '@' +  Var('tools_win_revision'),

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

recursedeps = [
  # buildtools provides clang_format, libc++, and libc++abi
  'src/buildtools',
]
