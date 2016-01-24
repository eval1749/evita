// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/mock_io_delegate.h"
#include "gmock/gmock.h"

namespace dom {

class OsFileTest : public AbstractDomTest {
 public:
  ~OsFileTest() override = default;

 protected:
  OsFileTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(OsFileTest);
};

TEST_F(OsFileTest, OsFile_makeTempFileName_failed) {
  mock_io_delegate()->SetMakeTempFileName(L"", 123);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.makeTempFileName('foo', 'bar').catch(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("result instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "result.winLastError");
}

TEST_F(OsFileTest, OsFile_makeTempFileName_succeeded) {
  mock_io_delegate()->SetMakeTempFileName(L"xyz", 0);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.makeTempFileName('foo', 'bar').then(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_EQ("string", "typeof(result)");
  EXPECT_SCRIPT_EQ("foo", "FilePath.dirname(result)");
  EXPECT_SCRIPT_EQ("barxyz", "FilePath.basename(result)");
}

TEST_F(OsFileTest, OsFile_move_failed) {
  mock_io_delegate()->SetCallResult("MoveFile", 123);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.move('foo', 'bar').catch(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("result instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "result.winLastError");
}

TEST_F(OsFileTest, OsFile_move_succeeded) {
  mock_io_delegate()->SetCallResult("MoveFile", 0);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.move('foo', 'bar').then(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_EQ("boolean", "typeof(result)");
  EXPECT_SCRIPT_EQ("false", "result");

  mock_io_delegate()->SetCallResult("MoveFile", 0);
  EXPECT_SCRIPT_VALID(
      "promise = Os.File.move('foo', 'bar', {noOverwrite: "
      "true}).then(catcher);");
  EXPECT_SCRIPT_EQ("true", "result");

  EXPECT_SCRIPT_EQ(
      "TypeError: Failed to execute 'move' on 'File': Expect arguments[2] as "
      "'MoveFileOptions' but '[object Object]'",
      "promise = Os.File.move('foo', 'bar', {baz: true}).then(catcher);")
      << "Bad dictionary member";
}

TEST_F(OsFileTest, OsFile_open_failed) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId(), 123);
  EXPECT_SCRIPT_VALID(
      "var reason;"
      "Os.File.open('foo.js').catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_open_succeeded) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(

      "var file;"
      "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");
}

TEST_F(OsFileTest, OsFile_stat_failed) {
  domapi::FileStatus data;
  mock_io_delegate()->SetFileStatus(data, 123);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "Os.File.stat('foo').catch(function(x) { result = x; });");
  EXPECT_SCRIPT_TRUE("result instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "result.winLastError");
}

TEST_F(OsFileTest, OsFile_stat_succeeded) {
  domapi::FileStatus data;
  data.is_directory = true;
  data.is_symlink = true;
  data.last_write_time = base::Time::FromJsTime(123456.0);
  data.readonly = false;
  mock_io_delegate()->SetFileStatus(data, 0);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "Os.File.stat('foo').then(function(x) { result = x; });");
  EXPECT_SCRIPT_TRUE("result instanceof Os.File.Info");
  EXPECT_SCRIPT_EQ("true", "result.isDir");
  EXPECT_SCRIPT_EQ("true", "result.isSymLink");
  EXPECT_SCRIPT_EQ("123456", "result.lastModificationDate.valueOf()");
  EXPECT_SCRIPT_EQ("false", "result.readonly");
}

TEST_F(OsFileTest, OsFile_read_failed) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var file;"
      "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetCallResult("ReadFile", 123);
  EXPECT_SCRIPT_VALID(
      "var reason;"
      "var arrayBuffer = new ArrayBuffer(10);"
      "var arrayView = new Uint8Array(arrayBuffer);"
      "file.read(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_read_succeeded) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var file;"
      "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetCallResult("ReadFile", 0, 123);
  EXPECT_SCRIPT_VALID(
      "var transferred;"
      "var arrayBuffer = new ArrayBuffer(10);"
      "var arrayView = new Uint8Array(arrayBuffer);"
      "file.read(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("123", "transferred");
}

TEST_F(OsFileTest, OsFile_remove_failed) {
  mock_io_delegate()->SetCallResult("RemoveFile", 123);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.remove('foo').catch(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("result instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "result.winLastError");
}

TEST_F(OsFileTest, OsFile_remove_succeeded) {
  mock_io_delegate()->SetCallResult("RemoveFile", 0);
  EXPECT_SCRIPT_VALID(
      "var result;"
      "function catcher(x) { result = x; }"
      "var promise = Os.File.remove('foo').then(catcher);");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_EQ("boolean", "typeof(result)");
  EXPECT_SCRIPT_EQ("true", "result");
}

TEST_F(OsFileTest, OsFile_write_failed) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var file;"
      "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  mock_io_delegate()->SetCallResult("WriteFile", 123);
  EXPECT_SCRIPT_VALID(
      "var reason;"
      "var arrayBuffer = new ArrayBuffer(10);"
      "var arrayView = new Uint8Array(arrayBuffer);"
      "file.write(arrayView).catch(function(x) { reason = x; });");
  EXPECT_SCRIPT_TRUE("reason instanceof Os.File.Error");
  EXPECT_SCRIPT_EQ("123", "reason.winLastError");
}

TEST_F(OsFileTest, OsFile_write_succeeded) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  EXPECT_SCRIPT_VALID(
      "var file;"
      "Os.File.open('foo.js').then(function(x) { file = x });");
  EXPECT_SCRIPT_TRUE("file instanceof Os.File");

  std::vector<uint8_t> expected_bytes{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  mock_io_delegate()->SetCallResult("WriteFile", 0,
                                    static_cast<int>(expected_bytes.size()));
  EXPECT_SCRIPT_VALID(
      "var transferred;"
      "var arrayBuffer = new ArrayBuffer(10);"
      "var arrayView = new Uint8Array(arrayBuffer);"
      "for (var i = 0; i < arrayView.length; ++i) arrayView[i] = i;"
      "file.write(arrayView).then(function(x) { transferred = x; });");
  EXPECT_SCRIPT_EQ("10", "transferred");
  EXPECT_EQ(expected_bytes, mock_io_delegate()->bytes());
}

}  // namespace dom
