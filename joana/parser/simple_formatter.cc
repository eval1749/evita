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
      FormatWithIndent(child);
      *ostream_ << std::endl;
    }
    *ostream_ << '}';
    return true;
  }
  *ostream_ << std::endl;
  IndentScope scope(this);
  FormatWithIndent(statement);
  return false;
}

void SimpleFormatter::FormatWithIndent(const ast::Node& node) {
  auto* runner = &node;
  while (auto* labeled = runner->TryAs<ast::LabeledStatement>()) {
    OutputSpaces(indent_ * indent_size_ - 1);
    Format(labeled->label());
    *ostream_ << ':' << std::endl;
    runner = &labeled->statement();
  }
  OutputIndent();
  Format(*runner);
}

void SimpleFormatter::OutputIndent() {
  OutputSpaces(indent_ * indent_size_);
}

void SimpleFormatter::OutputSpaces(int amount) {
  for (auto counter = 0; counter < amount; ++counter)
    *ostream_ << ' ';
}

void SimpleFormatter::OutputUsingSoourceCode(const ast::Node& node) {
  *ostream_ << base::UTF16ToUTF8(node.range().GetString());
}

// NodeVisitor implementations
void SimpleFormatter::VisitComment(ast::Comment* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitInvalid(ast::Invalid* node) {
  *ostream_ << *node;
}

void SimpleFormatter::VisitPunctuator(ast::Punctuator* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitModule(ast::Module* node) {
  for (const auto& child : ast::NodeTraversal::ChildrenOf(*node)) {
    FormatWithIndent(child);
    *ostream_ << std::endl;
  }
}

void SimpleFormatter::VisitName(ast::Name* node) {
  OutputUsingSoourceCode(*node);
}

// Literals
void SimpleFormatter::VisitBooleanLiteral(ast::BooleanLiteral* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitNullLiteral(ast::NullLiteral* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitNumericLiteral(ast::NumericLiteral* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitStringLiteral(ast::StringLiteral* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitUndefinedLiteral(ast::UndefinedLiteral* node) {
  OutputUsingSoourceCode(*node);
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
  OutputUsingSoourceCode(node->literal());
}

void SimpleFormatter::VisitReferenceExpression(ast::ReferenceExpression* node) {
  OutputUsingSoourceCode(node->name());
}

// Statements
void SimpleFormatter::VisitBlockStatement(ast::BlockStatement* node) {
  *ostream_ << '{' << std::endl;
  {
    IndentScope scope(this);
    for (const auto& child : ast::NodeTraversal::ChildrenOf(*node)) {
      FormatWithIndent(child);
      *ostream_ << std::endl;
    }
  }
  OutputIndent();
  *ostream_ << '}';
}

void SimpleFormatter::VisitBreakStatement(ast::BreakStatement* node) {
  *ostream_ << "break";
  if (node->has_label())
    *ostream_ << ' ' << node->label();
  *ostream_ << ';';
}

void SimpleFormatter::VisitCaseClause(ast::CaseClause* node) {
  *ostream_ << "case ";
  Format(node->expression());
  *ostream_ << ": ";
  Format(node->statement());
}

void SimpleFormatter::VisitContinueStatement(ast::ContinueStatement* node) {
  *ostream_ << "continue";
  if (node->has_label())
    *ostream_ << ' ' << node->label();
  *ostream_ << ';';
}

void SimpleFormatter::VisitDoStatement(ast::DoStatement* node) {
  *ostream_ << "do";
  if (FormatChildStatement(node->statement()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "while (";
  Format(node->condition());
  *ostream_ << ");";
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

void SimpleFormatter::VisitLabeledStatement(ast::LabeledStatement* node) {
  Format(node->statement());
}

void SimpleFormatter::VisitSwitchStatement(ast::SwitchStatement* node) {
  *ostream_ << "switch (";
  Format(node->expression());
  *ostream_ << ") {" << std::endl;
  IndentScope switch_scope(this);
  for (const auto& clause : node->clauses()) {
    auto* runner = clause;
    for (;;) {
      if (auto* labeled = runner->TryAs<ast::LabeledStatement>()) {
        OutputIndent();
        Format(labeled->label());
        *ostream_ << ':' << std::endl;
        runner = &labeled->statement();
        continue;
      }

      if (auto* case_clause = runner->TryAs<ast::CaseClause>()) {
        OutputIndent();
        *ostream_ << "case ";
        Format(case_clause->expression());
        *ostream_ << ':' << std::endl;
        runner = &case_clause->statement();
        continue;
      }
      IndentScope scope(this);
      OutputIndent();
      Format(*runner);
      *ostream_ << std::endl;
      break;
    }
  }
  *ostream_ << '}';
}

void SimpleFormatter::VisitThrowStatement(ast::ThrowStatement* node) {
  *ostream_ << "throw ";
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitTryCatchStatement(ast::TryCatchStatement* node) {
  *ostream_ << "try";
  FormatChildStatement(node->block());
  *ostream_ << " catch (";
  Format(node->catch_name());
  *ostream_ << ')';
  FormatChildStatement(node->catch_block());
  if (!node->has_finally())
    return;
  *ostream_ << " finally";
  FormatChildStatement(node->finally_block());
}

void SimpleFormatter::VisitTryFinallyStatement(ast::TryFinallyStatement* node) {
  *ostream_ << "try";
  FormatChildStatement(node->block());
  *ostream_ << " finally";
  FormatChildStatement(node->finally_block());
}

void SimpleFormatter::VisitWhileStatement(ast::WhileStatement* node) {
  *ostream_ << "while (";
  Format(node->condition());
  *ostream_ << ')';
  if (FormatChildStatement(node->statement()))
    return;
  *ostream_ << ';';
}

}  // namespace internal
}  // namespace joana
