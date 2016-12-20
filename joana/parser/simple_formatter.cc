// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "joana/parser/simple_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/ast/declarations.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/expressions.h"
#include "joana/ast/literals.h"
#include "joana/ast/module.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/public/source_code.h"

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
    for (const auto& child : block->statements()) {
      FormatWithIndent(*child);
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

void SimpleFormatter::FormatExpressionList(const ast::ExpressionList& list) {
  auto delimiter = "";
  for (const auto& element : list) {
    if (element->Is<ast::ElisionExpression>()) {
      *ostream_ << ',';
      delimiter = "";
      continue;
    }
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(*element);
  }
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

void SimpleFormatter::VisitEmpty(ast::Empty* node) {
  *ostream_ << "(empty)";
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

// Declarations
void SimpleFormatter::VisitArrowFunction(ast::ArrowFunction* node) {
  Format(node->parameter_list());
  *ostream_ << " =>";
  if (auto* block = node->body().TryAs<ast::BlockStatement>()) {
    FormatChildStatement(*block);
    return;
  }
  *ostream_ << ' ';
  Format(node->body());
}

void SimpleFormatter::VisitClass(ast::Class* node) {
  *ostream_ << "class";
  if (!node->name().Is<ast::Empty>()) {
    *ostream_ << ' ';
    Format(node->name());
  }
  if (!node->heritage().Is<ast::EmptyExpression>()) {
    *ostream_ << " extends ";
    Format(node->heritage());
  }
  *ostream_ << ' ';
  Format(node->body());
}

void SimpleFormatter::VisitFunction(ast::Function* node) {
  switch (node->kind()) {
    case ast::FunctionKind::Async:
      *ostream_ << "async function";
      break;
    case ast::FunctionKind::Generator:
      *ostream_ << "function*";
      break;
    case ast::FunctionKind::Getter:
    case ast::FunctionKind::Invalid:
    case ast::FunctionKind::Setter:
      NOTREACHED();
      break;
    case ast::FunctionKind::Normal:
      *ostream_ << "function";
      break;
  }
  if (!node->name().Is<ast::Empty>()) {
    *ostream_ << ' ';
    Format(node->name());
  }
  Format(node->parameter_list());
  if (auto* block = node->body().TryAs<ast::BlockStatement>()) {
    FormatChildStatement(*block);
    return;
  }
  *ostream_ << ' ';
  Format(node->body());
}

void SimpleFormatter::VisitMethod(ast::Method* node) {
  if (node->is_static())
    *ostream_ << "static ";
  switch (node->kind()) {
    case ast::FunctionKind::Async:
      *ostream_ << "async ";
      break;
    case ast::FunctionKind::Generator:
      *ostream_ << '*';
      break;
    case ast::FunctionKind::Getter:
      *ostream_ << "get ";
      break;
    case ast::FunctionKind::Normal:
      break;
    case ast::FunctionKind::Invalid:
      NOTREACHED();
      break;
    case ast::FunctionKind::Setter:
      *ostream_ << "set ";
      break;
  }
  Format(node->name());
  Format(node->parameter_list());
  if (auto* block = node->body().TryAs<ast::BlockStatement>()) {
    if (block->statements().empty()) {
      *ostream_ << " {}";
      return;
    }
    if (block->statements().size() == 1) {
      *ostream_ << " { ";
      Format(**block->statements().begin());
      *ostream_ << " }";
      return;
    }
    FormatChildStatement(*block);
    return;
  }
  if (!node->body().Is<ast::EmptyStatement>())
    *ostream_ << ' ';
  Format(node->body());
}

// Expressions

void SimpleFormatter::VisitArrayLiteralExpression(
    ast::ArrayLiteralExpression* node) {
  *ostream_ << '[';
  FormatExpressionList(node->elements());
  *ostream_ << ']';
}

void SimpleFormatter::VisitAssignmentExpression(
    ast::AssignmentExpression* node) {
  Format(node->lhs());
  *ostream_ << ' ';
  Format(node->op());
  *ostream_ << ' ';
  Format(node->rhs());
}

void SimpleFormatter::VisitBinaryExpression(ast::BinaryExpression* node) {
  Format(node->lhs());
  *ostream_ << ' ';
  Format(node->op());
  *ostream_ << ' ';
  Format(node->rhs());
}

void SimpleFormatter::VisitCallExpression(ast::CallExpression* node) {
  Format(node->callee());
  *ostream_ << '(';
  auto delimiter = "";
  for (const auto& argument : node->arguments()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(*argument);
  }
  *ostream_ << ')';
}

void SimpleFormatter::VisitCommaExpression(ast::CommaExpression* node) {
  FormatExpressionList(node->expressions());
}

void SimpleFormatter::VisitConditionalExpression(
    ast::ConditionalExpression* node) {
  Format(node->condition());
  *ostream_ << " ? ";
  Format(node->true_expression());
  *ostream_ << " : ";
  Format(node->false_expression());
}

void SimpleFormatter::VisitComputedMemberExpression(
    ast::ComputedMemberExpression* node) {
  Format(node->expression());
  *ostream_ << '[';
  Format(node->name_expression());
  *ostream_ << ']';
}

void SimpleFormatter::VisitDeclarationExpression(
    ast::DeclarationExpression* node) {
  Format(node->declaration());
}

void SimpleFormatter::VisitGroupExpression(ast::GroupExpression* node) {
  *ostream_ << '(';
  Format(node->expression());
  *ostream_ << ')';
}

void SimpleFormatter::VisitElisionExpression(ast::ElisionExpression* node) {}

void SimpleFormatter::VisitEmptyExpression(ast::EmptyExpression* node) {
  *ostream_ << "()";
}

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

void SimpleFormatter::VisitNewExpression(ast::NewExpression* node) {
  *ostream_ << "new ";
  Format(node->expression());
  *ostream_ << '(';
  FormatExpressionList(node->arguments());
  *ostream_ << ')';
}

void SimpleFormatter::VisitObjectLiteralExpression(
    ast::ObjectLiteralExpression* node) {
  const auto& members = node->members();
  auto number_of_members = 0;
  for (const auto& member : node->members()) {
    if (member->Is<ast::ElisionExpression>())
      continue;
    ++number_of_members;
  }
  if (number_of_members == 0) {
    *ostream_ << "{}";
    return;
  }
  if (number_of_members == 1) {
    *ostream_ << '{';
    auto delimiter = "";
    for (const auto& member : node->members()) {
      *ostream_ << delimiter;
      if (member->Is<ast::DeclarationExpression>()) {
        *ostream_ << ' ';
        delimiter = "";
        FormatWithIndent(*member);
        continue;
      }
      if (member->Is<ast::ElisionExpression>()) {
        delimiter = ",";
        continue;
      }
      *ostream_ << ' ';
      FormatWithIndent(*member);
      delimiter = ",";
    }
    *ostream_ << " }";
    return;
  }
  *ostream_ << '{' << std::endl;
  IndentScope indent_scope(this);
  size_t count = 0;
  for (const auto& member : members) {
    ++count;
    if (member->Is<ast::DeclarationExpression>()) {
      FormatWithIndent(*member);
      *ostream_ << std::endl;
      continue;
    }
    if (member->Is<ast::ElisionExpression>()) {
      if (count < members.size())
        *ostream_ << ',' << std::endl;
      continue;
    }
    FormatWithIndent(*member);
    if (count < members.size())
      *ostream_ << ',';
    *ostream_ << std::endl;
  }
  *ostream_ << '}';
}

void SimpleFormatter::VisitMemberExpression(ast::MemberExpression* node) {
  Format(node->expression());
  *ostream_ << '.';
  Format(node->name());
}

void SimpleFormatter::VisitPropertyDefinitionExpression(
    ast::PropertyDefinitionExpression* node) {
  Format(node->name());
  *ostream_ << ": ";
  Format(node->value());
}

void SimpleFormatter::VisitReferenceExpression(ast::ReferenceExpression* node) {
  OutputUsingSoourceCode(node->name());
}

void SimpleFormatter::VisitUnaryExpression(ast::UnaryExpression* node) {
  if (node->op() == ast::PunctuatorKind::PostPlusPlus ||
      node->op() == ast::PunctuatorKind::PostMinusMinus) {
    Format(node->expression());
    OutputUsingSoourceCode(node->op());
    return;
  }
  OutputUsingSoourceCode(node->op());
  if (node->expression().Is<ast::ElisionExpression>())
    return;
  if (node->op().Is<ast::Name>())
    *ostream_ << ' ';
  Format(node->expression());
}

void SimpleFormatter::VisitRegExpLiteralExpression(
    ast::RegExpLiteralExpression* node) {
  *ostream_ << '/';
  Format(node->pattern());
  *ostream_ << '/';
  OutputUsingSoourceCode(node->flags());
}

// RegExp
void SimpleFormatter::VisitAnyCharRegExp(ast::AnyCharRegExp* node) {
  *ostream_ << '.';
}

void SimpleFormatter::VisitAssertionRegExp(ast::AssertionRegExp* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitCaptureRegExp(ast::CaptureRegExp* node) {
  *ostream_ << '(';
  Format(node->pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitCharSetRegExp(ast::CharSetRegExp* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitComplementCharSetRegExp(
    ast::ComplementCharSetRegExp* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitGreedyRepeatRegExp(ast::GreedyRepeatRegExp* node) {
  Format(node->pattern());
  *ostream_ << node->repeat();
}

void SimpleFormatter::VisitInvalidRegExp(ast::InvalidRegExp* node) {
  const auto string = ast::ErrorStringOf(node->error_code());
  if (string.empty())
    *ostream_ << node->error_code();
  else
    *ostream_ << string;
}

void SimpleFormatter::VisitLiteralRegExp(ast::LiteralRegExp* node) {
  OutputUsingSoourceCode(*node);
}

void SimpleFormatter::VisitLazyRepeatRegExp(ast::LazyRepeatRegExp* node) {
  Format(node->pattern());
  *ostream_ << node->repeat() << '?';
}

void SimpleFormatter::VisitLookAheadRegExp(ast::LookAheadRegExp* node) {
  *ostream_ << "(?=";
  Format(node->pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitLookAheadNotRegExp(ast::LookAheadNotRegExp* node) {
  *ostream_ << "(?!";
  Format(node->pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitOrRegExp(ast::OrRegExp* node) {
  auto delimiter = "";
  for (const auto& pattern : node->patterns()) {
    *ostream_ << delimiter;
    delimiter = "|";
    Format(*pattern);
  }
}

void SimpleFormatter::VisitSequenceRegExp(ast::SequenceRegExp* node) {
  if (node->patterns().empty()) {
    *ostream_ << "(?:)";
    return;
  }
  for (const auto& pattern : node->patterns())
    Format(*pattern);
}

// Statements
void SimpleFormatter::VisitBlockStatement(ast::BlockStatement* node) {
  *ostream_ << '{' << std::endl;
  {
    IndentScope scope(this);
    for (const auto& child : node->statements()) {
      FormatWithIndent(*child);
      *ostream_ << std::endl;
    }
  }
  OutputIndent();
  *ostream_ << '}';
}

void SimpleFormatter::VisitBreakStatement(ast::BreakStatement* node) {
  if (node->label().Is<ast::Empty>()) {
    *ostream_ << "break;";
    return;
  }
  *ostream_ << "break ";
  Format(node->label());
  *ostream_ << ';';
}

void SimpleFormatter::VisitCaseClause(ast::CaseClause* node) {
  *ostream_ << "case ";
  Format(node->expression());
  *ostream_ << ": ";
  Format(node->statement());
}

void SimpleFormatter::VisitConstStatement(ast::ConstStatement* node) {
  *ostream_ << "const ";
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitContinueStatement(ast::ContinueStatement* node) {
  if (node->label().Is<ast::Empty>()) {
    *ostream_ << "continue;";
    return;
  }
  *ostream_ << "continue ";
  Format(node->label());
  *ostream_ << ';';
}

void SimpleFormatter::VisitDeclarationStatement(
    ast::DeclarationStatement* node) {
  Format(node->declaration());
}

void SimpleFormatter::VisitDoStatement(ast::DoStatement* node) {
  *ostream_ << "do";
  if (FormatChildStatement(node->statement()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "while (";
  Format(node->expression());
  *ostream_ << ");";
}

void SimpleFormatter::VisitEmptyStatement(ast::EmptyStatement* node) {
  *ostream_ << ';';
}

void SimpleFormatter::VisitExpressionStatement(ast::ExpressionStatement* node) {
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitForStatement(ast::ForStatement* node) {
  *ostream_ << "for (";
  if (node->keyword().Is<ast::Name>()) {
    Format(node->keyword());
    *ostream_ << ' ';
  }
  Format(node->init());
  *ostream_ << ';';
  if (!node->condition().Is<ast::ElisionExpression>()) {
    *ostream_ << ' ';
    Format(node->condition());
  }
  *ostream_ << ';';
  if (!node->step().Is<ast::ElisionExpression>()) {
    *ostream_ << ' ';
    Format(node->step());
  }
  *ostream_ << ')';
  FormatChildStatement(node->body());
}

void SimpleFormatter::VisitForInStatement(ast::ForInStatement* node) {
  *ostream_ << "for (";
  if (node->keyword().Is<ast::Name>()) {
    Format(node->keyword());
    *ostream_ << ' ';
  }
  Format(node->expression());
  *ostream_ << ')';
  FormatChildStatement(node->body());
}

void SimpleFormatter::VisitForOfStatement(ast::ForOfStatement* node) {
  *ostream_ << "for (";
  if (node->keyword().Is<ast::Name>()) {
    Format(node->keyword());
    *ostream_ << ' ';
  }
  Format(node->binding());
  *ostream_ << " of ";
  Format(node->expression());
  *ostream_ << ')';
  FormatChildStatement(node->body());
}

void SimpleFormatter::VisitIfElseStatement(ast::IfElseStatement* node) {
  *ostream_ << "if (";
  Format(node->expression());
  *ostream_ << ")";
  if (FormatChildStatement(node->then_clause()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "else";
  FormatChildStatement(node->else_clause());
}

void SimpleFormatter::VisitIfStatement(ast::IfStatement* node) {
  *ostream_ << "if (";
  Format(node->expression());
  *ostream_ << ")";
  FormatChildStatement(node->then_clause());
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

void SimpleFormatter::VisitLetStatement(ast::LetStatement* node) {
  *ostream_ << "let ";
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitReturnStatement(ast::ReturnStatement* node) {
  if (node->expression().Is<ast::ElisionExpression>()) {
    *ostream_ << "return;";
    return;
  }
  *ostream_ << "return ";
  Format(node->expression());
  *ostream_ << ';';
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

void SimpleFormatter::VisitTryCatchFinallyStatement(
    ast::TryCatchFinallyStatement* node) {
  *ostream_ << "try";
  FormatChildStatement(node->try_block());
  *ostream_ << " catch ";
  Format(node->catch_parameter());
  FormatChildStatement(node->catch_block());
  *ostream_ << " finally";
  FormatChildStatement(node->finally_block());
}

void SimpleFormatter::VisitTryCatchStatement(ast::TryCatchStatement* node) {
  *ostream_ << "try";
  FormatChildStatement(node->try_block());
  *ostream_ << " catch ";
  Format(node->catch_parameter());
  FormatChildStatement(node->catch_block());
}

void SimpleFormatter::VisitTryFinallyStatement(ast::TryFinallyStatement* node) {
  *ostream_ << "try";
  FormatChildStatement(node->try_block());
  *ostream_ << " finally";
  FormatChildStatement(node->finally_block());
}

void SimpleFormatter::VisitVarStatement(ast::VarStatement* node) {
  *ostream_ << "var ";
  Format(node->expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitWhileStatement(ast::WhileStatement* node) {
  *ostream_ << "while (";
  Format(node->expression());
  *ostream_ << ')';
  FormatChildStatement(node->statement());
}

void SimpleFormatter::VisitWithStatement(ast::WithStatement* node) {
  *ostream_ << "with (";
  Format(node->expression());
  *ostream_ << ')';
  FormatChildStatement(node->statement());
}

}  // namespace internal
}  // namespace joana
