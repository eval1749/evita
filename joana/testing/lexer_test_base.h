// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_TESTING_LEXER_TEST_BASE_H_
#define JOANA_TESTING_LEXER_TEST_BASE_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "joana/ast/node_factory.h"
#include "joana/base/memory/zone.h"
#include "joana/base/source_code.h"
#include "joana/base/source_code_factory.h"
#include "joana/testing/simple_error_sink.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

class ParserContext;

//
// LexerTestBase
//
class LexerTestBase : public ::testing::Test {
 protected:
  LexerTestBase();
  ~LexerTestBase() override;

  ast::NodeFactory& node_factory() { return node_factory_; }
  ParserContext& context() { return *context_; }
  SimpleErrorSink& error_sink() { return error_sink_; }
  const SourceCode& source_code() const;

  void PrepareSouceCode(base::StringPiece script_text);

 private:
  SimpleErrorSink error_sink_;
  Zone zone_;
  ast::NodeFactory node_factory_;
  const std::unique_ptr<ParserContext> context_;
  const SourceCode* source_code_ = nullptr;
  SourceCode::Factory source_code_factory_;

  DISALLOW_COPY_AND_ASSIGN(LexerTestBase);
};

}  // namespace joana

#endif  // JOANA_TESTING_LEXER_TEST_BASE_H_
