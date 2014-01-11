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

  protected: void PopulateSample(const char* sample) {
    RunScript(std::string() +
        "var doc = new Document('sample');"
        "var r = new Range(doc);" +
        "r.text = '" + sample + "';");
  }

  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

static const char* kInvalidPosition = "Error: Invalid position.";

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

TEST_F(RangeTest, endOf) {
  RunScript("var doc = new Document('endOf');"
            "var range = new Range(doc);"
                         //012345678901
            "range.text = 'foo bar  baz';"
            "function test(x) {"
            "  range.end = x;"
            "  range.endOf(Unit.WORD);"
            "  return range.end;"
            "}");
  EXPECT_SCRIPT_EQ("3", "test(0)");
  EXPECT_SCRIPT_EQ("3", "test(1)");
  EXPECT_SCRIPT_EQ("3", "test(2)");
  EXPECT_SCRIPT_EQ("7", "test(5)");
  EXPECT_SCRIPT_EQ("7", "test(7)");
  EXPECT_SCRIPT_EQ("8", "test(8)");
}

TEST_F(RangeTest, set_start_end) {
  RunScript("var doc1 = new Document('text');"
            "var range1 = new Range(doc1);"
            "range1.text = 'abcdefghijkl';"
            "range1.start = 5;");
  EXPECT_EQ("5", RunScript("range1.start"));
  EXPECT_EQ("1 5",
            RunScript("range1.end = 1; range1.start + ' ' + range1.end"));
  EXPECT_EQ(kInvalidPosition, RunScript("range1.start = -1"));
  EXPECT_EQ(kInvalidPosition, RunScript("range1.start = 100"));
  EXPECT_EQ(kInvalidPosition, RunScript("range1.end = -1"));
  EXPECT_EQ(kInvalidPosition, RunScript("range1.end = 100"));
}

TEST_F(RangeTest, startOf) {
  RunScript("var doc = new Document('startOf');"
            "var range = new Range(doc);"
            "range.text = 'foo bar baz';"
            "function test(x) {"
            "  range.start = x;"
            "  range.startOf(Unit.WORD);"
            "  return range.start;"
            "}");
  EXPECT_SCRIPT_EQ("0", "test(2)");
  EXPECT_SCRIPT_EQ("4", "test(5)");
}

TEST_F(RangeTest, text) {
  RunScript("var doc1 = new Document('text');"
            "var range1 = new Range(doc1);"
            "range1.text = 'abcdefghijkl';"
            "var range2 = new Range(doc1, 3, 6);");
  EXPECT_EQ("def", RunScript("range2.text"));
}

TEST_F(RangeTest, toLocalLocaleLowerCase) {
  PopulateSample("ABCDEFGHIJ");
  EXPECT_EQ("abcdefghij", RunScript("r.toLocaleLowerCase(); r.text"));
}

TEST_F(RangeTest, toLocalLowerCase) {
  PopulateSample("ABCDEFGHIJ");
  EXPECT_EQ("abcdefghij", RunScript("r.toLowerCase(); r.text"));
}

TEST_F(RangeTest, toLocalLocaleUpperCase) {
  PopulateSample("abcdefghij");
  EXPECT_EQ("ABCDEFGHIJ", RunScript("r.toLocaleUpperCase(); r.text"));
}

TEST_F(RangeTest, toLocalUpperCase) {
  PopulateSample("abcdefghij");
  EXPECT_EQ("ABCDEFGHIJ", RunScript("r.toUpperCase(); r.text"));
}

}  // namespace
