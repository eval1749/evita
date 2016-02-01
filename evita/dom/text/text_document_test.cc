// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/dom/file_path.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_io_delegate.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "evita/dom/text/text_document.h"
#include "evita/text/buffer.h"
#include "gmock/gmock.h"

namespace dom {

using ::testing::Eq;
using ::testing::_;

class TextDocumentTest : public AbstractDomTest {
 protected:
  TextDocumentTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextDocumentTest);
};

TEST_F(TextDocumentTest, Constructor) {
  // TODO(yosi): We should remove all buffers for each test case.
  EXPECT_SCRIPT_EQ("bar",
                   "var sample1 = TextDocument.new('bar');"
                   "sample1.name");
  EXPECT_SCRIPT_EQ("bar (2)",
                   "var sample2 = TextDocument.new('bar'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3)",
                   "var sample2 = TextDocument.new('bar'); sample2.name");

  EXPECT_SCRIPT_VALID("TextDocument.new('bar.cc')");
  EXPECT_SCRIPT_EQ("bar (2).cc",
                   "var sample2 = TextDocument.new('bar.cc'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3).cc",
                   "var sample2 = TextDocument.new('bar.cc'); sample2.name");

  EXPECT_SCRIPT_VALID("TextDocument.new('.bar')");
  EXPECT_SCRIPT_EQ(".bar (2)",
                   "var sample2 = TextDocument.new('.bar'); sample2.name");
  EXPECT_SCRIPT_EQ(".bar (3)",
                   "var sample2 = TextDocument.new('.bar'); sample2.name");
}

TEST_F(TextDocumentTest, TextDocument_addObserver) {
  EXPECT_SCRIPT_VALID(
      "var result_doc, result_type;"
      "function callback(type, doc) {"
      "  result_doc = doc;"
      "  result_type = type;"
      "}"
      "TextDocument.addObserver(callback);"
      "var doc1 = TextDocument.new('addObserver');");

  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("add", "result_type")
      << "The observer gets 'add' notification.";

  EXPECT_SCRIPT_VALID("TextDocument.remove(doc1)");

  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("remove", "result_type")
      << "The observer gets 'remove' notification.";

  EXPECT_SCRIPT_VALID("TextDocument.removeObserver(callback)");
  EXPECT_SCRIPT_VALID("var doc2 = TextDocument.new('addObserver2')");

  RunMessageLoopUntilIdle();
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("remove", "result_type")
      << "The observer doesn't get notification.";
}

TEST_F(TextDocumentTest, TextDocument_list) {
  EXPECT_SCRIPT_VALID(
      "['foo', 'bar', 'baz'].forEach(function(name) {"
      "  TextDocument.new(name);"
      "});"
      "var samples = TextDocument.list.sort(function(a, b) {"
      "  return a.name.localeCompare(b.name);"
      "});");
  EXPECT_SCRIPT_EQ("3", "samples.length");
  EXPECT_SCRIPT_EQ("bar", "samples[0].name");
  EXPECT_SCRIPT_EQ("baz", "samples[1].name");
  EXPECT_SCRIPT_EQ("foo", "samples[2].name");
}

TEST_F(TextDocumentTest, TextDocument_open) {
  EXPECT_SCRIPT_VALID("var a = TextDocument.open('foo');");
  auto const absoulte_file_name = FilePath::FullPath(L"foo");
  EXPECT_SCRIPT_VALID(
      "var b = TextDocument.open('foo');"
      "var c = TextDocument.open('bar');");
  EXPECT_SCRIPT_EQ(base::UTF16ToUTF8(absoulte_file_name), "a.fileName");
  EXPECT_SCRIPT_TRUE("a === b");
  EXPECT_SCRIPT_TRUE("a !== c");
}

TEST_F(TextDocumentTest, TextDocumentFind) {
  EXPECT_SCRIPT_TRUE("var sample1 = TextDocument.find('foo'); sample1 == null");
  EXPECT_SCRIPT_VALID("TextDocument.new('foo')");
  EXPECT_SCRIPT_EQ("foo",
                   "var sample2 = TextDocument.find('foo'); sample2.name");
}

TEST_F(TextDocumentTest, TextDocument_remove) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = TextDocument.new('foo');"
      "var doc2 = TextDocument.new('bar');"
      "TextDocument.remove(doc1);");
  EXPECT_SCRIPT_TRUE("TextDocument.find('foo') == null");
  EXPECT_SCRIPT_EQ("1", "TextDocument.list.length");
  EXPECT_SCRIPT_VALID("TextDocument.remove(doc2);");
  EXPECT_SCRIPT_EQ("0", "TextDocument.list.length");
}

