: Create *.ninja files under out/
cd src
.\tools\gyp\gyp ^
    -G msvs-version=12 ^
    -f ninja ^
    --generator-output=.. ^
    --include build\common.gypi ^
  build\all.gyp
exit/b
