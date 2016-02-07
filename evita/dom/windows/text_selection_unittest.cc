// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/macros.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/testing/mock_view_impl.h"
#include "gmock/gmock.h"

namespace dom {

using ::testing::_;

class TextSelectionTest : public AbstractDomTest {
 protected:
  TextSelectionTest() = default;

 private:
  void SetUp() override {
    AbstractDomTest::SetUp();
    EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
    EXPECT_SCRIPT_VALID(
        "var doc = TextDocument.new('delete');"
        "var window = new TextWindow(new TextRange(doc));"
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
        "  var end_mark = selection.startIsActive ? '^' : '|';"
        "  var start_mark = selection.startIsActive ? '|' : '^';"
        "  return result.substr(0, start) + start_mark +"
        "         result.substring(start, end) + end_mark +"
        "         result.substr(end);"
        "}");
  }

  DISALLOW_COPY_AND_ASSIGN(TextSelectionTest);
};

TEST_F(TextSelectionTest, focusOffset) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var range = new TextRange(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection;"
      "sample.range.text = 'foo';");
  EXPECT_SCRIPT_EQ("3", "sample.focusOffset");
  EXPECT_SCRIPT_VALID("sample.startIsActive = true;");
  EXPECT_SCRIPT_EQ("0", "sample.focusOffset");
}

TEST_F(TextSelectionTest, endKey) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit, opt_alter) {"
      "  return doTest(sample, function(selection) {"
      "    selection.endKey(unit);"
      "  });"
      "}");

  EXPECT_SCRIPT_EQ("foo|$bar$", "testIt('|foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo|$bar$", "testIt('fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo|  $bar$", "testIt('|foo  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo|  $bar$", "testIt('fo|o  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo  |$bar$", "testIt('foo|  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo|  $bar$", "testIt('foo  |$bar$', Unit.LINE)");
}

TEST_F(TextSelectionTest, endKeyExtend) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit, opt_alter) {"
      "  return doTest(sample, function(selection) {"
      "    selection.endKey(unit, Alter.EXTEND);"
      "  });"
      "}");

  EXPECT_SCRIPT_EQ("^foo|$bar$", "testIt('|foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("fo^o|$bar$", "testIt('fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("^foo|  $bar$", "testIt('|foo  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("fo^o|  $bar$", "testIt('fo|o  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo^  |$bar$", "testIt('foo|  $bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo|  ^$bar$", "testIt('foo  |$bar$', Unit.LINE)");
}

TEST_F(TextSelectionTest, endOf) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit) {"
      "  return doTest(sample, function(selection) {"
      "   selection.endOf(unit);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("foo|$bar$$baz", "testIt('|foo$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar|$$baz", "testIt('foo$|bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$|$baz", "testIt('foo$bar$|$baz', Unit.LINE)");

  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo b|ar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz|", "testIt('foo bar  |baz', Unit.WORD)");
}

TEST_F(TextSelectionTest, endOfExtend) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit) {"
      "  return doTest(sample, function(selection) {"
      "   selection.endOf(unit, Alter.EXTEND);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("^foo|$bar$$baz", "testIt('|foo$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$^bar|$$baz", "testIt('foo$|bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$|$baz", "testIt('foo$bar$|$baz', Unit.LINE)");

  EXPECT_SCRIPT_EQ("^foo| bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("f^oo| bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("fo^o| bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo b^ar|  baz", "testIt('foo b|ar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  ^baz|", "testIt('foo bar  |baz', Unit.WORD)");
}

TEST_F(TextSelectionTest, homeKey) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit) {"
      "  var has_opt_alter = arguments.length >= 3;"
      "  return doTest(sample, function(selection) {"
      "    selection.homeKey(unit);"
      "  });"
      "}");

  EXPECT_SCRIPT_EQ("|foo$bar$", "testIt('|foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo$bar$", "testIt('fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo$bar$", "testIt('foo|$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|bar$", "testIt('foo$|bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|bar$", "testIt('foo$b|ar$', Unit.LINE)");

  // Special handling of leading whitespaces.
  EXPECT_SCRIPT_EQ("  |foo$bar$", "testIt('|  foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |foo$bar$", "testIt(' | foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|  foo$bar$", "testIt('  |foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |foo$bar$", "testIt('  f|oo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |foo$bar$", "testIt('  fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |foo$bar$", "testIt('  foo|$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  foo$|bar$", "testIt('  foo$|bar$', Unit.LINE)");
}

