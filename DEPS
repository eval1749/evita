# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  "googlecode_url": "http://%s.googlecode.com/svn",
  "chromium_git": "https://chromium.googlesource.com",
  "v8_revision": "21393",
}

deps = {
  "src/third_party/cygwin":
      "/trunk/deps/third_party/cygwin@231940",

  "src/tools/deps2git":
    "/trunk/tools/deps2git@270777",

  "src/testing/gmock":
    (Var("googlecode_url") % "googlemock") + "/trunk@410",

  "src/testing/gtest":
    (Var("googlecode_url") % "googletest") + "/trunk@643",

  "src/tools/gyp":
    (Var("googlecode_url") % "gyp") + "/trunk@1921",

  "src/third_party/hunspell":
    "/trunk/deps/third_party/hunspell@256272",

  "src/third_party/icu":
    "/trunk/deps/third_party/icu46@262949",

  "src/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),
}


hooks = [
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']

  },
]
