# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'chromium_git': 'https://chromium.googlesource.com',
  'v8_revision': 'bc4c36e63a8f0cee3398ec0b72da5bdfafba3954',
}

deps = {
  'src/third_party/cygwin':
      Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446b273697fadf3a10ff1007a97c0b7de6df',

  'src/tools/deps2git':
    Var('chromium_git') + '/chromium/tools/deps2git.git' + '@' + 'f04828eb0b5acd3e7ad983c024870f17f17b06d9',

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + '29763965ab52f24565299976b936d1265cb6a271', # from svn revision 501

  'src/testing/gtest':
    Var('chromium_git') + '/external/googletest.git' + '@' + 'be1868139ffe0ccd0e8e3b37292b84c821d9c8ad', # from svn revision 704

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + 'd174d75bf69c682cb62af9187879e01513b35e52',

  'src/third_party/hunspell':
    Var('chromium_git') + '/chromium/deps/hunspell.git' + '@' + 'c956c0e97af00ef789afb2f64d02c9a5a50e6eb1',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'd319ad9e25e8045f051dcd30bd5835f99936af6c',

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),
}


hooks = [
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']

  },
]
