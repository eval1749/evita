// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "base/strings/stringprintf.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class RangeTest : public dom::AbstractDomTest {
  protected: RangeTest() {
  }
  public: virtual ~RangeTest() {
  }

  protected: void PopulateSample(const char* sample) {
    EXPECT_SCRIPT_VALID(base::StringPrintf(
        "var doc = new Document('sample');"
        "var r = new Range(doc);"
        "r.text = '%s';",
        sample));
  }

  protected: virtual void SetUp() override {
    dom::AbstractDomTest::SetUp();
    EXPECT_SCRIPT_VALID(
      "var doc = new Document('sample');"
      "var range = new Range(doc);");
    EXPECT_SCRIPT_VALID(
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
      "  sampler(range);"
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
  EXPECT_SCRIPT_VALID(
      "function doTest2(sample, sampler) {"
      "  range.collapseTo(0);"
      "  range.end = doc.length;"
      "  var source = sample;"
      "  source = source.replace(/\\\\n/g, '\\n');"
      "  source = source.replace(/\\\\t/g, '\\t');"
      "  var caret = source.indexOf('|');"
      "  source = source.replace(/[|]/g, '');"
      "  range.text = source;"
      "  range.collapseTo(caret);"
      "  sampler(range);"
      "  caret = range.start;"
      "  range.start = 0;"
      "  range.end = range.document.length;"
      "  var result = range.text;"
      "  result = result.substr(0, caret) + '|' + result.substr(caret);"
      "  result = result.replace(/\\n/g, '\\\\n');"
      "  result = result.replace(/\\t/g, '\\\\t');"
      "  return result;"
      "}");
  }

  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

TEST_F(RangeTest, analyzeCase) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample) {"
      "  var doc = new Document('analyzeCase');"
      "  var range = new Range(doc);"
      "  range.text = sample;"
      "  return range.analyzeCase();"
      "}");
  EXPECT_SCRIPT_EQ("CAPITALIZED_TEXT", "testIt('Foo::bar(baz)')");
  EXPECT_SCRIPT_EQ("CAPITALIZED_WORDS", "testIt('Foo::Bar::Baz')");
  EXPECT_SCRIPT_EQ("LOWER", "testIt('abcd')");
  EXPECT_SCRIPT_EQ("MIXED", "testIt('0x25AD')");
  EXPECT_SCRIPT_EQ("MIXED", "testIt('MiXed')");
  EXPECT_SCRIPT_EQ("UPPER", "testIt('ABCD')");
}

TEST_F(RangeTest, ctor) {
  EXPECT_SCRIPT_VALID("var doc1 = new Document('range')");
  EXPECT_SCRIPT_VALID("var range1 = new Range(doc1)");
  EXPECT_SCRIPT_TRUE("range1.document === doc1");
  EXPECT_SCRIPT_VALID("range1.text = '0123456';");
  EXPECT_SCRIPT_VALID("range1.start = 1;");

  EXPECT_SCRIPT_VALID("var range2 = new Range(doc1, 2)");
  EXPECT_SCRIPT_TRUE("range2.document === doc1");
  EXPECT_SCRIPT_EQ("2", "range2.start");
  EXPECT_SCRIPT_EQ("2", "range2.end");

  EXPECT_SCRIPT_VALID("var range3 = new Range(doc1, 3, 4)");
  EXPECT_SCRIPT_TRUE("range3.document === doc1");
  EXPECT_SCRIPT_EQ("3", "range3.start");
  EXPECT_SCRIPT_EQ("4", "range3.end");

  EXPECT_SCRIPT_VALID("var range4 = new Range(range1)");
  EXPECT_SCRIPT_TRUE("range4.document === doc1");
  EXPECT_SCRIPT_EQ("1", "range4.start");
  EXPECT_SCRIPT_EQ("7", "range4.end");


  EXPECT_SCRIPT_VALID("var range5 = new Range(range1, 2)");
  EXPECT_SCRIPT_TRUE("range5.document === doc1");
  EXPECT_SCRIPT_EQ("2", "range5.start");
  EXPECT_SCRIPT_EQ("2", "range5.end");

  EXPECT_SCRIPT_VALID("var range6 = new Range(range1, 2, 4)");
  EXPECT_SCRIPT_TRUE("range6.document === doc1");
  EXPECT_SCRIPT_EQ("2", "range6.start");
  EXPECT_SCRIPT_EQ("4", "range6.end");
}

