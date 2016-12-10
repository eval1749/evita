// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/statements.h"

#include "joana/public/ast/expressions.h"
#include "joana/public/ast/node_editor.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace ast {

//
// BlockStatement
//
BlockStatement::BlockStatement(const Punctuator& semi_colon)
    : Statement(semi_colon.range()) {
  DCHECK_EQ(semi_colon.kind(), PunctuatorKind::LeftBrace);
}

BlockStatement::~BlockStatement() = default;

//
// EmptyStatement
//
EmptyStatement::EmptyStatement(const Punctuator& semi_colon)
    : Statement(semi_colon.range()) {
  DCHECK_EQ(semi_colon.kind(), PunctuatorKind::SemiColon);
}

EmptyStatement::~EmptyStatement() = default;

//
// ExpressionStatement
//
ExpressionStatement::ExpressionStatement(const Expression& expression)
    : Statement(expression.range()) {
  NodeEditor().AppendChild(this, const_cast<Expression*>(&expression));
}

ExpressionStatement::~ExpressionStatement() = default;

const Expression& ExpressionStatement::expression() const {
  auto* const child = NodeTraversal::FirstChildOf(*this);
  DCHECK(child) << *this;
  return child->As<Expression>();
}

//
// InvalidStatement
//
InvalidStatement::InvalidStatement(const Node& node, int error_code)
    : Statement(node.range()), error_code_(error_code) {}

InvalidStatement::~InvalidStatement() = default;

//
// Statement
//
Statement::Statement(const SourceCodeRange& range) : ContainerNode(range) {}

Statement::~Statement() = default;

}  // namespace ast
}  // namespace joana
