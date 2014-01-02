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
  EXPECT_EQ("bar", RunScript("var sample1 = new Document('bar');"
                             "sample1.name"));
  EXPECT_EQ("bar (2)",
            RunScript("var sample2 = new Document('bar');"
                      "sample2.name"));
  EXPECT_EQ("bar (3)",
            RunScript("var sample2 = new Document('bar');"
                      "sample2.name"));

  RunScript("new Document('bar.cc')");
  EXPECT_EQ("bar (2).cc",
            RunScript("var sample2 = new Document('bar.cc');"
                      "sample2.name"));
  EXPECT_EQ("bar (3).cc",
            RunScript("var sample2 = new Document('bar.cc');"
                      "sample2.name"));

  RunScript("new Document('.bar')");
  EXPECT_EQ(".bar (2)",
            RunScript("var sample2 = new Document('.bar');"
                      "sample2.name"));
  EXPECT_EQ(".bar (3)",
            RunScript("var sample2 = new Document('.bar');"
                      "sample2.name"));
}

TEST_F(DocumentTest, FindBuffer) {
  EXPECT_EQ("PASS",
            RunScript("var sample1 = Document.find('foo');"
                      "sample1 == null ? 'PASS' : 'FAIL'"));
  RunScript("new Document('foo')");
  EXPECT_EQ("foo",
            RunScript("var sample2 = Document.find('foo');"
                      "sample2.name"));
}

TEST_F(DocumentTest, Name) {
  EXPECT_EQ("DocumentTest",
            RunScript("var sample1 = new Document('DocumentTest');"
                      "sample1.name"));
}

}  // namespace
