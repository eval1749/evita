// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "joana/parser/parser.h"

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "joana/parser/simple_error_sink.h"
#include "joana/parser/simple_formatter.h"
#include "joana/public/ast/edit_context.h"
#include "joana/public/ast/edit_context_builder.h"
#include "joana/public/ast/error_codes.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/error_sink.h"
#include "joana/public/memory/zone.h"
#include "joana/public/source_code.h"
#include "joana/public/source_code_factory.h"
#include "joana/public/source_code_range.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {
namespace internal {

class ParserTest : public ::testing::Test {
 protected:
  ParserTest() = default;
  ~ParserTest() override = default;

  std::string Parse(base::StringPiece script_text);

 private:
  DISALLOW_COPY_AND_ASSIGN(ParserTest);
};

std::string ParserTest::Parse(base::StringPiece script_text) {
  Zone zone("ParserTest");
  SimpleErrorSink error_sink;
  ast::NodeFactory node_factory(&zone);
  const auto& context = ast::EditContext::Builder()
                            .SetErrorSink(&error_sink)
                            .SetNodeFactory(&node_factory)
                            .Build();
  SourceCode::Factory source_code_factory(&zone);
  const auto& script_text16 = base::UTF8ToUTF16(script_text);
  auto& source_code = source_code_factory.New(
      base::FilePath(), base::StringPiece16(script_text16));

  std::ostringstream ostream;
  Parser parser(context.get(), source_code.range());
  SimpleFormatter(&ostream).Format(parser.Run());
  return ostream.str();
}

#define TEST_PARSER(script_text)           \
  {                                        \
    auto* const source = script_text;      \
    EXPECT_EQ(source, Parse(script_text)); \
  }

TEST_F(ParserTest, EmptyStatement) {
  TEST_PARSER(";\n");
}

TEST_F(ParserTest, ExpressionStatement) {
  TEST_PARSER("false\n");
  TEST_PARSER("true\n");
}

}  // namespace internal
}  // namespace joana
