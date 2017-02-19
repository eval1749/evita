// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/parser/regexp/regexp_parser.h"

#include "joana/parser/public/parser_options.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace parser {

//
// RegExpParserTest
//
class RegExpParserTest : public LexerTestBase {
 protected:
  RegExpParserTest() = default;
  ~RegExpParserTest() override = default;

  std::string Parse(base::StringPiece script_text,
                    const ParserOptions& options);
  std::string Parse(base::StringPiece script_text);
  std::string ParseStrictly(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(RegExpParserTest);
};

std::string RegExpParserTest::Parse(base::StringPiece script_text,
                                    const ParserOptions& options) {
  PrepareSouceCode(script_text);
  RegExpParser parser(&context(), source_code().range(), options);
  const auto& node = parser.Parse();
  std::ostringstream ostream;
  ostream << AsPrintableTree(node) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string RegExpParserTest::Parse(base::StringPiece script_text) {
  return Parse(script_text, ParserOptions());
}

std::string RegExpParserTest::ParseStrictly(base::StringPiece script_text) {
  const auto& options =
      ParserOptions::Builder().set_enable_strict_regexp(true).Build();
  return Parse(script_text, options);
}

TEST_F(RegExpParserTest, Basic) {
  EXPECT_EQ(
      "SequenceRegExp\n"
      "+--LiteralRegExp |a|\n"
      "+--RepeatRegExp\n"
      "|  +--LiteralRegExp |b|\n"
      "|  +--RegExpRepeat<*> |*|\n"
      "+--LiteralRegExp |c/|\n",
      Parse("ab*c/"));

  EXPECT_EQ(
      "RepeatRegExp\n"
      "+--LiteralRegExp |a|\n"
      "+--RegExpRepeat<{2}> |{2}|\n",
      Parse("a{2}"));

  EXPECT_EQ(
      "RepeatRegExp\n"
      "+--CaptureRegExp\n"
      "|  +--OrRegExp\n"
      "|  |  +--LiteralRegExp |foo|\n"
      "|  |  +--LiteralRegExp |bar|\n"
      "+--RegExpRepeat<*> |*|\n",
      Parse("(foo|bar)*"));

  EXPECT_EQ("SequenceRegExp |)|\n", Parse("(?:)")) << "empty regexp";

  // non-strict regexp
  EXPECT_EQ(
      "CaptureRegExp\n"
      "+--OrRegExp\n"
      "|  +--LiteralRegExp |foo|\n"
      "|  +--EmptyRegExp |)|\n",
      Parse("(foo|)"))
      << "'|' is non-syntax char";

  EXPECT_EQ("LiteralRegExp |()foo|\n", Parse("()foo"))
      << "'()' is non-syntax char";

  EXPECT_EQ("LiteralRegExp |#{foo}|\n", Parse("#{foo}"))
      << "'{}' is non-syntax char";
}

TEST_F(RegExpParserTest, GroupingError) {
  EXPECT_EQ("LiteralRegExp |)foo|\n", Parse(")foo"))
      << "')' is not syntax character";

  EXPECT_EQ(
      "CaptureRegExp\n"
      "+--LiteralRegExp |foo|\n"
      "REGEXP_ERROR_REGEXP_EXPECT_RPAREN@0:4\n",
      Parse("(foo"))
      << "no right parenthesis";

  EXPECT_EQ(
      "SequenceRegExp\n"
      "+--InvalidRegExp |)|\n"
      "+--LiteralRegExp |foo|\n"
      "REGEXP_ERROR_REGEXP_EXPECT_PRIMARY@0:1\n",
      ParseStrictly(")foo"))
      << "')' is syntax character";

  EXPECT_EQ("LiteralRegExp |abc|\n", Parse("(?abc)"));

  EXPECT_EQ(
      "LiteralRegExp |abc|\n"
      "REGEXP_ERROR_REGEXP_INVALID_GROUPING@0:2\n",
      ParseStrictly("(?abc)"));
}

TEST_F(RegExpParserTest, Repeat) {
  EXPECT_EQ(
      "SequenceRegExp\n"
      "+--LiteralRegExp |ab|\n"
      "+--RepeatRegExp\n"
      "|  +--LiteralRegExp |c|\n"
      "|  +--RegExpRepeat<*> |*|\n",
      Parse("abc*"));

  EXPECT_EQ(
      "CaptureRegExp\n"
      "+--LiteralRegExp |*|\n",
      Parse("(*)"));

  EXPECT_EQ(
      "InvalidRegExp |*|\n"
      "REGEXP_ERROR_REGEXP_EXPECT_PRIMARY@1:2\n",
      ParseStrictly("(*)"))
      << "no regexp before '*'";
}

TEST_F(RegExpParserTest, Or) {
  EXPECT_EQ(
      "OrRegExp\n"
      "+--EmptyRegExp |||\n"
      "+--LiteralRegExp |foo|\n",
      ParseStrictly("|foo"))
      << "matches empty string or 'foo'";

  EXPECT_EQ(
      "OrRegExp\n"
      "+--EmptyRegExp |||\n"
      "+--LiteralRegExp |foo|\n",
      ParseStrictly("|foo"))
      << "matches 'foo' or empty string";

  EXPECT_EQ(
      "OrRegExp\n"
      "+--LiteralRegExp |foo|\n"
      "+--EmptyRegExp |)|\n",
      ParseStrictly("(foo|)"));

  EXPECT_EQ(
      "OrRegExp\n"
      "+--EmptyRegExp |||\n"
      "+--EmptyRegExp ||\n",
      ParseStrictly("|"))
      << "empty or empty";
}

TEST_F(RegExpParserTest, RealWorld) {
  EXPECT_EQ(
      "OrRegExp\n"
      "+--SequenceRegExp\n"
      "|  +--AssertionRegExp |^|\n"
      "|  +--LiteralRegExp |\\|\n"
      "|  +--RepeatRegExp\n"
      "|  |  +--LiteralRegExp |s|\n"  // This should be known charset
      "|  |  +--RegExpRepeat<*> |*|\n"
      "|  +--LiteralRegExp |<!|\n"
      "|  +--OrRegExp\n"
      "|  |  +--LiteralRegExp |\\[CDATA\\[|\n"
      "|  |  +--LiteralRegExp |--|\n"
      "+--SequenceRegExp\n"
      "|  +--OrRegExp\n"
      "|  |  +--LiteralRegExp |\\]\\]|\n"
      "|  |  +--LiteralRegExp |--|\n"
      "|  +--LiteralRegExp |>\\|\n"
      "|  +--RepeatRegExp\n"
      "|  |  +--LiteralRegExp |s|\n"  // This should be known charset
      "|  |  +--RegExpRepeat<*> |*|\n"
      "|  +--AssertionRegExp |$|\n",
      ParseStrictly("^\\s*<!(?:\\[CDATA\\[|--)|(?:\\]\\]|--)>\\s*$"));
}

}  // namespace parser
}  // namespace joana
