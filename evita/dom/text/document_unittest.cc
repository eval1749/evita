// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/run_loop.h"
#pragma warning(pop)
#include "base/strings/utf_string_conversions.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/document_set.h"
#include "evita/dom/file_path.h"
#include "evita/dom/mock_io_delegate.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/text/buffer.h"

namespace {

using ::testing::Eq;
using ::testing::_;

class DocumentTest : public dom::AbstractDomTest {
  protected: DocumentTest() {
  }
  public: virtual ~DocumentTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentTest);
};

TEST_F(DocumentTest, Constructor) {
  // TODO(yosi): We should remove all buffers for each test case.
  EXPECT_SCRIPT_EQ("bar", "var sample1 = new Document('bar');"
                          "sample1.name");
  EXPECT_SCRIPT_EQ("bar (2)",
                   "var sample2 = new Document('bar'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3)",
                   "var sample2 = new Document('bar'); sample2.name");

  EXPECT_SCRIPT_VALID("new Document('bar.cc')");
  EXPECT_SCRIPT_EQ("bar (2).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");

  EXPECT_SCRIPT_VALID("new Document('.bar')");
  EXPECT_SCRIPT_EQ(".bar (2)",
                   "var sample2 = new Document('.bar'); sample2.name");
  EXPECT_SCRIPT_EQ(".bar (3)",
                   "var sample2 = new Document('.bar'); sample2.name");
}

TEST_F(DocumentTest, Document_addObserver) {
  EXPECT_SCRIPT_VALID(
      "var result_doc, result_type;"
      "function callback(type, doc) {"
      "  result_doc = doc;"
      "  result_type = type;"
      "}"
      "Document.addObserver(callback);"
      "var doc1 = new Document('addObserver');");
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("add", "result_type") <<
      "The observer gets 'add' notification.";

  EXPECT_SCRIPT_VALID("Document.remove(doc1)");
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("remove", "result_type") <<
      "The observer gets 'remove' notification.";

  EXPECT_SCRIPT_VALID("Document.removeObserver(callback)");
  EXPECT_SCRIPT_VALID("var doc2 = new Document('addObserver2')");
  {
    base::RunLoop run_loop;
    run_loop.RunUntilIdle();
  }
  EXPECT_SCRIPT_TRUE("result_doc === doc1");
  EXPECT_SCRIPT_EQ("remove", "result_type") <<
      "The observer doesn't get notification.";
}

TEST_F(DocumentTest, Document_list) {
  EXPECT_SCRIPT_VALID(
      "['foo', 'bar', 'baz'].forEach(function(name) {"
      "  new Document(name);"
      "});"
      "var samples = Document.list.sort(function(a, b) {"
      "  return a.name.localeCompare(b.name);"
      "});");
  EXPECT_SCRIPT_EQ("3", "samples.length");
  EXPECT_SCRIPT_EQ("bar", "samples[0].name");
  EXPECT_SCRIPT_EQ("baz", "samples[1].name");
  EXPECT_SCRIPT_EQ("foo", "samples[2].name");
}

TEST_F(DocumentTest, Document_open) {
  EXPECT_SCRIPT_VALID("var a = Document.open('foo');");
  auto const absoulte_filename = dom::FilePath::FullPath(L"foo");
  EXPECT_SCRIPT_VALID("var b = Document.open('foo');"
                      "var c = Document.open('bar');");
  EXPECT_SCRIPT_EQ(base::UTF16ToUTF8(absoulte_filename), "a.filename");
  EXPECT_SCRIPT_TRUE("a === b");
  EXPECT_SCRIPT_TRUE("a !== c");
}

TEST_F(DocumentTest, DocumentFind) {
  EXPECT_SCRIPT_TRUE("var sample1 = Document.find('foo'); sample1 == null");
  EXPECT_SCRIPT_VALID("new Document('foo')");
  EXPECT_SCRIPT_EQ("foo", "var sample2 = Document.find('foo'); sample2.name");
}

TEST_F(DocumentTest, DocumentGetOrNew) {
  EXPECT_SCRIPT_VALID("var doc = new Document('foo');");
  EXPECT_SCRIPT_TRUE("doc === Document.getOrNew('foo')");
}

TEST_F(DocumentTest, Document_remove) {
  EXPECT_SCRIPT_VALID("var doc1 = new Document('foo');"
            "var doc2 = new Document('bar');"
            "Document.remove(doc1);");
  EXPECT_SCRIPT_TRUE("Document.find('foo') == null");
  EXPECT_SCRIPT_EQ("1", "Document.list.length");
  EXPECT_SCRIPT_VALID("Document.remove(doc2);");
  EXPECT_SCRIPT_EQ("0", "Document.list.length");
}