TEST_F(TextDocumentTest, charCodeAt) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "new TextRange(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("111", "doc.charCodeAt(1)");
}

TEST_F(TextDocumentTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('dispatchEvent');"
      "var event_type;"
      "doc.addEventListener('foo', function(e) { event_type = e.type; });");

  EXPECT_SCRIPT_VALID("doc.dispatchEvent(new Event('foo'));");
  EXPECT_SCRIPT_EQ("foo", "event_type")
      << "Instances of TextDocument is an event target.";
}

TEST_F(TextDocumentTest, getLineAndColumn) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('getLineAndColumn');"
      "var range = new TextRange(doc);"
      "range.text = '01\\n02\\n03\\n04\\n';"
      "function testIt(offset) {"
      "  var result = doc.getLineAndColumn_(offset);"
      "  return result.lineNumber + ',' + result.column;"
      "}");
  EXPECT_SCRIPT_EQ("1,0", "testIt(0)");
  EXPECT_SCRIPT_EQ("1,1", "testIt(1)");
  EXPECT_SCRIPT_EQ("1,2", "testIt(2)");
  EXPECT_SCRIPT_EQ("2,0", "testIt(3)");
  EXPECT_SCRIPT_EQ("2,1", "testIt(4)");
  EXPECT_SCRIPT_EQ(
      "RangeError: Failed to execute 'getLineAndColumn_' on 'TextDocument': "
      "Invalid offset 100, valid range is [0, 12]",
      "testIt(100)");
}

TEST_F(TextDocumentTest, length) {
  EXPECT_SCRIPT_VALID("var doc = TextDocument.new('length');");
  EXPECT_SCRIPT_EQ("0", "doc.length");
  EXPECT_SCRIPT_VALID("new TextRange(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("6", "doc.length");
}

TEST_F(TextDocumentTest, load_failed_open) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId(), 123);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var promise = doc.load('foo.cc')");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.UNKNOWN === doc.obsolete");
  EXPECT_SCRIPT_FALSE("doc.readonly");
}

TEST_F(TextDocumentTest, load_failed_read) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("ReadFile", 0, 123);
  mock_io_delegate()->SetCallResult("ReadFile", 123, 0);
  mock_io_delegate()->SetCallResult("CloseFile", 0, 0);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var beforeLoad, afterLoad;"
      "doc.addEventListener('beforeload', function() { beforeLoad = true; });"
      "doc.addEventListener('load', function() { afterLoad = true; });"
      "var promise = doc.load('foo.cc')");
  EXPECT_SCRIPT_TRUE("beforeLoad");
  EXPECT_SCRIPT_TRUE("afterLoad");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.UNKNOWN === doc.obsolete");
  EXPECT_SCRIPT_FALSE("doc.readonly");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
}

TEST_F(TextDocumentTest, load_succeeded) {
  std::vector<uint8_t> bytes{
      102, 111, 111, 13, 10,  // foo\r\n
      98,  97,  114, 13, 10,  // bar\r\n
  };
  mock_io_delegate()->set_bytes(bytes);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("ReadFile", 0,
                                    static_cast<int>(bytes.size()));
  mock_io_delegate()->SetCallResult("ReadFile", 0, 0);
  domapi::FileStatus file_status;
  file_status.file_size = 123456;
  file_status.is_directory = false;
  file_status.is_symlink = false;
  file_status.last_write_time = base::Time::FromJsTime(123456.0);
  file_status.readonly = true;
  mock_io_delegate()->SetFileStatus(file_status, 0);
  mock_io_delegate()->SetCallResult("CloseFile", 0, 0);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var beforeLoad, afterLoad;"
      "doc.addEventListener('beforeload', function() { beforeLoad = true; });"
      "doc.addEventListener('load', function() { afterLoad = true; });"
      "var promise = doc.load('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_SCRIPT_TRUE("beforeLoad");
  EXPECT_SCRIPT_TRUE("afterLoad");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_EQ("utf-8", "doc.encoding");
  EXPECT_SCRIPT_EQ("8", "doc.length");
  EXPECT_SCRIPT_EQ("3", "doc.newline");
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("123456", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_EQ("C++", "doc.mode.name");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.NO === doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.readonly") << "set readonly from file attribute";
}

TEST_F(TextDocumentTest, mode) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var mode = Mode.chooseModeByFileName('foo.txt');"
      "doc.mode = mode;");
  EXPECT_SCRIPT_TRUE("doc.mode === mode");
}

