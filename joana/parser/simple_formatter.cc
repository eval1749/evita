// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "joana/parser/simple_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/public/ast/comment.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/invalid.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/name.h"
#include "joana/public/ast/node_traversal.h"
#include "joana/public/ast/punctuator.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/template.h"

namespace joana {
namespace internal {

SimpleFormatter::SimpleFormatter(std::ostream* ostream) : ostream_(ostream) {}

SimpleFormatter::~SimpleFormatter() = default;

void SimpleFormatter::Format(const ast::Node& node) {
  Visit(node);
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
  *ostream_ << "INVALID(" << node->error_code() << ')';
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

void SimpleFormatter::VisitTemplate(ast::Template* node) {
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
  *ostream_ << "ERROR(" << node->error_code() << ')';
}

void SimpleFormatter::VisitLiteralExpression(ast::LiteralExpression* node) {
  OutputAsSourceCode(node->literal());
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
  *ostream_ << '}' << std::endl;
}

void SimpleFormatter::VisitEmptyStatement(ast::EmptyStatement* node) {
  *ostream_ << ';';
}

void SimpleFormatter::VisitExpressionStatement(ast::ExpressionStatement* node) {
  Format(node->expression());
}

void SimpleFormatter::VisitInvalidStatement(ast::InvalidStatement* node) {
  *ostream_ << "ERROR(" << node->error_code() << ')';
}

}  // namespace internal
}  // namespace joana
