@if "%echo_%"=="" echo off
rem Copyright (c) 2013 Project Vogue. All rights reserved.
rem Use of this source code is governed by a BSD-style license that can be
rem found in the LICENSE file.
rem This script run after "git clone %repo_url% src".

if "%evita_repo_url%"=="" set repo_url=https://%creds.code.google.com%code.google.com/p/evita

if exist .git goto should_be_root_directory
if not exist src goto should_be_root_directory

setlocal
echo Creating .gclient
cmd /c gclient config --verbose --name=src --git-deps --unmanaged git+%repo_url%
cd src
echo Downloading...
cmd /c gclient sync
echo Ready. You can build with "ninja -C ../out/Debug".
endlocal

exit/b 0

:should_be_root_directory
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  echo !!
  echo !! You must run this script at root directory rather than
  echo !! git directory.
  echo !!
  echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  exit/b 1