TEST_F(RangeTest, capitalize) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('endOf');"
    "var range = new Range(doc);"
    "function test(sample) {"
    "  range.text = sample;"
    "  range.capitalize();"
    "  return range.text;"
    "}");
  EXPECT_SCRIPT_EQ("Foo bar", "test('FOO BAR')");
  EXPECT_SCRIPT_EQ("Foo bar", "test('foo bar')");
  EXPECT_SCRIPT_EQ(" Foo bar", "test(' foo Bar')");
}

TEST_F(RangeTest, collapsed) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('collapsed');"
    "var range = new Range(doc);");
  EXPECT_SCRIPT_TRUE("range.collapsed");
  EXPECT_SCRIPT_TRUE("range.start == range.end");
  EXPECT_SCRIPT_VALID("range.text = 'foo';");
  EXPECT_SCRIPT_FALSE("range.collapsed");
  EXPECT_SCRIPT_TRUE("range.start != range.end");
}

TEST_F(RangeTest, collapseTo) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('endOf');"
    "var range = new Range(doc);"
    "range.text = 'foo';"
    "range.collapseTo(1);");
  EXPECT_SCRIPT_EQ("1", "range.start");
  EXPECT_SCRIPT_EQ("1", "range.end");
}

// Note: Since RangeTest.Clipboard uses system clipboard, this test is
// flaky. User interactio may change contents of clipboard during test
// execution.
TEST_F(RangeTest, Clipboard) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('Clipboard');"
    "var range = new Range(doc);"
    "range.text = 'foo bar';"
    "range.collapseTo(0);"
    "range.moveEnd(Unit.CHARACTER, 3);"
    "range.copy();"
    "range.collapseTo(3);"
    "range.paste();"
    "range.start = 0;"
    "range.end = doc.length;");
  EXPECT_SCRIPT_EQ("foofoo bar", "range.text");
}

