
: This script should run after git clone.
if "%evita_repo_url%"=="" set evita_repo_url=https://%creds.code.google.com%code.google.com/p/evita

setlocal
: To make |git merge-base HEAD origin| works.
git merge-base HEAD origin
if not %ERRORLEVEL% == 0 (
  git remote set-head -a origin
)

: Download modules
set git_url=https://chromium.googlesource.com

cmd /c git submodule add %git_url%/chromium/tools/deps2git.git  tools/deps2git
cmd /c git submodule add %git_url%/chromium/testing/gtest  testing/gtest
cmd /c git submodule add %git_url%/external/gyp tools/gyp
cmd /c git submodule add %git_url%/chromium/deps/icu46 third_party/icu
cmd /c git submodule add %git_url%/external/v8 third_party/v8

endlocal