TEST_F(DocumentTest, charCodeAt_) {
  EXPECT_SCRIPT_VALID("var doc = new Document('foo');"
            "new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("111", "doc.charCodeAt_(1)");
}

TEST_F(DocumentTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('dispatchEvent');"
      "var event_type;"
      "doc.addEventListener('foo', function(e) { event_type = e.type; });");

  EXPECT_SCRIPT_VALID("doc.dispatchEvent(new Event('foo'));");
  EXPECT_SCRIPT_EQ("foo", "event_type") <<
      "Instances of Document is an event target.";
}

TEST_F(DocumentTest, getLineAndColumn) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('getLineAndColumn');"
      "var range = new Range(doc);"
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
  EXPECT_SCRIPT_EQ("RangeError: Invalid position 100, valid range is [0, 12]",
                   "testIt(100)");
}

TEST_F(DocumentTest, length) {
  EXPECT_SCRIPT_VALID("var doc = new Document('length');");
  EXPECT_SCRIPT_EQ("0", "doc.length");
  EXPECT_SCRIPT_VALID("new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("6", "doc.length");
}

TEST_F(DocumentTest, load_failed_open) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId(), 123);

  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var promise = doc.load('foo.cc')");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_TRUE("doc.obsolete == Document.Obsolete.UNKNOWN");
  EXPECT_SCRIPT_FALSE("doc.readonly");
}

TEST_F(DocumentTest, load_failed_read) {
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("ReadFile", 0, 123);
  mock_io_delegate()->SetCallResult("ReadFile", 123, 0);
  mock_io_delegate()->SetCallResult("CloseFile", 0, 0);

  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var promise = doc.load('foo.cc')");
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_TRUE("doc.obsolete == Document.Obsolete.UNKNOWN");
  EXPECT_SCRIPT_FALSE("doc.readonly");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
}

TEST_F(DocumentTest, load_succeeded) {
  std::vector<uint8_t> bytes {
     102, 111, 111, 13, 10, // foo\r\n
     98, 97, 114, 13, 10, // bar\r\n
  };
  mock_io_delegate()->set_bytes(bytes);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("ReadFile", 0, bytes.size());
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
    "var doc = new Document('foo');"
    "var promise = doc.load('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_SCRIPT_TRUE("promise instanceof Promise");
  EXPECT_SCRIPT_EQ("utf-8", "doc.encoding");
  EXPECT_SCRIPT_EQ("8", "doc.length");
  EXPECT_SCRIPT_EQ("3", "doc.newline");
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("123456", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_TRUE("doc.lastStatTime_.valueOf() != 0");
  EXPECT_SCRIPT_EQ("C++", "doc.mode.name");
  EXPECT_SCRIPT_EQ("0", "doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.readonly") << "set readonly from file attribute";
}

TEST_F(DocumentTest, mode) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var mode = new PlainTextMode();"
      "doc.mode = mode;");
  EXPECT_SCRIPT_TRUE("doc.mode === mode");
}

TEST_F(DocumentTest, mode_auto_mode) {
  EXPECT_SCRIPT_VALID(
      "function testIt(filename) {"
      "  return (new Document(filename)).mode.constructor.name;"
      "}");
  EXPECT_SCRIPT_EQ("CxxMode", "testIt('foo.cc')");
  EXPECT_SCRIPT_EQ("CxxMode", "testIt('foo.cpp')");
  EXPECT_SCRIPT_EQ("CxxMode", "testIt('foo.h')");
  EXPECT_SCRIPT_EQ("JavaMode", "testIt('foo.java')");
  EXPECT_SCRIPT_EQ("JavaScriptMode", "testIt('foo.js')");
}

TEST_F(DocumentTest, modified) {
  EXPECT_SCRIPT_VALID("var doc = new Document('foo');"
            "var range = new Range(doc);");
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

TEST_F(DocumentTest, name) {
  EXPECT_SCRIPT_EQ("baz", "var sample1 = new Document('baz'); sample1.name");
}

TEST_F(DocumentTest, newline) {
  EXPECT_SCRIPT_VALID("var doc = new Document('newline');");
  // See |NewlineMode| in "evita/ed_defs.h"
  EXPECT_SCRIPT_EQ("0", "doc.newline") << "Default newline is detect.";
  EXPECT_SCRIPT_VALID("doc.newline = 1;") << "Set newline to LF.";
  EXPECT_SCRIPT_EQ("1", "doc.newline");
}

TEST_F(DocumentTest, parseFileProperties) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "var range = new Range(doc);"
    "range.text = '-*- var1: foo; VAR2: bar ; var3 : baz; -*-';"
    "doc.parseFileProperties();");
  // Property names are case sensitive
  // Property values doesn't have leading and trailing whitespaces.
  EXPECT_SCRIPT_EQ("foo", "doc.properties.get('var1')");
  EXPECT_SCRIPT_EQ("undefined", "doc.properties.get('var2')");
  EXPECT_SCRIPT_EQ("bar", "doc.properties.get('VAR2')");
  EXPECT_SCRIPT_EQ("baz", "doc.properties.get('var3')");
}

