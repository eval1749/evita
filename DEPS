# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'chromium_git': 'https://chromium.googlesource.com',

  'buildtools_revision':  'f7310ee61725e68b1c8921e381830362cf4f0999',
  'v8_revision': '4890671f8f41d117bc88633096fdd30cdf15bb6e',
}

deps = {
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446b273697fadf3a10ff1007a97c0b7de6df',

  'src/tools/deps2git':
    Var('chromium_git') + '/chromium/tools/deps2git.git' + '@' + 'f04828eb0b5acd3e7ad983c024870f17f17b06d9',

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566

  'src/testing/gtest':
    Var('chromium_git') + '/external/googletest.git' + '@' + '9855a87157778d39b95eccfb201a9dc90f6d61c6', # from svn revision 746

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + '01528c7244837168a1c80f06ff60fa5a9793c824',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + '6b3ce817f8e828c3b7a577d2395f0882eb56ef18',

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),
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
