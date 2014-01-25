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

class TextSelectionTest : public dom::AbstractDomTest {
  protected: TextSelectionTest() {
  }
  public: virtual ~TextSelectionTest() {
  }

  protected: virtual void SetUp() override {
    dom::AbstractDomTest::SetUp();
    EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
    EXPECT_SCRIPT_VALID(
      "var doc = new Document('delete');"
      "var window = new TextWindow(new Range(doc));"
      "var selection = window.selection;"
      "var range = selection.range;"
      "function doTest(sample, sampler) {"
      "  range.start = 0;"
      "  range.end = doc.length;"
      "  sample = sample.replace(/[$]/g, '\\n');"
      "  range.text = sample.replace(/[|]/g, '');"
      "  range.collapseTo(sample.indexOf('|'));"
      "  var before_end = sample.lastIndexOf('|');"
      "  if (range.start != before_end)"
      "    --before_end;"
      "  range.end = before_end;"
      "  sampler(selection);"
      "  var start = range.start;"
      "  var end = range.end;"
      "  range.start = 0;"
      "  range.end = doc.length;"
      "  var result = range.text.replace(/\\n/g, '$');"
      "  if (start == end)"
      "    return result.substr(0, start) + '|' + result.substr(start);"
      "  return result.substr(0, start) + '|' +"
      "         result.substring(start, end) + '|' +"
      "         result.substr(end);"
      "}");
  }

  DISALLOW_COPY_AND_ASSIGN(TextSelectionTest);
};

TEST_F(TextSelectionTest, active) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection;"
      "sample.range.text = 'foo';");
  EXPECT_SCRIPT_EQ("3", "sample.active");
  EXPECT_SCRIPT_VALID("sample.startIsActive = true;");
  EXPECT_SCRIPT_EQ("0", "sample.active");
}

TEST_F(TextSelectionTest, endof) {
  EXPECT_SCRIPT_VALID(
    "function testIt(sample, unit) {"
    "  return doTest(sample, function(selection) {"
    "   selection.endOf(unit);"
    "  });"
    "}");
  EXPECT_SCRIPT_EQ("foo|$bar$$baz",
                   "testIt('|foo$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar|$$baz",
                   "testIt('foo$|bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$|$baz",
                   "testIt('foo$bar$|$baz', Unit.LINE)");

  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo b|ar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz|", "testIt('foo bar  |baz', Unit.WORD)");
}

TEST_F(TextSelectionTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection");
  EXPECT_SCRIPT_TRUE("sample instanceof TextSelection");
  EXPECT_SCRIPT_FALSE("sample instanceof Range");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_EQ("0", "sample.range.start");
  EXPECT_SCRIPT_EQ("0", "sample.range.end");
  EXPECT_SCRIPT_FALSE("sample.startIsActive");
  EXPECT_SCRIPT_VALID("sample.startIsActive = true;");
  EXPECT_SCRIPT_TRUE("sample.startIsActive");
}

}  // namespace
