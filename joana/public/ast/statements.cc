// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/statements.h"

#include "joana/public/ast/punctuator.h"

namespace joana {
namespace ast {

//
// EmptyStatement
//
EmptyStatement::EmptyStatement(const Punctuator& semi_colon)
    : Statement(semi_colon.range()) {
  DCHECK_EQ(semi_colon.kind(), PunctuatorKind::SemiColon);
}

EmptyStatement::~EmptyStatement() = default;

//
// Statement
//
Statement::Statement(const SourceCodeRange& range) : ContainerNode(range) {}

Statement::~Statement() = default;

}  // namespace ast
}  // namespace joana
