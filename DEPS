#
vars = {
  "googlecode_url": "http://%s.googlecode.com/svn",
  "chromium_git": "https://chromium.googlesource.com",
  "v8_revision": "18251",
}

deps = {
  "src/third_party/icu":
    "/trunk/deps/third_party/icu46@239289",

  "src/third_party/v8":
    (Var("googlecode_url") % "v8") + "/trunk@" + Var("v8_revision"),
}
