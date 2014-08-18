# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  "googlecode_url": "http://%s.googlecode.com/svn",
  "chromium_git": "https://chromium.googlesource.com",
  "v8_revision": "23117",
}

deps = {
  "src/third_party/cygwin":
      "/trunk/deps/third_party/cygwin@231940",

  "src/tools/deps2git":
    "/trunk/tools/deps2git@276439",

  "src/testing/gmock":
    Var("chromium_git") + "/external/googlemock.git@896ba0e03f520fb9b6ed582bde2bd00847e3c3f2", # from svn revision 485

  "src/testing/gtest":
    Var("chromium_git") + "/external/googletest.git@4650552ff637bb44ecf7784060091cbed3252211", # from svn revision 692

  "src/tools/gyp":
    (Var("googlecode_url") % "gyp") + "/trunk@1964",

  "src/third_party/hunspell":
    "/trunk/deps/third_party/hunspell@287123",

  "src/third_party/icu":
    "/trunk/deps/third_party/icu52@287122",

  "src/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),
}


hooks = [
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']

  },
]