TEST_F(RangeTest, delete) {
  EXPECT_SCRIPT_VALID(
    "function testIt(sample, unit, count) {"
    "  var doc = new Document('delete');"
    "  var range = new Range(doc);"
    "  range.text = sample.replace(/[|]/g, '');"
    "  range.collapseTo(sample.indexOf('|'));"
    "  var end = sample.lastIndexOf('|');"
    "  if (range.start != end)"
    "    --end;"
    "  range.end = end;"
    "  if (arguments.length >= 3)"
    "    range.delete(unit, count);"
    "  else"
    "    range.delete(unit);"
    "  var caret = range.start;"
    "  range.start = 0;"
    "  range.end = doc.length;"
    "  var result = range.text;"
    "  result = result.substr(0, caret) + '|' + result.substr(caret);"
    "  return result;"
    "}");
  // Backspace
  EXPECT_SCRIPT_EQ("|abcd", "testIt('|abcd', Unit.CHARACTER, -1)");
  EXPECT_SCRIPT_EQ("a|cd", "testIt('ab|cd', Unit.CHARACTER, -1)");
  EXPECT_SCRIPT_EQ("abc|", "testIt('abcd|', Unit.CHARACTER, -1)");

  EXPECT_SCRIPT_EQ("|foo bar", "testIt('|foo bar', Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("|o bar", "testIt('fo|o bar', Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("| bar", "testIt('foo| bar', Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("foo|bar", "testIt('foo |bar', Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("foo |", "testIt('foo bar|', Unit.WORD, -1)");

  // Delete
  EXPECT_SCRIPT_EQ("|bcd", "testIt('|abcd', Unit.CHARACTER, 1)");
  EXPECT_SCRIPT_EQ("ab|d", "testIt('ab|cd', Unit.CHARACTER, 1)");
  EXPECT_SCRIPT_EQ("abcd|", "testIt('abcd|', Unit.CHARACTER, 1)");

  EXPECT_SCRIPT_EQ("|bar", "testIt('|foo bar', Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("fo|bar", "testIt('fo|o bar', Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("foo|bar", "testIt('foo| bar', Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("foo |", "testIt('foo |bar', Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("foo bar|", "testIt('foo bar|', Unit.WORD, 1)");

  // Range
  EXPECT_SCRIPT_EQ("foo | baz", "testIt('foo |bar| baz', Unit.CHARACTER)");
}

TEST_F(RangeTest,  endOf) {
  EXPECT_SCRIPT_VALID(
    "function testIt(sample, unit) {"
    "  return doTest2(sample, function(range) {"
    "   range.endOf(unit);"
    "  });"
    "}");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo| bar  baz", "testIt('foo| bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo |bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz|", "testIt('foo bar | baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz|", "testIt('foo bar  baz|', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz |", "testIt('foo bar  baz |', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  baz  |", "testIt('foo bar  baz | ', Unit.WORD)");
}

TEST_F(RangeTest, insertBefore) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('insertBefore');"
    "var range = new Range(doc);"
    "range.text = 'foo';"
    "range.insertBefore('bar');"
    "var range2 = new Range(doc);"
    "range2.end = doc.length;");
  EXPECT_SCRIPT_EQ("3", "range.start");
  EXPECT_SCRIPT_EQ("6", "range.end");
  EXPECT_SCRIPT_EQ("barfoo", "range2.text");
}

TEST_F(RangeTest, move) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('move');"
      "var range = new Range(doc);"
                   //0123456789012345  678901234  56789012345
      "range.text = 'this is a word.\\nline two\\nline three\\n';"
      "function test(start, end, unit, count) {"
      "  range.collapseTo(start);"
      "  range.end = end;"
      "  range.move(unit, count);"
      "  return range.start + ' ' + range.end;"
      "}");
  EXPECT_SCRIPT_EQ("4 4", "test(2, 2, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("6 6", "test(2, 4, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.CHARACTER, -2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.CHARACTER, -2)");

  EXPECT_SCRIPT_EQ("25 25", "test(2, 2, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("25 25", "test(2, 4, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 16", "test(18, 18, Unit.LINE, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 16", "test(18, 20, Unit.LINE, -1)");

  // Forward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("8 8", "test(5, 5, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("8 8", "test(2, 5, Unit.WORD, 1)");

  // Backward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("4 4", "test(5, 5, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("4 4", "test(5, 7, Unit.WORD, -1)");

  // Forkward th|is is a word. => this |is a word.
  EXPECT_SCRIPT_EQ("5 5", "test(2, 2, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("5 5", "test(0, 2, Unit.WORD, 1)");

  // Backward th|is is a word. => |this is a word.
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.WORD, -1)");

  // Forward this is| a word. =>  this is |a word.
  EXPECT_SCRIPT_EQ("8 8", "test(7, 7, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("8 8", "test(5, 7, Unit.WORD, 1)");

  // Backward this is| a word. => this |is a word.
  EXPECT_SCRIPT_EQ("5 5", "test(7, 7, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("5 5", "test(7, 8, Unit.WORD, -1)");
}


TEST_F(RangeTest, moveEnd) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('move');"
      "var range = new Range(doc);"
                   //0123456789012345  678901234  56789012345
      "range.text = 'this is a word.\\nline two\\nline three\\n';"
      "function test(start, end, unit, count) {"
      "  range.collapseTo(start);"
      "  range.end = end;"
      "  range.moveEnd(unit, count);"
      "  return range.start + ' ' + range.end;"
      "}");
  EXPECT_SCRIPT_EQ("2 4", "test(2, 2, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("2 6", "test(2, 4, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.CHARACTER, -2)");
  EXPECT_SCRIPT_EQ("2 2", "test(2, 4, Unit.CHARACTER, -2)");

  EXPECT_SCRIPT_EQ("2 25", "test(2, 2, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("2 25", "test(2, 4, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 16", "test(18, 18, Unit.LINE, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 16", "test(18, 20, Unit.LINE, -1)");

  // Forward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("5 8", "test(5, 5, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("2 8", "test(2, 5, Unit.WORD, 1)");

  // Backward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("4 4", "test(5, 5, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("3 4", "test(3, 5, Unit.WORD, -1)");

  // Forkward th|is is a word. => this |is a word.
  EXPECT_SCRIPT_EQ("2 5", "test(2, 2, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("0 5", "test(0, 2, Unit.WORD, 1)");

  // Backward th|is is a word. => |this is a word.
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.WORD, -1)");

  // Forward this is| a word. =>  this is |a word.
  EXPECT_SCRIPT_EQ("7 8", "test(7, 7, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("5 8", "test(5, 7, Unit.WORD, 1)");

  // Backward this is| a word. => this |is a word.
  EXPECT_SCRIPT_EQ("5 5", "test(7, 7, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("5 5", "test(6, 7, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("4 5", "test(4, 7, Unit.WORD, -1)");
}

TEST_F(RangeTest, moveEndWhile) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, charset, opt_count) {"
      "  var has_count = arguments.length >= 3;"
      "  return doTest(sample, function(range) {"
      "    if (has_count)"
      "      range.moveEndWhile(charset, opt_count);"
      "    else"
      "      range.moveEndWhile(charset);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('|...abc', '.')");
  EXPECT_SCRIPT_EQ("|.|..abc", "testIt('|...abc', '.', 1)");
  EXPECT_SCRIPT_EQ("|..|.abc", "testIt('|...abc', '.', 2)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('|...abc', '.', 3)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('|...abc', '.', 4)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('|...abc', '.', Count.FORWARD)");

  EXPECT_SCRIPT_EQ("..|.abc", "testIt('...|abc', '.', -1)");
  EXPECT_SCRIPT_EQ(".|..abc", "testIt('...|abc', '.', -2)");
  EXPECT_SCRIPT_EQ("|...abc", "testIt('...|abc', '.', -3)");
  EXPECT_SCRIPT_EQ("|...abc", "testIt('...|abc', '.', -4)");
  EXPECT_SCRIPT_EQ("|...abc", "testIt('...|abc', '.', Count.BACKWARD)");

  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', '.', 0)");

  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', 'x', 1)");
  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', 'x', -1)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('...|abc', 'x', 1)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('...|abc', 'x', -1)");
}

TEST_F(RangeTest, moveStart) {
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('move');"
      "var range = new Range(doc);"
                   //0123456789012345  678901234  56789012345
      "range.text = 'this is a word.\\nline two\\nline three\\n';"
      "function test(start, end, unit, count) {"
      "  range.collapseTo(start);"
      "  range.end = end;"
      "  range.moveStart(unit, count);"
      "  return range.start + ' ' + range.end;"
      "}");
  EXPECT_SCRIPT_EQ("4 4", "test(2, 2, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("4 4", "test(2, 4, Unit.CHARACTER, 2)");
  EXPECT_SCRIPT_EQ("0 2", "test(2, 2, Unit.CHARACTER, -2)");
  EXPECT_SCRIPT_EQ("0 4", "test(2, 4, Unit.CHARACTER, -2)");

  EXPECT_SCRIPT_EQ("25 25", "test(2, 2, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("25 25", "test(2, 4, Unit.LINE, 2)");
  EXPECT_SCRIPT_EQ("0 2", "test(2, 2, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 18", "test(18, 18, Unit.LINE, -1)");
  EXPECT_SCRIPT_EQ("0 4", "test(2, 4, Unit.LINE, -2)");
  EXPECT_SCRIPT_EQ("16 20", "test(18, 20, Unit.LINE, -1)");

  // Forward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("8 8", "test(5, 5, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("8 8", "test(5, 7, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("8 9", "test(5, 9, Unit.WORD, 1)");

  // Backward this |is a word.  => this is |a word.
  EXPECT_SCRIPT_EQ("4 5", "test(5, 5, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("4 7", "test(5, 7, Unit.WORD, -1)");

  // Forkward th|is is a word. => this |is a word.
  EXPECT_SCRIPT_EQ("5 5", "test(2, 2, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("5 5", "test(2, 4, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("5 7", "test(2, 7, Unit.WORD, 1)");

  // Backward th|is is a word. => |this is a word.
  EXPECT_SCRIPT_EQ("0 2", "test(2, 2, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("0 4", "test(2, 4, Unit.WORD, -1)");

  // Forward this is| a word. =>  this is |a word.
  EXPECT_SCRIPT_EQ("8 8", "test(7, 7, Unit.WORD, 1)");
  EXPECT_SCRIPT_EQ("8 9", "test(7, 9, Unit.WORD, 1)");

  // Backward this is| a word. => this |is a word.
  EXPECT_SCRIPT_EQ("5 7", "test(7, 7, Unit.WORD, -1)");
  EXPECT_SCRIPT_EQ("5 8", "test(7, 8, Unit.WORD, -1)");
}


TEST_F(RangeTest, moveStartWhile) {
  EXPECT_SCRIPT_VALID(
      "function testIt(sample, charset, opt_count) {"
      "  var has_count = arguments.length >= 3;"
      "  return doTest(sample, function(range) {"
      "    if (has_count)"
      "      range.moveStartWhile(charset, opt_count);"
      "    else"
      "      range.moveStartWhile(charset);"
      "  });"
      "}");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('|...abc', '.')");
  EXPECT_SCRIPT_EQ(".|..abc", "testIt('|...abc', '.', 1)");
  EXPECT_SCRIPT_EQ("..|.abc", "testIt('|...abc', '.', 2)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('|...abc', '.', 3)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('|...abc', '.', 4)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('|...abc', '.', Count.FORWARD)");

  EXPECT_SCRIPT_EQ("..|.|abc", "testIt('...|abc', '.', -1)");
  EXPECT_SCRIPT_EQ(".|..|abc", "testIt('...|abc', '.', -2)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('...|abc', '.', -3)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('...|abc', '.', -4)");
  EXPECT_SCRIPT_EQ("|...|abc", "testIt('...|abc', '.', Count.BACKWARD)");

  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', '.', 0)");

  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', 'x', 1)");
  EXPECT_SCRIPT_EQ("|...abc", "testIt('|...abc', 'x', -1)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('...|abc', 'x', 1)");
  EXPECT_SCRIPT_EQ("...|abc", "testIt('...|abc', 'x', -1)");
}

TEST_F(RangeTest, setSpelling) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = new Document('spelling');"
      "var range1 = new Range(doc1);"
      "range1.text = 'foo bar baz';"
      "range1.start = 4;"
      "range1.end = 7;"
      "range1.setSpelling(2);");
  EXPECT_SCRIPT_EQ("0", "doc1.spellingAt(0)");
  EXPECT_SCRIPT_EQ("0", "doc1.spellingAt(3)");
  EXPECT_SCRIPT_EQ("2", "doc1.spellingAt(4)");
  EXPECT_SCRIPT_EQ("2", "doc1.spellingAt(5)");
  EXPECT_SCRIPT_EQ("2", "doc1.spellingAt(6)");
  EXPECT_SCRIPT_EQ("0", "doc1.spellingAt(7)");
  EXPECT_SCRIPT_EQ("0", "doc1.spellingAt(8)");
}

TEST_F(RangeTest, set_start_end) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = new Document('text');"
      "var range1 = new Range(doc1);"
      "range1.text = 'abcdefghijkl';"
      "range1.start = 5;");
  EXPECT_SCRIPT_EQ("5", "range1.start");
  EXPECT_SCRIPT_EQ("1 5", "range1.end = 1; range1.start + ' ' + range1.end");
  EXPECT_SCRIPT_EQ("RangeError: Invalid position -1, valid range is [0, 12]",
                   "range1.start = -1");
  EXPECT_SCRIPT_EQ("RangeError: Invalid position 100, valid range is [0, 12]",
                   "range1.start = 100");
  EXPECT_SCRIPT_EQ("RangeError: Invalid position -1, valid range is [0, 12]",
                   "range1.end = -1");
  EXPECT_SCRIPT_EQ("RangeError: Invalid position 100, valid range is [0, 12]",
                   "range1.end = 100");
}

TEST_F(RangeTest, startOf) {
  EXPECT_SCRIPT_VALID(
    "function testIt(sample, unit) {"
    "  return doTest2(sample, function(range) {"
    "   range.startOf(unit);"
    "  });"
    "}");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('|foo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('f|oo bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('fo|o bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("|foo bar  baz", "testIt('foo| bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo |bar  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo bar|  baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo |bar  baz", "testIt('foo bar | baz', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz", "testIt('foo bar  baz|', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar  |baz ", "testIt('foo bar  baz |', Unit.WORD)");

  EXPECT_SCRIPT_EQ("|++foo", "testIt('+|+foo', Unit.WORD)");
  EXPECT_SCRIPT_EQ("++|foo", "testIt('++|foo', Unit.WORD)");

  EXPECT_SCRIPT_EQ("foo bar|++", "testIt('foo bar|++', Unit.WORD)");
  EXPECT_SCRIPT_EQ("foo bar|++", "testIt('foo bar+|+', Unit.WORD)");
}

TEST_F(RangeTest, text) {
  EXPECT_SCRIPT_VALID(
      "var doc1 = new Document('text');"
      "var range1 = new Range(doc1);"
      "range1.text = 'abcdefghijkl';"
      "var range2 = new Range(doc1, 3, 6);");
  EXPECT_SCRIPT_EQ("def", "range2.text");
}

TEST_F(RangeTest, toLocalLocaleLowerCase) {
  PopulateSample("ABCDEFGHIJ");
  EXPECT_SCRIPT_EQ("abcdefghij", "r.toLocaleLowerCase(); r.text");
}

TEST_F(RangeTest, toLocalLowerCase) {
  PopulateSample("ABCDEFGHIJ");
  EXPECT_SCRIPT_EQ("abcdefghij", "r.toLowerCase(); r.text");
}

TEST_F(RangeTest, toLocalLocaleUpperCase) {
  PopulateSample("abcdefghij");
  EXPECT_SCRIPT_EQ("ABCDEFGHIJ", "r.toLocaleUpperCase(); r.text");
}

TEST_F(RangeTest, toLocalUpperCase) {
  PopulateSample("abcdefghij");
  EXPECT_SCRIPT_EQ("ABCDEFGHIJ", "r.toUpperCase(); r.text");
}

}  // namespace
