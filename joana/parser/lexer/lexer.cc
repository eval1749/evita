// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/parser/lexer/lexer.h"

#include "joana/parser/lexer/character_reader.h"
#include "joana/public/ast/comment.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/source_code_range.h"

namespace joana {
namespace internal {

Lexer::Lexer(ast::NodeFactory* node_factory,
             ErrorSink* error_sink,
             const SourceCodeRange& range)
    : error_sink_(error_sink),
      node_factory_(node_factory),
      range_(range),
      reader_(new CharacterReader(range)) {}

Lexer::~Lexer() = default;

const SourceCode& Lexer::source_code() const {
  return reader_->source_code();
}

bool Lexer::HasMore() const {
  return reader_->HasMore();
}

const ast::Node& Lexer::NextToken() {
  DCHECK(HasMore());
  const auto start = reader_->location();
  reader_->Read();
  return node_factory_->NewComment(
      SourceCodeRange(source_code(), start, reader_->location()));
}

}  // namespace internal
}  // namespace joana
