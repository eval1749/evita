// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <string>

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"
#include "gmock/gmock.h"

namespace dom {

class OsDirectoryTest : public AbstractDomTest {
 public:
  ~OsDirectoryTest() override = default;

 protected:
  OsDirectoryTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(OsDirectoryTest);
};

TEST_F(OsDirectoryTest, OpenFailed) {
  mock_io_delegate()->SetOpenDirectoryResult(domapi::IoContextId(), 123);
  EXPECT_SCRIPT_VALID(
      "var reason;"
      "Os.Directory.open('my_dir').catch(x => reason = x);");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsDirectoryTest, OpenSucceeded) {
  mock_io_delegate()->SetOpenDirectoryResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var directory;"
      "Os.Directory.open('my_dir').then(x => directory = x);");
  EXPECT_SCRIPT_TRUE("directory instanceof Os.Directory");
}

TEST_F(OsDirectoryTest, ReadFailed) {
  mock_io_delegate()->SetOpenDirectoryResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var directory;"
      "Os.Directory.open('my_dir').then(x => directory = x);");
  EXPECT_SCRIPT_TRUE("directory instanceof Os.Directory");

  mock_io_delegate()->SetCallResult("ReadDirectory", 123);
  EXPECT_SCRIPT_VALID(
      "var reason;"
      "directory.read(100).catch(x => reason = x);");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsDirectoryTest, ReadSucceeded) {
  mock_io_delegate()->SetOpenDirectoryResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var directory;"
      "Os.Directory.open('my_dir').then(x => directory = x);");
  EXPECT_SCRIPT_TRUE("directory instanceof Os.Directory");

  std::array<domapi::FileStatus, 2> entries;
  entries[0].file_size = 11;
  entries[0].is_directory = false;
  entries[0].is_symlink = false;
  entries[0].last_write_time = base::Time::FromJsTime(123456.0);
  entries[0].name = L"foo";
  entries[0].readonly = false;

  entries[1].file_size = 22;
  entries[1].is_directory = false;
  entries[1].is_symlink = false;
  entries[1].last_write_time = base::Time::FromJsTime(123456.1);
  entries[1].name = L"bar";
  entries[1].readonly = false;

  mock_io_delegate()->SetReadDirectoryResult({entries[0], entries[1]});

  EXPECT_SCRIPT_VALID(
      "var entries;"
      "directory.read(100).then(x => entries = x);");
  EXPECT_SCRIPT_EQ("2", "entries.length");
  EXPECT_SCRIPT_EQ("11", "entries[0].size");
  EXPECT_SCRIPT_EQ("22", "entries[1].size");
}

}  // namespace dom
