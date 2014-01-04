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

class RangeTest : public dom::AbstractDomTest {
  protected: RangeTest() {
  }
  public: virtual ~RangeTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

TEST_F(RangeTest, Constructor) {
  // TODO(yosi): We should remove all buffers for each test case.
  RunScript("var doc1 = new Document('range')");
  RunScript("var range1 = new Range(doc1)");
  RunScript("var range2 = new Range(doc1, 0)");
  RunScript("var range3 = new Range(doc1, 0, 0)");
  EXPECT_EQ("true", RunScript("range1.document === doc1"));
  EXPECT_EQ("true", RunScript("range2.document === doc1"));
  EXPECT_EQ("true", RunScript("range3.document === doc1"));
}

TEST_F(RangeTest, text) {
  RunScript("var doc1 = new Document('text');"
            "var range1 = new Range(doc1);"
            "range1.text = 'abcdefghijkl';"
            "var range2 = new Range(doc1, 3, 6);");
  EXPECT_EQ("def", RunScript("range2.text"));
}

}  // namespace
