// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "base/strings/utf_string_conversions.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/file_path.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

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

TEST_F(DocumentTest, Document_load) {
  EXPECT_CALL(*mock_view_impl(),
              LoadFile(_, Eq(dom::FilePath::FullPath(L"foo"))));
  EXPECT_CALL(*mock_view_impl(),
              LoadFile(_, Eq(dom::FilePath::FullPath(L"bar"))));
  EXPECT_SCRIPT_VALID("var a = Document.load('foo');");
  auto const document = dom::Document::Find(L"foo");
  auto const absoulte_filename = dom::FilePath::FullPath(L"foo");
  document->buffer()->SetFile(absoulte_filename, FileTime());
  EXPECT_SCRIPT_VALID("var b = Document.load('foo');"
            "var c = Document.load('bar');");
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

TEST_F(DocumentTest, length) {
  EXPECT_SCRIPT_VALID("var doc = new Document('length');");
  EXPECT_SCRIPT_EQ("0", "doc.length");
  EXPECT_SCRIPT_VALID("new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("6", "doc.length");
}

TEST_F(DocumentTest, load_) {
  EXPECT_CALL(*mock_view_impl(), LoadFile(_, Eq(L"foo")));
  EXPECT_SCRIPT_VALID("var doc = new Document('foo'); doc.load_('foo')");
  
}

TEST_F(DocumentTest, mode) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var mode = new PlainTextMode();"
      "doc.mode = mode;");
  EXPECT_SCRIPT_TRUE("doc.mode === mode");
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

TEST_F(DocumentTest, save) {
  EXPECT_CALL(*mock_view_impl(), SaveFile(_, Eq(L"foo")));
  EXPECT_SCRIPT_VALID("var doc = new Document('foo'); doc.save('foo')");
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
