// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"

namespace {

class OsFileTest : public dom::AbstractDomTest {
  protected: OsFileTest() {
  }
  public: virtual ~OsFileTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(OsFileTest);
};

TEST_F(OsFileTest, OsFile_open_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(nullptr, 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "Os.File.open('foo.js').catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_open_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      reinterpret_cast<domapi::IoHandle*>(123), 0);
  EXPECT_SCRIPT_VALID(
    "var file;"
    "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");
}

TEST_F(OsFileTest, OsFile_stat_) {
  domapi::QueryFileStatusCallbackData data;
  data.error_code = 123;
  data.file_size = 456;
  data.is_directory = true;
  data.is_symlink = true;
  data.last_write_time = base::Time::FromJsTime(123456.0);
  data.readonly = false;
  mock_io_delegate()->SetQueryFileStatusCallbackData(data);
  EXPECT_SCRIPT_VALID(
    "var result;"
    "function callback(data) {"
    "   result = data;"
    "}"
    "Os.File.stat_('foo', callback);");
  EXPECT_SCRIPT_EQ("123", "result.errorCode");
  EXPECT_SCRIPT_EQ("true", "result.isDir");
  EXPECT_SCRIPT_EQ("true", "result.isSymLink");
  EXPECT_SCRIPT_EQ("123456", "result.lastModificationDate.valueOf()");
  EXPECT_SCRIPT_EQ("false", "result.readonly");
}

TEST_F(OsFileTest, OsFile_read_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(
      reinterpret_cast<domapi::IoHandle*>(123), 0);
  EXPECT_SCRIPT_VALID(
    "var file;"
    "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetFileIoCallbackData(0, 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "file.read(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_read_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      reinterpret_cast<domapi::IoHandle*>(123), 0);
  EXPECT_SCRIPT_VALID(
    "var file;"
    "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetFileIoCallbackData(123, 0);
  EXPECT_SCRIPT_VALID(
    "var transferred;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "file.read(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("123", "transferred");
}

TEST_F(OsFileTest, OsFile_write_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(
      reinterpret_cast<domapi::IoHandle*>(123), 0);
  EXPECT_SCRIPT_VALID(
    "var file;"
    "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetFileIoCallbackData(0, 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "file.write(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_write_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      reinterpret_cast<domapi::IoHandle*>(123), 0);
  EXPECT_SCRIPT_VALID(
    "var file;"
    "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetFileIoCallbackData(123, 0);
  EXPECT_SCRIPT_VALID(
    "var transferred;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "file.write(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("123", "transferred");
}

}  // namespace