TEST_F(TextDocumentTest, mode_auto_mode) {
  EXPECT_SCRIPT_VALID(
      "function testIt(fileName) {"
      "  var doc = TextDocument.new(fileName);"
      "  doc.mode = Mode.chooseModeByFileName(fileName);"
      "  return doc.mode.constructor.name;"
      "}");
  EXPECT_SCRIPT_EQ("CppMode", "testIt('foo.cc')");
  EXPECT_SCRIPT_EQ("CppMode", "testIt('foo.cpp')");
  EXPECT_SCRIPT_EQ("CppMode", "testIt('foo.h')");
  EXPECT_SCRIPT_EQ("JavaMode", "testIt('foo.java')");
  EXPECT_SCRIPT_EQ("JavaScriptMode", "testIt('foo.js')");
}

TEST_F(TextDocumentTest, modified) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var range = new TextRange(doc);");
  EXPECT_SCRIPT_FALSE("doc.modified");

  EXPECT_SCRIPT_VALID("range.text = 'foo';");
  EXPECT_SCRIPT_TRUE("doc.modified");

#if 0
  // Since mock ViewDelegate doesn't reset modified flag, we disable
  // this test case.
  EXPECT_CALL(*mock_view_impl(), SaveFile(_, Eq(L"foo")));
  EXPECT_SCRIPT_VALID("doc.save('foo')");
  EXPECT_SCRIPT_FALSE("doc.modified");
#endif
}

TEST_F(TextDocumentTest, name) {
  EXPECT_SCRIPT_EQ("baz",
                   "var sample1 = TextDocument.new('baz'); sample1.name");
}

TEST_F(TextDocumentTest, newline) {
  EXPECT_SCRIPT_VALID("var doc = TextDocument.new('newline');");
  // See |Newline| in "evita/dom/enums.js"
  EXPECT_SCRIPT_EQ("0", "doc.newline") << "Default newline is detect.";
  EXPECT_SCRIPT_VALID("doc.newline = 1;") << "Set newline to LF.";
  EXPECT_SCRIPT_EQ("1", "doc.newline");
}

TEST_F(TextDocumentTest, parseFileProperties) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var range = new TextRange(doc);"
      "range.text = '-*- var1: foo; VAR2: bar ; var3 : baz; -*-';"
      "doc.parseFileProperties();");
  // Property names are case sensitive
  // Property values doesn't have leading and trailing whitespaces.
  EXPECT_SCRIPT_EQ("foo", "doc.properties.get('var1')");
  EXPECT_SCRIPT_EQ("undefined", "doc.properties.get('var2')");
  EXPECT_SCRIPT_EQ("bar", "doc.properties.get('VAR2')");
  EXPECT_SCRIPT_EQ("baz", "doc.properties.get('var3')");
}

TEST_F(TextDocumentTest, properties) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = TextDocument.new('doc1');"
      "doc1.properties.set('foo', 'bar');"
      "var doc2 = TextDocument.new('doc2');"
      "doc2.properties.set('foo', 'baz');");
  EXPECT_SCRIPT_EQ("bar", "doc1.properties.get('foo')");
  EXPECT_SCRIPT_EQ("baz", "doc2.properties.get('foo')");
}

TEST_F(TextDocumentTest, redo) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('redo');"
      "var range = new TextRange(doc);"
      "range.text = 'foo';"
      "doc.undo(3);"
      "doc.redo(0);"
      "range.start = 0;"
      "range.end = doc.length");
  EXPECT_SCRIPT_EQ("foo", "range.text");
}

TEST_F(TextDocumentTest, renameTo) {
  EXPECT_SCRIPT_VALID("var doc = TextDocument.new('foo'); doc.renameTo('bar')");
  EXPECT_SCRIPT_EQ("bar", "doc.name");
}

