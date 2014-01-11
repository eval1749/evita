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
    CHECK(RunScript(std::string() +
        "var doc = new Document('sample');"
        "var r = new Range(doc);" +
        "r.text = '" + sample + "';"));
  }

  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

static const char* kInvalidPosition = "Error: Invalid position.";

TEST_F(RangeTest, Constructor) {
  EXPECT_VALID_SCRIPT("var doc1 = new Document('range')");
  EXPECT_VALID_SCRIPT("var range1 = new Range(doc1)");
  EXPECT_VALID_SCRIPT("var range2 = new Range(doc1, 0)");
  EXPECT_VALID_SCRIPT("var range3 = new Range(doc1, 0, 0)");
  EXPECT_SCRIPT_TRUE("range1.document === doc1");
  EXPECT_SCRIPT_TRUE("range2.document === doc1");
  EXPECT_SCRIPT_TRUE("range3.document === doc1");
}

TEST_F(RangeTest, capitalize) {
  EXPECT_VALID_SCRIPT(
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

TEST_F(RangeTest, collapseTo) {
  EXPECT_VALID_SCRIPT(
    "var doc = new Document('endOf');"
    "var range = new Range(doc);"
    "range.text = 'foo';"
    "range.collapseTo(1);");
  EXPECT_SCRIPT_EQ("1", "range.start");
  EXPECT_SCRIPT_EQ("1", "range.end");
}

TEST_F(RangeTest, endOf) {
  EXPECT_VALID_SCRIPT(
    "var doc = new Document('endOf');"
    "var range = new Range(doc);"
                 //012345678901
    "range.text = 'foo bar  baz';"
    "function test(x) {"
    "  range.start = x;"
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

TEST_F(RangeTest, move) {
  EXPECT_VALID_SCRIPT(
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

  EXPECT_SCRIPT_EQ("36 36", "test(2, 2, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("36 36", "test(2, 4, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 0", "test(18, 18, Unit.PARAGRAPH, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 0", "test(18, 20, Unit.PARAGRAPH, -1)");

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
  EXPECT_VALID_SCRIPT(
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

  EXPECT_SCRIPT_EQ("2 36", "test(2, 2, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("2 36", "test(2, 4, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 2, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 0", "test(18, 18, Unit.PARAGRAPH, -1)");
  EXPECT_SCRIPT_EQ("0 0", "test(2, 4, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 0", "test(18, 20, Unit.PARAGRAPH, -1)");

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

TEST_F(RangeTest, moveStart) {
  EXPECT_VALID_SCRIPT(
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

  EXPECT_SCRIPT_EQ("36 36", "test(2, 2, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("36 36", "test(2, 4, Unit.PARAGRAPH, 2)");
  EXPECT_SCRIPT_EQ("0 2", "test(2, 2, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 18", "test(18, 18, Unit.PARAGRAPH, -1)");
  EXPECT_SCRIPT_EQ("0 4", "test(2, 4, Unit.PARAGRAPH, -2)");
  EXPECT_SCRIPT_EQ("0 20", "test(18, 20, Unit.PARAGRAPH, -1)");

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

TEST_F(RangeTest, set_start_end) {
  EXPECT_VALID_SCRIPT(
      "var doc1 = new Document('text');"
      "var range1 = new Range(doc1);"
      "range1.text = 'abcdefghijkl';"
      "range1.start = 5;");
  EXPECT_SCRIPT_EQ("5", "range1.start");
  EXPECT_SCRIPT_EQ("1 5", "range1.end = 1; range1.start + ' ' + range1.end");
  EXPECT_SCRIPT_EQ(kInvalidPosition, "range1.start = -1");
  EXPECT_SCRIPT_EQ(kInvalidPosition, "range1.start = 100");
  EXPECT_SCRIPT_EQ(kInvalidPosition, "range1.end = -1");
  EXPECT_SCRIPT_EQ(kInvalidPosition, "range1.end = 100");
}

TEST_F(RangeTest, startOf) {
  EXPECT_VALID_SCRIPT(
      "var doc = new Document('startOf');"
      "var range = new Range(doc);"
                   //01234567890
      "range.text = 'foo bar baz';"
      "function test(x) {"
      "  range.end = x;"
      "  range.start = x;"
      "  range.startOf(Unit.WORD);"
      "  return range.start;"
      "}");
  EXPECT_SCRIPT_EQ("0", "test(2)");
  EXPECT_SCRIPT_EQ("4", "test(5)");
}

TEST_F(RangeTest, text) {
  EXPECT_VALID_SCRIPT(
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