TEST_F(DocumentTest, properties) {
  EXPECT_SCRIPT_VALID(
    "var doc1 = new Document('doc1');"
    "doc1.properties.set('foo', 'bar');"
    "var doc2 = new Document('doc2');"
    "doc2.properties.set('foo', 'baz');");
  EXPECT_SCRIPT_EQ("bar", "doc1.properties.get('foo')");
  EXPECT_SCRIPT_EQ("baz", "doc2.properties.get('foo')");
}

TEST_F(DocumentTest, redo) {
  EXPECT_SCRIPT_VALID("var doc = new Document('redo');"
            "var range = new Range(doc);"
            "range.text = 'foo';"
            "doc.undo(3);"
            "doc.redo(0);"
            "range.start = 0;"
            "range.end = doc.length");
  EXPECT_SCRIPT_EQ("foo", "range.text");
}

TEST_F(DocumentTest, renameTo) {
  EXPECT_SCRIPT_VALID("var doc = new Document('foo'); doc.renameTo('bar')");
  EXPECT_SCRIPT_EQ("bar", "doc.name");
}

TEST_F(DocumentTest, save_failed_open) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId(), 123);
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "doc.save('foo.cc');");
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_EQ("-1", "doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(DocumentTest, save_failed_encode) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("CloseFile", 0);
  mock_io_delegate()->SetCallResult("RemoveFile", 0);

  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "new Range(doc).text = 'f\\u0234o\\nbar\\n';"
    "doc.encoding = 'shift_jis';"
    "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(1, mock_io_delegate()->num_remove_called());
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_EQ("-1", "doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(DocumentTest, save_failed_write) {
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("WriteFile", 123);
  mock_io_delegate()->SetCallResult("CloseFile", 0);
  mock_io_delegate()->SetCallResult("RemoveFile", 0);

  EXPECT_SCRIPT_VALID(
    "var doc = new Document('foo');"
    "new Range(doc).text = 'foo\\nbar\\n';"
    "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(1, mock_io_delegate()->num_remove_called());
  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("0", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_EQ("-1", "doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(DocumentTest, save_succeeded) {
  std::vector<uint8_t> expected_bytes {
     102, 111, 111, 13, 10, // foo\r\n
     98, 97, 114, 13, 10, // bar\r\n
  };
  mock_io_delegate()->set_bytes(expected_bytes);
  mock_io_delegate()->SetMakeTempFileName(L"foo.tmp", 0);
  mock_io_delegate()->SetOpenFileResult(domapi::IoContextId::New(), 0);
  mock_io_delegate()->SetCallResult("WriteFile", 0, expected_bytes.size());
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
    "var doc = new Document('foo');"
    "new Range(doc).text = 'foo\\nbar\\n';"
    "doc.save('foo.cc');");
  EXPECT_EQ(1, mock_io_delegate()->num_close_called());
  EXPECT_EQ(expected_bytes, mock_io_delegate()->bytes());

  EXPECT_SCRIPT_TRUE("doc.filename.endsWith('foo.cc')");
  EXPECT_SCRIPT_EQ("123456", "doc.lastWriteTime.valueOf()");
  EXPECT_SCRIPT_EQ("0", "doc.obsolete");
  EXPECT_SCRIPT_TRUE("doc.lastStatusCheckTime_ != new Date(0)");
}

TEST_F(DocumentTest, slice) {
  EXPECT_SCRIPT_VALID("var doc = new Document('slice');"
    "var range = new Range(doc);"
    "range.text = '0123456789';");
  EXPECT_SCRIPT_EQ("56789", "doc.slice(5)");
  EXPECT_SCRIPT_EQ("34", "doc.slice(3, 5)");
  EXPECT_SCRIPT_EQ("789", "doc.slice(-3)");
  EXPECT_SCRIPT_EQ("3456", "doc.slice(-7, -3)");
  EXPECT_SCRIPT_EQ("", "doc.slice(100)");
  EXPECT_SCRIPT_EQ("56789", "doc.slice(5, 100)");
}

TEST_F(DocumentTest, state) {
  EXPECT_SCRIPT_VALID("var doc = new Document('state');");
  // See |text::Buffer::State| for |enum| fields.
  EXPECT_SCRIPT_EQ("0", "doc.state");
}

TEST_F(DocumentTest, undo) {
  EXPECT_SCRIPT_VALID("var doc = new Document('undo');"
            "var range = new Range(doc);"
            "range.text = 'foo';"
            "doc.undo(3);"
            "range.start = 0;"
            "range.end = doc.length");
  EXPECT_SCRIPT_EQ("", "range.text");
}

}  // namespace