TEST_F(TextSelectionTest, homeKeyExtend) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit) {"
      "  var has_opt_alter = arguments.length >= 3;"
      "  return doTest(sample, function(selection) {"
      "    selection.homeKey(unit, Alter.EXTEND);"
      "  });"
      "}");

  EXPECT_SCRIPT_EQ("|foo$bar$", "testIt('|foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|fo^o$bar$", "testIt('fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo^$bar$", "testIt('foo|$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|bar$", "testIt('foo$|bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|b^ar$", "testIt('foo$b|ar$', Unit.LINE)");

  // Special handling of leading whitespaces.
  EXPECT_SCRIPT_EQ("^  |foo$bar$", "testIt('|  foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ(" ^ |foo$bar$", "testIt(' | foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|  ^foo$bar$", "testIt('  |foo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |f^oo$bar$", "testIt('  f|oo$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |fo^o$bar$", "testIt('  fo|o$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  |foo^$bar$", "testIt('  foo|$bar$', Unit.LINE)");
  EXPECT_SCRIPT_EQ("  foo$|bar$", "testIt('  foo$|bar$', Unit.LINE)");
}

TEST_F(TextSelectionTest, startOf) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit, opt_alter) {"
      "  return doTest(sample, function(selection) {"
      "    selection.startOf(unit);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("|foo$bar$$baz", "testIt('|foo$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo$bar$$baz", "testIt('fo|o$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo$bar$$baz", "testIt('foo|$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|bar$$baz", "testIt('foo$|bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$|$baz", "testIt('foo$bar$|$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$$|baz", "testIt('foo$bar$$|baz', Unit.LINE)");

  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('foo| bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo |bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo b|ar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo ba|r  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo bar | baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  |baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  b|az', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  ba|z', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  baz|', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz ", "testIt('foo bar  baz |', Unit.WORD)");
}

TEST_F(TextSelectionTest, startOfExtend) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, unit, opt_alter) {"
      "  return doTest(sample, function(selection) {"
      "    selection.startOf(unit, Alter.EXTEND);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("|foo$bar$$baz", "testIt('|foo$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|fo^o$bar$$baz", "testIt('fo|o$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("|foo^$bar$$baz", "testIt('foo|$bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$|bar$$baz", "testIt('foo$|bar$$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$|$baz", "testIt('foo$bar$|$baz', Unit.LINE)");
  EXPECT_SCRIPT_EQ("foo$bar$$|baz", "testIt('foo$bar$$|baz', Unit.LINE)");

  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|f^oo bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|fo^o bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo^ bar  baz", "testIt('foo| bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo |bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |b^ar  baz", "testIt('foo b|ar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |ba^r  baz", "testIt('foo ba|r  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar^  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar ^ baz", "testIt('foo bar | baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  |baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |b^az", "testIt('foo bar  b|az', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |ba^z", "testIt('foo bar  ba|z', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz^", "testIt('foo bar  baz|', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz ^", "testIt('foo bar  baz |', Unit.WORD)");
}

TEST_F(TextSelectionTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('foo');"
      "var range = new TextRange(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection");
  EXPECT_SCRIPT_TRUE("sample instanceof TextSelection");
  EXPECT_SCRIPT_FALSE("sample instanceof TextRange");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_EQ("0", "sample.range.start");
  EXPECT_SCRIPT_EQ("0", "sample.range.end");
  EXPECT_SCRIPT_FALSE("sample.startIsActive");
  EXPECT_SCRIPT_VALID("sample.startIsActive = true;");
  EXPECT_SCRIPT_TRUE("sample.startIsActive");
}

}  // namespace dom
