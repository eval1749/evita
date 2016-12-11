// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "joana/parser/simple_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/public/ast/error_codes.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

namespace joana {
namespace internal {

//
// SimpleFormatter::IndentScope
//
class SimpleFormatter::IndentScope final {
 public:
  explicit IndentScope(SimpleFormatter* formatter) : formatter_(formatter) {
    ++formatter_->indent_;
  }

  ~IndentScope() {
    DCHECK_GE(formatter_->indent_, 1);
    --formatter_->indent_;
  }

 private:
  SimpleFormatter* const formatter_;

  DISALLOW_COPY_AND_ASSIGN(IndentScope);
};

//
// SimpleFormatter
//
SimpleFormatter::SimpleFormatter(std::ostream* ostream) : ostream_(ostream) {}

SimpleFormatter::~SimpleFormatter() = default;

void SimpleFormatter::Format(const ast::Node& node) {
  Visit(node);
}

bool SimpleFormatter::FormatChildStatement(const ast::Statement& statement) {
  if (auto* block = statement.TryAs<ast::BlockStatement>()) {
    *ostream_ << " {" << std::endl;
    IndentScope scope(this);
    for (const auto& child : ast::NodeTraversal::ChildrenOf(*block)) {
      OutputIndent();
      Format(child);
      *ostream_ << std::endl;
    }
    *ostream_ << '}';
    return true;
  }
  *ostream_ << std::endl;
  IndentScope scope(this);
  OutputIndent();
  Format(statement);
  return false;
}

void SimpleFormatter::OutputAsSourceCode(const ast::Node& node) {
  *ostream_ << base::UTF16ToUTF8(node.range().GetString());
}

void SimpleFormatter::OutputIndent() {
  for (auto counter = 0; counter < indent_; ++counter)
    *ostream_ << "  ";
}

// NodeVisitor implementations
void SimpleFormatter::VisitComment(ast::Comment* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitInvalid(ast::Invalid* node) {
  *ostream_ << *node;
}

void SimpleFormatter::VisitPunctuator(ast::Punctuator* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitModule(ast::Module* node) {
  for (const auto& child : ast::NodeTraversal::ChildrenOf(*node)) {
    Format(child);
    *ostream_ << std::endl;
  }
}

void SimpleFormatter::VisitName(ast::Name* node) {
  OutputAsSourceCode(*node);
}

// Literals
void SimpleFormatter::VisitBooleanLiteral(ast::BooleanLiteral* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitNullLiteral(ast::NullLiteral* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitNumericLiteral(ast::NumericLiteral* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitStringLiteral(ast::StringLiteral* node) {
  OutputAsSourceCode(*node);
}

void SimpleFormatter::VisitUndefinedLiteral(ast::UndefinedLiteral* node) {
  OutputAsSourceCode(*node);
}

// Expressions
void SimpleFormatter::VisitInvalidExpression(ast::InvalidExpression* node) {
  const auto string = ast::ErrorStringOf(node->error_code());
  if (string.empty())
    *ostream_ << node->error_code();
  else
    *ostream_ << string;
}

void SimpleFormatter::VisitLiteralExpression(ast::LiteralExpression* node) {
  OutputAsSourceCode(node->literal());
}

void SimpleFormatter::VisitReferenceExpression(ast::ReferenceExpression* node) {
  OutputAsSourceCode(node->name());
}

// Statements
void SimpleFormatter::VisitBlockStatement(ast::BlockStatement* node) {
  OutputIndent();
  *ostream_ << '{' << std::endl;
  ++indent_;
  for (const auto& child : ast::NodeTraversal::ChildrenOf(*node)) {
    OutputIndent();
    Format(child);
    *ostream_ << std::endl;
  }
  --indent_;
  OutputIndent();
  *ostream_ << '}';
}

void SimpleFormatter::VisitEmptyStatement(ast::EmptyStatement* node) {
  *ostream_ << ';';
}

void SimpleFormatter::VisitExpressionStatement(ast::ExpressionStatement* node) {
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitIfStatement(ast::IfStatement* node) {
  *ostream_ << "if (";
  Format(node->condition());
  *ostream_ << ")";
  if (!node->has_else()) {
    FormatChildStatement(node->then_clause());
    return;
  }

  if (FormatChildStatement(node->then_clause()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "else";
  FormatChildStatement(node->else_clause());
}

void SimpleFormatter::VisitInvalidStatement(ast::InvalidStatement* node) {
  const auto string = ast::ErrorStringOf(node->error_code());
  if (string.empty())
    *ostream_ << node->error_code();
  else
    *ostream_ << string;
}

}  // namespace internal
}  // namespace joana
