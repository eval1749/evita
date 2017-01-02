// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "joana/parser/regexp/regexp_lexer.h"

#include "joana/parser/public/parser_options.h"
#include "joana/parser/public/parser_options_builder.h"
#include "joana/testing/lexer_test_base.h"
#include "joana/testing/print_as_tree.h"

namespace joana {
namespace parser {

//
// RegExpLexerTest
//
class RegExpLexerTest : public LexerTestBase {
 protected:
  RegExpLexerTest() = default;
  ~RegExpLexerTest() override = default;

  std::string Scan(base::StringPiece script_text, const ParserOptions& options);
  std::string Scan(base::StringPiece script_text);
  std::string ScanStrictly(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(RegExpLexerTest);
};

std::string RegExpLexerTest::Scan(base::StringPiece script_text,
                                  const ParserOptions& options) {
  PrepareSouceCode(script_text);
  RegExpLexer lexer(&context(), source_code().range(), options);
  std::ostringstream ostream;
  while (lexer.CanPeekToken())
    ostream << AsPrintableTree(lexer.ConsumeToken()) << std::endl;
  for (const auto& error : error_sink().errors())
    ostream << error << std::endl;
  return ostream.str();
}

std::string RegExpLexerTest::Scan(base::StringPiece script_text) {
  return Scan(script_text, ParserOptions());
}

std::string RegExpLexerTest::ScanStrictly(base::StringPiece script_text) {
  const auto& options =
      ParserOptions::Builder().set_enable_strict_regexp(true).Build();
  return Scan(script_text, options);
}

TEST_F(RegExpLexerTest, Empty) {
  EXPECT_EQ("", Scan(""));
}

TEST_F(RegExpLexerTest, Assertion) {
  EXPECT_EQ(
      "AssertionRegExp |^|\n"
      "AssertionRegExp |\\b|\n"
      "AssertionRegExp |\\B|\n"
      "AssertionRegExp |$|\n",
      Scan("^\\b\\B$"));
}

TEST_F(RegExpLexerTest, Atom) {
  EXPECT_EQ("LiteralRegExp |abc|\n", Scan("abc"));
  EXPECT_EQ("AnyCharRegExp |.|\n", Scan("."));
}

TEST_F(RegExpLexerTest, CharSet) {
  EXPECT_EQ("CharSetRegExp |[abc]|\n", Scan("[abc]"));
  EXPECT_EQ("ComplementCharSetRegExp |[^abc]|\n", Scan("[^abc]"));
}

TEST_F(RegExpLexerTest, Grouping) {
  EXPECT_EQ(
      "Punctuator |(|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n",
      Scan("(abc)"));

  EXPECT_EQ(
      "Punctuator |(?:|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n",
      Scan("(?:abc)"));

  EXPECT_EQ(
      "Punctuator |(?=|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n",
      Scan("(?=abc)"));

  EXPECT_EQ(
      "Punctuator |(?!|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n",
      Scan("(?!abc)"));
}

TEST_F(RegExpLexerTest, GroupingError) {
  EXPECT_EQ(
      "Punctuator |(?|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n",
      Scan("(?abc)"));

  EXPECT_EQ(
      "Punctuator |(?|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |)|\n"
      "REGEXP_ERROR_REGEXP_INVALID_GROUPING@0:2\n",
      ScanStrictly("(?abc)"));
}

TEST_F(RegExpLexerTest, Or) {
  EXPECT_EQ(
      "LiteralRegExp |abc|\n"
      "Punctuator |||\n"
      "LiteralRegExp |def|\n",
      Scan("abc|def"));

  EXPECT_EQ(
      "Punctuator |(|\n"
      "LiteralRegExp |abc|\n"
      "Punctuator |||\n"
      "LiteralRegExp |def|\n"
      "Punctuator |)|\n"
      "RegExpRepeat<*> |*|\n",
      Scan("(abc|def)*"));
}

TEST_F(RegExpLexerTest, RepeatError) {
  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "LiteralRegExp |{|\n"
      "LiteralRegExp |}|\n",
      Scan("a{}"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{0}> |{}|\n"
      "REGEXP_ERROR_REGEXP_INVALID_NUMBER@1:2\n",
      ScanStrictly("a{}"));
}

TEST_F(RegExpLexerTest, RepeatGreedy) {
  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<*> |*|\n",
      Scan("a*"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<?> |?|\n",
      Scan("a?"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<+> |+|\n",
      Scan("a+"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{1}> |{1}|\n",
      Scan("a{1}"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{2,}> |{2,}|\n",
      Scan("a{2,}"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{1,2}> |{1,2}|\n",
      Scan("a{1,2}"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<*> |{0,}|\n",
      Scan("a{0,}"))
      << "'*' == '{0,}'";

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<+> |{1,}|\n",
      Scan("a{1,}"))
      << "'+' == '{1,}'";

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<?> |{0,1}|\n",
      Scan("a{0,1}"))
      << "'?' == '{0,1}'";

  EXPECT_EQ(
      "LiteralRegExp |ab|\n"
      "LiteralRegExp |c|\n"
      "RegExpRepeat<*> |*|\n",
      Scan("abc*"));
}

TEST_F(RegExpLexerTest, RepeatLazy) {
  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<*?> |*?|\n",
      Scan("a*?"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<??> |??|\n",
      Scan("a??"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<+?> |+?|\n",
      Scan("a+?"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{1}?> |{1}?|\n",
      Scan("a{1}?"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{2,}?> |{2,}?|\n",
      Scan("a{2,}?"));

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<*?> |{0,}?|\n",
      Scan("a{0,}?"))
      << "'*' == '{0,}'";

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<+?> |{1,}?|\n",
      Scan("a{1,}?"))
      << "'+' == '{1,}'";

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<??> |{0,1}?|\n",
      Scan("a{0,1}?"))
      << "'?' == '{0,1}'";

  EXPECT_EQ(
      "LiteralRegExp |a|\n"
      "RegExpRepeat<{1,2}?> |{1,2}?|\n",
      Scan("a{1,2}?"));
}

}  // namespace parser
}  // namespace joana
