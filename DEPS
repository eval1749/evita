# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'googlecode_url': 'http://%s.googlecode.com/svn',
  'chromium_git': 'https://chromium.googlesource.com',
  'v8_revision': 'd71b62088ad094b1187c06c92d1f27fab56aaddb', # from svn revision 23496
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
    Var('chromium_git') + '/external/gyp.git' + '@' + '43a3b907eb20bd59c28c6e315b3d9e348ab57ac7', # from svn revision 1970

  'src/third_party/hunspell':
    Var('chromium_git') + '/chromium/deps/hunspell.git' + '@' + 'c956c0e97af00ef789afb2f64d02c9a5a50e6eb1',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu52.git' + '@' + '527ea2dd86afa2751a85d1cc4695f9e2e2d18022', # from svn revision 291706

  'src/v8':
    Var('chromium_git') + '/external/v8.git' + '@' +  Var('v8_revision'),
}


hooks = [
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']

  },
]
