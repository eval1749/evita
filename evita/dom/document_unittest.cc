// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

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

  RunScript("new Document('bar.cc')");
  EXPECT_SCRIPT_EQ("bar (2).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");
  EXPECT_SCRIPT_EQ("bar (3).cc",
                   "var sample2 = new Document('bar.cc'); sample2.name");

  RunScript("new Document('.bar')");
  EXPECT_SCRIPT_EQ(".bar (2)",
                   "var sample2 = new Document('.bar'); sample2.name");
  EXPECT_SCRIPT_EQ(".bar (3)",
                   "var sample2 = new Document('.bar'); sample2.name");
}

TEST_F(DocumentTest, DocumentFind) {
  EXPECT_SCRIPT_TRUE("var sample1 = Document.find('foo'); sample1 == null");
  RunScript("new Document('foo')");
  EXPECT_SCRIPT_EQ("foo", "var sample2 = Document.find('foo'); sample2.name");
}

TEST_F(DocumentTest, charCodeAt) {
  RunScript("var doc = new Document('foo');"
            "new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("111", "doc.charCodeAt(1)");
}

TEST_F(DocumentTest, length) {
  RunScript("var doc = new Document('length');");
  EXPECT_SCRIPT_EQ("0", "doc.length");
  RunScript("new Range(doc).text = 'foobar';");
  EXPECT_SCRIPT_EQ("6", "doc.length");
}

TEST_F(DocumentTest, name) {
  EXPECT_SCRIPT_EQ("baz", "var sample1 = new Document('baz'); sample1.name");
}

}  // namespace