TEST_F(TextDocumentTest, save_failed_open) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId(), 123);
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "doc.save('foo.cc');");
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.UNKNOWN === doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(TextDocumentTest, save_failed_encode) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("CloseFile", 0);
  mock_io_delegate()->SetCallResult("RemoveFile", 0);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "new TextRange(doc).text = 'f\\u0234o\\nbar\\n';"
      "doc.encoding = 'shift_jis';"
      "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(1, mock_io_delegate()->num_remove_called());
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.UNKNOWN === doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(TextDocumentTest, save_failed_write) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("WriteFile", 123);
  mock_io_delegate()->SetCallResult("CloseFile", 0);
  mock_io_delegate()->SetCallResult("RemoveFile", 0);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "new TextRange(doc).text = 'foo\\nbar\\n';"
      "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(1, mock_io_delegate()->num_remove_called());
  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.UNKNOWN === doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(TextDocumentTest, save_succeeded) {
  std::vector<uint8_t> expected_bytes{
      102, 111, 111, 13, 10,  // foo\r\n
      98,  97,  114, 13, 10,  // bar\r\n
  };
  mock_io_delegate()->set_bytes(expected_bytes);
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("WriteFile", 0,
                                    static_cast<int>(expected_bytes.size()));
  mock_io_delegate()->SetCallResult("CloseFile", 0);
  mock_io_delegate()->SetCallResult("MoveFile", 0);
  domapi::FileStatus file_status;
  file_status.file_size = 10;
  file_status.is_directory = false;
  file_status.is_symlink = false;
  file_status.last_write_time = base::Time::FromJsTime(123456.0);
  file_status.readonly = false;
  mock_io_delegate()->SetFileStatus(file_status, 0);

  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "new TextRange(doc).text = 'foo\\nbar\\n';"
      "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(expected_bytes, mock_io_delegate()->bytes());

  EXPECT_SCRIPT_TRUE("doc.fileName.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("123456", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("TextDocument.Obsolete.NO === doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(TextDocumentTest, setSpelling) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = TextDocument.new('spelling');"
      "var range1 = new TextRange(doc1);"
      "range1.text = 'foo bar baz';"
      "doc1.setSpelling(4, 7, 2);");
  EXPECT_SCRIPT_EQ("", "doc1.spellingAt(0)");
  EXPECT_SCRIPT_EQ("", "doc1.spellingAt(3)");
  EXPECT_SCRIPT_EQ("misspelled", "doc1.spellingAt(4)");
  EXPECT_SCRIPT_EQ("misspelled", "doc1.spellingAt(5)");
  EXPECT_SCRIPT_EQ("misspelled", "doc1.spellingAt(6)");
  EXPECT_SCRIPT_EQ("", "doc1.spellingAt(7)");
  EXPECT_SCRIPT_EQ("", "doc1.spellingAt(8)");
}

TEST_F(TextDocumentTest, setSyntax) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('syntax');"
      "var range = new TextRange(doc);"
      "range.text = 'foo bar baz';"
      "doc.setSyntax(0, 3, 'keyword');"
      "doc.setSyntax(4, 7, 'operator');"
      "doc.setSyntax(8, 11, 'identifier');");
  EXPECT_SCRIPT_EQ("keyword", "doc.syntaxAt(0)");
  EXPECT_SCRIPT_EQ("keyword", "doc.syntaxAt(1)");
  EXPECT_SCRIPT_EQ("keyword", "doc.syntaxAt(2)");
  EXPECT_SCRIPT_EQ("normal", "doc.syntaxAt(3)");
  EXPECT_SCRIPT_EQ("operator", "doc.syntaxAt(4)");
  EXPECT_SCRIPT_EQ("operator", "doc.syntaxAt(5)");
  EXPECT_SCRIPT_EQ("operator", "doc.syntaxAt(6)");
  EXPECT_SCRIPT_EQ("normal", "doc.syntaxAt(7)");
  EXPECT_SCRIPT_EQ("identifier", "doc.syntaxAt(8)");
  EXPECT_SCRIPT_EQ("identifier", "doc.syntaxAt(9)");
  EXPECT_SCRIPT_EQ("identifier", "doc.syntaxAt(10)");
  EXPECT_SCRIPT_EQ("normal", "doc.syntaxAt(11)");
}

TEST_F(TextDocumentTest, slice) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('slice');"
      "var range = new TextRange(doc);"
      "range.text = '0123456789';");
  EXPECT_SCRIPT_EQ("56789", "doc.slice(5)");
  EXPECT_SCRIPT_EQ("34", "doc.slice(3, 5)");
  EXPECT_SCRIPT_EQ("789", "doc.slice(-3)");
  EXPECT_SCRIPT_EQ("3456", "doc.slice(-7, -3)");
  EXPECT_SCRIPT_EQ("", "doc.slice(100)");
  EXPECT_SCRIPT_EQ("56789", "doc.slice(5, 100)");
}

TEST_F(TextDocumentTest, state) {
  EXPECT_SCRIPT_VALID("var doc = TextDocument.new('state');");
  // See |text::Buffer::State| for |enum| fields.
  EXPECT_SCRIPT_EQ("0", "doc.state");
}

TEST_F(TextDocumentTest, undo) {
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('undo');"
      "var range = new TextRange(doc);"
      "range.text = 'foo';"
      "doc.undo(3);"
      "range.start = 0;"
      "range.end = doc.length");
  EXPECT_SCRIPT_EQ("", "range.text");
}

}  // namespace dom
