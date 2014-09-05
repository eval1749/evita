# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'googlecode_url': 'http://%s.googlecode.com/svn',
  'chromium_git': 'https://chromium.googlesource.com',
  'v8_revision':  '1af4d9551ad496a28c342004b1a4e2a3840228f7', # from svn revision 23706
}

deps = {
  'src/third_party/cygwin':
      Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446b273697fadf3a10ff1007a97c0b7de6df',

  'src/tools/deps2git':
    Var('chromium_git') + '/chromium/tools/deps2git.git' + '@' + 'f04828eb0b5acd3e7ad983c024870f17f17b06d9',

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git@896ba0e03f520fb9b6ed582bde2bd00847e3c3f2', # from svn revision 485

  'src/testing/gtest':
    Var('chromium_git') + '/external/googletest.git@4650552ff637bb44ecf7784060091cbed3252211', # from svn revision 692

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + '1ff523f4eccd38fb9f73e60dddd895736ce90346', # from svn revision 1973

  'src/third_party/hunspell':
    Var('chromium_git') + '/chromium/deps/hunspell.git' + '@' + 'c956c0e97af00ef789afb2f64d02c9a5a50e6eb1',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu52.git' + '@' + '89831135d5f76413fcdd3852f8de7f344b3fa40c', # from svn revision 291781

  'src/v8':
    Var('chromium_git') + '/external/v8.git' + '@' +  Var('v8_revision'),
}


hooks = [
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']

  },
]
