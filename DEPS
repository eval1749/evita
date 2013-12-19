#
vars = {
  "googlecode_url": "http://%s.googlecode.com/svn",
  "chromium_git": "https://chromium.googlesource.com",
  "v8_revision": "18251",
}

deps = {
  "src/third_party/cygwin":
      "/trunk/deps/third_party/cygwin@231940",

  "src/tools/deps2git":
    "/trunk/tools/deps2git@236839",

  "src/testing/gtest":
    (Var("googlecode_url") % "googletest") + "/trunk@643",

  "src/tools/gyp":
    (Var("googlecode_url") % "gyp") + "/trunk@1806",

  "src/third_party/icu":
    "/trunk/deps/third_party/icu46@239289",

  "src/third_party/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),
}


hooks = [
  {
    'pattern': '.',
    'action': ['cmd.exe', '/C', 'src\\build\\setup_ninja.cmd']
  },
]
