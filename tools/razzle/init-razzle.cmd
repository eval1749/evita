: This script should run after git clone.
if "%evita_repo_url%"=="" set evita_repo_url=https://%creds.code.google.com%code.google.com/p/evita


: To make |git merge-base HEAD origin| works.
git remote set-head -a origin

: Download modules
cmd /c git submodule add https://chromium.googlesource.com/chromium/tools/deps2git.git  tools/deps2git
cmd /c git submodule add https://chromium.googlesource.com/external/gyp tools/gyp
cmd /c git submodule add https://chromium.googlesource.com/chromium/deps/icu46 third_party/icu
cmd /c git submodule add https://chromium.googlesource.com/external/v8 third_party/v8
