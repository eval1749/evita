// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"

namespace {

class OsProcessTest : public dom::AbstractDomTest {
  protected: OsProcessTest() {
  }
  public: virtual ~OsProcessTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(OsProcessTest);
};

TEST_F(OsProcessTest, OsProcess_open_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(domapi::IoContextId(), 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "Os.Process.open('foo.js').catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsProcessTest, OsProcess_open_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
    "var process;"
    "Os.Process.open('foo.js').then(function(x) { process = x });");
  EXPECT_SCRIPT_TRUE("process instanceof Os.Process");
}

TEST_F(OsProcessTest, OsProcess_read_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(
      domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
    "var process;"
    "Os.Process.open('foo.js').then(function(x) { process = x });");
  EXPECT_SCRIPT_TRUE("process instanceof Os.Process");

  mock_io_delegate()->SetFileIoCallbackData(0, 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "process.read(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsProcessTest, OsProcess_read_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
    "var process;"
    "Os.Process.open('foo.js').then(function(x) { process = x });");
  EXPECT_SCRIPT_TRUE("process instanceof Os.Process");

  mock_io_delegate()->SetFileIoCallbackData(123, 0);
  EXPECT_SCRIPT_VALID(
    "var transferred;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "process.read(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("123", "transferred");
}

TEST_F(OsProcessTest, OsProcess_write_failed) {
  mock_io_delegate()->SetOpenFileCallbackData(
      domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
    "var process;"
    "Os.Process.open('foo.js').then(function(x) { process = x });");
  EXPECT_SCRIPT_TRUE("process instanceof Os.Process");

  mock_io_delegate()->SetFileIoCallbackData(0, 123);
  EXPECT_SCRIPT_VALID(
    "var reason;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "process.write(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsProcessTest, OsProcess_write_succeeded) {
  mock_io_delegate()->SetOpenFileCallbackData(
      domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
    "var process;"
    "Os.Process.open('foo.js').then(function(x) { process = x });");
  EXPECT_SCRIPT_TRUE("process instanceof Os.Process");

  mock_io_delegate()->SetFileIoCallbackData(123, 0);
  EXPECT_SCRIPT_VALID(
    "var transferred;"
    "var arrayBuffer = new ArrayBuffer(10);"
    "var arrayView = new Uint8Array(arrayBuffer);"
    "process.write(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("123", "transferred");
}

}  // namespace
