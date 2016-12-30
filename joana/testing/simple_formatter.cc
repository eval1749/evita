// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
#include <string>

#include "joana/testing/simple_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/error_codes.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_nodes.h"
#include "joana/ast/literals.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"
#include "joana/base/source_code.h"

namespace joana {
namespace parser {

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

void SimpleFormatter::FormatBindingElements(
    const ast::BindingElementList& elements) {
  auto* delimiter = "";
  for (const auto& element : elements) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(element);
  }
}

bool SimpleFormatter::FormatChildStatement(const ast::Statement& statement) {
  if (auto* block = statement.TryAs<ast::BlockStatement>()) {
    *ostream_ << " {" << std::endl;
    IndentScope scope(this);
    for (const auto& child : block->statements()) {
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

void SimpleFormatter::FormatExpressionList(const ast::ExpressionList& list) {
  auto delimiter = "";
  for (const auto& element : list) {
    if (element.Is<ast::ElisionExpression>()) {
      *ostream_ << ',';
      delimiter = "";
      continue;
    }
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(element);
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
void SimpleFormatter::VisitJsDoc(const ast::JsDoc& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitComment(const ast::Comment& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitEmpty(const ast::Empty& node) {
  *ostream_ << "(empty)";
}

void SimpleFormatter::VisitPunctuator(const ast::Punctuator& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitName(const ast::Name& node) {
  OutputUsingSoourceCode(node);
}

// Binding Element
void SimpleFormatter::VisitArrayBindingPattern(
    const ast::ArrayBindingPattern& node) {
  *ostream_ << '[';
  auto* delimiter = "";
  for (const auto& element : node.elements()) {
    *ostream_ << delimiter;
    delimiter = " ";
    Format(element);
  }
  *ostream_ << ']';
  if (node.initializer().Is<ast::ElisionExpression>())
    return;
  *ostream_ << " = ";
  Format(node.initializer());
}

void SimpleFormatter::VisitBindingCommaElement(
    const ast::BindingCommaElement& node) {
  *ostream_ << ',';
}

void SimpleFormatter::VisitBindingInvalidElement(
    const ast::BindingInvalidElement& node) {
  *ostream_ << "(invalid)";
}

void SimpleFormatter::VisitBindingNameElement(
    const ast::BindingNameElement& node) {
  Format(node.name());
  if (node.initializer().Is<ast::ElisionExpression>())
    return;
  *ostream_ << " = ";
  Format(node.initializer());
}

void SimpleFormatter::VisitBindingProperty(const ast::BindingProperty& node) {
  Format(node.name());
  *ostream_ << ": ";
  Format(node.element());
}

void SimpleFormatter::VisitBindingRestElement(
    const ast::BindingRestElement& node) {
  *ostream_ << "...";
  Format(node.element());
}

void SimpleFormatter::VisitObjectBindingPattern(
    const ast::ObjectBindingPattern& node) {
  *ostream_ << '{';
  auto* delimiter = "";
  for (const auto& element : node.elements()) {
    *ostream_ << delimiter;
    delimiter = " ";
    Format(element);
  }
  *ostream_ << '}';
  if (node.initializer().Is<ast::ElisionExpression>())
    return;
  *ostream_ << " = ";
  Format(node.initializer());
}

// Compilation Unit
void SimpleFormatter::VisitExterns(const ast::Externs& node) {
  for (const auto& statement : node.statements()) {
    FormatWithIndent(statement);
    *ostream_ << std::endl;
  }
}

void SimpleFormatter::VisitModule(const ast::Module& node) {
  for (const auto& statement : node.statements()) {
    FormatWithIndent(statement);
    *ostream_ << std::endl;
  }
}

void SimpleFormatter::VisitScript(const ast::Script& node) {
  for (const auto& statement : node.statements()) {
    FormatWithIndent(statement);
    *ostream_ << std::endl;
  }
}

// JsDoc
void SimpleFormatter::VisitJsDocDocument(const ast::JsDocDocument& node) {
  *ostream_ << "/**";
  for (const auto& element : node.elements())
    Format(element);
  *ostream_ << "*/";
}

void SimpleFormatter::VisitJsDocName(const ast::JsDocName& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitJsDocTag(const ast::JsDocTag& node) {
  OutputUsingSoourceCode(node.name());
  for (const auto& node : node.parameters()) {
    *ostream_ << ' ';
    Format(node);
  }
}

void SimpleFormatter::VisitJsDocText(const ast::JsDocText& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitJsDocType(const ast::JsDocType& node) {
  *ostream_ << '{';
  Format(node.type());
  *ostream_ << '}';
}

// Literals
void SimpleFormatter::VisitBooleanLiteral(const ast::BooleanLiteral& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitNullLiteral(const ast::NullLiteral& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitNumericLiteral(const ast::NumericLiteral& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitStringLiteral(const ast::StringLiteral& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitUndefinedLiteral(const ast::UndefinedLiteral& node) {
  OutputUsingSoourceCode(node);
}

// Declarations
void SimpleFormatter::VisitArrowFunction(const ast::ArrowFunction& node) {
  Format(node.parameter_list());
  *ostream_ << " =>";
  if (auto* block = node.body().TryAs<ast::BlockStatement>()) {
    FormatChildStatement(*block);
    return;
  }
  *ostream_ << ' ';
  Format(node.body());
}

void SimpleFormatter::VisitClass(const ast::Class& node) {
  *ostream_ << "class";
  if (!node.name().Is<ast::Empty>()) {
    *ostream_ << ' ';
    Format(node.name());
  }
  if (!node.heritage().Is<ast::ElisionExpression>()) {
    *ostream_ << " extends ";
    Format(node.heritage());
  }
  *ostream_ << ' ';
  Format(node.body());
}

void SimpleFormatter::VisitFunction(const ast::Function& node) {
  switch (node.kind()) {
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
  if (!node.name().Is<ast::Empty>()) {
    *ostream_ << ' ';
    Format(node.name());
  }
  Format(node.parameter_list());
  if (auto* block = node.body().TryAs<ast::BlockStatement>()) {
    FormatChildStatement(*block);
    return;
  }
  *ostream_ << ' ';
  Format(node.body());
}

void SimpleFormatter::VisitMethod(const ast::Method& node) {
  if (node.method_kind() == ast::MethodKind::Static)
    *ostream_ << "static ";
  switch (node.kind()) {
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
  Format(node.name());
  Format(node.parameter_list());
  if (auto* block = node.body().TryAs<ast::BlockStatement>()) {
    if (block->statements().empty()) {
      *ostream_ << " {}";
      return;
    }
    if (block->statements().size() == 1) {
      *ostream_ << " { ";
      Format(*block->statements().begin());
      *ostream_ << " }";
      return;
    }
    FormatChildStatement(*block);
    return;
  }
  if (!node.body().Is<ast::EmptyStatement>())
    *ostream_ << ' ';
  Format(node.body());
}

// Expressions

void SimpleFormatter::VisitArrayLiteralExpression(
    const ast::ArrayLiteralExpression& node) {
  *ostream_ << '[';
  FormatExpressionList(node.elements());
  *ostream_ << ']';
}

void SimpleFormatter::VisitAssignmentExpression(
    const ast::AssignmentExpression& node) {
  Format(node.lhs());
  *ostream_ << ' ';
  Format(node.op());
  *ostream_ << ' ';
  Format(node.rhs());
}

void SimpleFormatter::VisitBinaryExpression(const ast::BinaryExpression& node) {
  Format(node.lhs());
  *ostream_ << ' ';
  Format(node.op());
  *ostream_ << ' ';
  Format(node.rhs());
}

void SimpleFormatter::VisitCallExpression(const ast::CallExpression& node) {
  Format(node.callee());
  *ostream_ << '(';
  auto delimiter = "";
  for (const auto& argument : node.arguments()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(argument);
  }
  *ostream_ << ')';
}

void SimpleFormatter::VisitCommaExpression(const ast::CommaExpression& node) {
  FormatExpressionList(node.expressions());
}

void SimpleFormatter::VisitConditionalExpression(
    const ast::ConditionalExpression& node) {
  Format(node.condition());
  *ostream_ << " ? ";
  Format(node.true_expression());
  *ostream_ << " : ";
  Format(node.false_expression());
}

void SimpleFormatter::VisitComputedMemberExpression(
    const ast::ComputedMemberExpression& node) {
  Format(node.expression());
  *ostream_ << '[';
  Format(node.name_expression());
  *ostream_ << ']';
}

void SimpleFormatter::VisitDeclarationExpression(
    const ast::DeclarationExpression& node) {
  Format(node.declaration());
}

void SimpleFormatter::VisitGroupExpression(const ast::GroupExpression& node) {
  *ostream_ << '(';
  Format(node.expression());
  *ostream_ << ')';
}

void SimpleFormatter::VisitDelimiterExpression(
    const ast::DelimiterExpression& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitElisionExpression(
    const ast::ElisionExpression& node) {}

void SimpleFormatter::VisitInvalidExpression(
    const ast::InvalidExpression& node) {
  const auto string = ast::ErrorStringOf(node.error_code());
  if (string.empty())
    *ostream_ << node.error_code();
  else
    *ostream_ << string;
}

void SimpleFormatter::VisitLiteralExpression(
    const ast::LiteralExpression& node) {
  OutputUsingSoourceCode(node.literal());
}

void SimpleFormatter::VisitNewExpression(const ast::NewExpression& node) {
  *ostream_ << "new ";
  Format(node.expression());
  *ostream_ << '(';
  FormatExpressionList(node.arguments());
  *ostream_ << ')';
}

void SimpleFormatter::VisitObjectLiteralExpression(
    const ast::ObjectLiteralExpression& node) {
  const auto& members = node.members();
  auto number_of_members = 0;
  for (const auto& member : node.members()) {
    if (member.Is<ast::DelimiterExpression>())
      continue;
    ++number_of_members;
  }
  if (number_of_members == 0) {
    *ostream_ << "{}";
    return;
  }
  if (number_of_members == 1) {
    *ostream_ << "{ ";
    for (const auto& member : node.members())
      Format(member);
    *ostream_ << " }";
    return;
  }
  *ostream_ << '{';
  {
    IndentScope indent_scope(this);
    for (const auto& member : members) {
      if (member.Is<ast::DelimiterExpression>()) {
        Format(member);
        continue;
      }
      *ostream_ << std::endl;
      FormatWithIndent(member);
    }
  }
  *ostream_ << std::endl;
  OutputIndent();
  *ostream_ << '}';
}

void SimpleFormatter::VisitMemberExpression(const ast::MemberExpression& node) {
  Format(node.expression());
  *ostream_ << '.';
  Format(node.name());
}

void SimpleFormatter::VisitParameterList(const ast::ParameterList& node) {
  *ostream_ << '(';
  auto delimiter = "";
  for (const auto& parameter : node) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(parameter);
  }
  *ostream_ << ')';
}

void SimpleFormatter::VisitPropertyDefinitionExpression(
    const ast::PropertyDefinitionExpression& node) {
  Format(node.name());
  *ostream_ << ": ";
  Format(node.value());
}

void SimpleFormatter::VisitReferenceExpression(
    const ast::ReferenceExpression& node) {
  OutputUsingSoourceCode(node.name());
}

void SimpleFormatter::VisitUnaryExpression(const ast::UnaryExpression& node) {
  if (node.op() == ast::PunctuatorKind::PostPlusPlus ||
      node.op() == ast::PunctuatorKind::PostMinusMinus) {
    Format(node.expression());
    OutputUsingSoourceCode(node.op());
    return;
  }
  OutputUsingSoourceCode(node.op());
  if (node.expression().Is<ast::ElisionExpression>())
    return;
  if (node.op().Is<ast::Name>())
    *ostream_ << ' ';
  Format(node.expression());
}

void SimpleFormatter::VisitRegExpLiteralExpression(
    const ast::RegExpLiteralExpression& node) {
  *ostream_ << '/';
  Format(node.pattern());
  *ostream_ << '/';
  OutputUsingSoourceCode(node.flags());
}

// RegExp
void SimpleFormatter::VisitAnyCharRegExp(const ast::AnyCharRegExp& node) {
  *ostream_ << '.';
}

void SimpleFormatter::VisitAssertionRegExp(const ast::AssertionRegExp& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitCaptureRegExp(const ast::CaptureRegExp& node) {
  *ostream_ << '(';
  Format(node.pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitCharSetRegExp(const ast::CharSetRegExp& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitComplementCharSetRegExp(
    const ast::ComplementCharSetRegExp& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitGreedyRepeatRegExp(
    const ast::GreedyRepeatRegExp& node) {
  Format(node.pattern());
  *ostream_ << node.repeat();
}

void SimpleFormatter::VisitInvalidRegExp(const ast::InvalidRegExp& node) {
  const auto string = ast::ErrorStringOf(node.error_code());
  if (string.empty())
    *ostream_ << node.error_code();
  else
    *ostream_ << string;
}

void SimpleFormatter::VisitLiteralRegExp(const ast::LiteralRegExp& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitLazyRepeatRegExp(const ast::LazyRepeatRegExp& node) {
  Format(node.pattern());
  *ostream_ << node.repeat() << '?';
}

void SimpleFormatter::VisitLookAheadRegExp(const ast::LookAheadRegExp& node) {
  *ostream_ << "(?=";
  Format(node.pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitLookAheadNotRegExp(
    const ast::LookAheadNotRegExp& node) {
  *ostream_ << "(?!";
  Format(node.pattern());
  *ostream_ << ')';
}

void SimpleFormatter::VisitOrRegExp(const ast::OrRegExp& node) {
  auto delimiter = "";
  for (const auto& pattern : node.patterns()) {
    *ostream_ << delimiter;
    delimiter = "|";
    Format(*pattern);
  }
}

void SimpleFormatter::VisitSequenceRegExp(const ast::SequenceRegExp& node) {
  if (node.patterns().empty()) {
    *ostream_ << "(?:)";
    return;
  }
  for (const auto& pattern : node.patterns())
    Format(*pattern);
}

// Statements
void SimpleFormatter::VisitBlockStatement(const ast::BlockStatement& node) {
  *ostream_ << '{' << std::endl;
  {
    IndentScope scope(this);
    for (const auto& child : node.statements()) {
      FormatWithIndent(child);
      *ostream_ << std::endl;
    }
  }
  OutputIndent();
  *ostream_ << '}';
}

void SimpleFormatter::VisitBreakStatement(const ast::BreakStatement& node) {
  if (node.label().Is<ast::Empty>()) {
    *ostream_ << "break;";
    return;
  }
  *ostream_ << "break ";
  Format(node.label());
  *ostream_ << ';';
}

void SimpleFormatter::VisitCaseClause(const ast::CaseClause& node) {
  *ostream_ << "case ";
  Format(node.expression());
  *ostream_ << ": ";
  Format(node.statement());
}

void SimpleFormatter::VisitConstStatement(const ast::ConstStatement& node) {
  *ostream_ << "const ";
  FormatBindingElements(node.elements());
  *ostream_ << ';';
}

void SimpleFormatter::VisitContinueStatement(
    const ast::ContinueStatement& node) {
  if (node.label().Is<ast::Empty>()) {
    *ostream_ << "continue;";
    return;
  }
  *ostream_ << "continue ";
  Format(node.label());
  *ostream_ << ';';
}

void SimpleFormatter::VisitDeclarationStatement(
    const ast::DeclarationStatement& node) {
  Format(node.declaration());
}

void SimpleFormatter::VisitDoStatement(const ast::DoStatement& node) {
  *ostream_ << "do";
  if (FormatChildStatement(node.statement()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "while (";
  Format(node.expression());
  *ostream_ << ");";
}

void SimpleFormatter::VisitEmptyStatement(const ast::EmptyStatement& node) {
  *ostream_ << ';';
}

void SimpleFormatter::VisitExpressionStatement(
    const ast::ExpressionStatement& node) {
  Format(node.expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitForStatement(const ast::ForStatement& node) {
  *ostream_ << "for (";
  if (node.keyword().Is<ast::Name>()) {
    Format(node.keyword());
    *ostream_ << ' ';
  }
  Format(node.init());
  *ostream_ << ';';
  if (!node.condition().Is<ast::ElisionExpression>()) {
    *ostream_ << ' ';
    Format(node.condition());
  }
  *ostream_ << ';';
  if (!node.step().Is<ast::ElisionExpression>()) {
    *ostream_ << ' ';
    Format(node.step());
  }
  *ostream_ << ')';
  FormatChildStatement(node.body());
}

void SimpleFormatter::VisitForInStatement(const ast::ForInStatement& node) {
  *ostream_ << "for (";
  if (node.keyword().Is<ast::Name>()) {
    Format(node.keyword());
    *ostream_ << ' ';
  }
  Format(node.expression());
  *ostream_ << ')';
  FormatChildStatement(node.body());
}

void SimpleFormatter::VisitForOfStatement(const ast::ForOfStatement& node) {
  *ostream_ << "for (";
  if (node.keyword().Is<ast::Name>()) {
    Format(node.keyword());
    *ostream_ << ' ';
  }
  Format(node.binding());
  *ostream_ << " of ";
  Format(node.expression());
  *ostream_ << ')';
  FormatChildStatement(node.body());
}

void SimpleFormatter::VisitIfElseStatement(const ast::IfElseStatement& node) {
  *ostream_ << "if (";
  Format(node.expression());
  *ostream_ << ")";
  if (FormatChildStatement(node.then_clause()))
    *ostream_ << ' ';
  else
    *ostream_ << std::endl;
  *ostream_ << "else";
  FormatChildStatement(node.else_clause());
}

void SimpleFormatter::VisitIfStatement(const ast::IfStatement& node) {
  *ostream_ << "if (";
  Format(node.expression());
  *ostream_ << ")";
  FormatChildStatement(node.then_clause());
}

void SimpleFormatter::VisitInvalidStatement(const ast::InvalidStatement& node) {
  const auto string = ast::ErrorStringOf(node.error_code());
  if (string.empty())
    *ostream_ << node.error_code();
  else
    *ostream_ << string;
}

void SimpleFormatter::VisitLabeledStatement(const ast::LabeledStatement& node) {
  Format(node.statement());
}

void SimpleFormatter::VisitLetStatement(const ast::LetStatement& node) {
  *ostream_ << "let ";
  FormatBindingElements(node.elements());
  *ostream_ << ';';
}

void SimpleFormatter::VisitReturnStatement(const ast::ReturnStatement& node) {
  if (node.expression().Is<ast::ElisionExpression>()) {
    *ostream_ << "return;";
    return;
  }
  *ostream_ << "return ";
  Format(node.expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitSwitchStatement(const ast::SwitchStatement& node) {
  *ostream_ << "switch (";
  Format(node.expression());
  *ostream_ << ") {" << std::endl;
  IndentScope switch_scope(this);
  for (const auto& clause : node.clauses()) {
    auto* runner = &clause;
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

void SimpleFormatter::VisitThrowStatement(const ast::ThrowStatement& node) {
  *ostream_ << "throw ";
  Format(node.expression());
  *ostream_ << ';';
}

void SimpleFormatter::VisitTryCatchFinallyStatement(
    const ast::TryCatchFinallyStatement& node) {
  *ostream_ << "try";
  FormatChildStatement(node.try_block());
  *ostream_ << " catch ";
  Format(node.catch_parameter());
  FormatChildStatement(node.catch_block());
  *ostream_ << " finally";
  FormatChildStatement(node.finally_block());
}

void SimpleFormatter::VisitTryCatchStatement(
    const ast::TryCatchStatement& node) {
  *ostream_ << "try";
  FormatChildStatement(node.try_block());
  *ostream_ << " catch ";
  Format(node.catch_parameter());
  FormatChildStatement(node.catch_block());
}

void SimpleFormatter::VisitTryFinallyStatement(
    const ast::TryFinallyStatement& node) {
  *ostream_ << "try";
  FormatChildStatement(node.try_block());
  *ostream_ << " finally";
  FormatChildStatement(node.finally_block());
}

void SimpleFormatter::VisitVarStatement(const ast::VarStatement& node) {
  *ostream_ << "var ";
  FormatBindingElements(node.elements());
  *ostream_ << ';';
}

void SimpleFormatter::VisitWhileStatement(const ast::WhileStatement& node) {
  *ostream_ << "while (";
  Format(node.expression());
  *ostream_ << ')';
  FormatChildStatement(node.statement());
}

void SimpleFormatter::VisitWithStatement(const ast::WithStatement& node) {
  *ostream_ << "with (";
  Format(node.expression());
  *ostream_ << ')';
  FormatChildStatement(node.statement());
}

// Types
void SimpleFormatter::VisitAnyType(const ast::AnyType& node) {
  *ostream_ << '*';
}

void SimpleFormatter::VisitFunctionType(const ast::FunctionType& node) {
  *ostream_ << "function(";
  if (node.kind() == ast::FunctionTypeKind::New)
    *ostream_ << "new:";
  else if (node.kind() == ast::FunctionTypeKind::This)
    *ostream_ << "this:";
  auto* delimiter = "";
  for (const auto& parameter_type : node.parameter_types()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(parameter_type);
  }
  *ostream_ << ')';
  if (node.return_type().Is<ast::VoidType>())
    return;
  *ostream_ << ':';
  Format(node.return_type());
}

void SimpleFormatter::VisitInvalidType(const ast::InvalidType& node) {
  *ostream_ << "(invalid)";
}

void SimpleFormatter::VisitNullableType(const ast::NullableType& node) {
  *ostream_ << '?';
  Format(node.type());
}

void SimpleFormatter::VisitNonNullableType(const ast::NonNullableType& node) {
  *ostream_ << '!';
  Format(node.type());
}

void SimpleFormatter::VisitOptionalType(const ast::OptionalType& node) {
  Format(node.type());
  *ostream_ << '=';
}

void SimpleFormatter::VisitRecordType(const ast::RecordType& node) {
  *ostream_ << '{';
  auto* delimiter = "";
  for (const auto& member : node.members()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(*member.first);
    *ostream_ << ": ";
    Format(*member.second);
  }
  *ostream_ << '}';
}

void SimpleFormatter::VisitRestType(const ast::RestType& node) {
  *ostream_ << "...";
  Format(node.type());
}

void SimpleFormatter::VisitTupleType(const ast::TupleType& node) {
  *ostream_ << '[';
  auto* delimiter = "";
  for (const auto& member : node.members()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(member);
  }
  *ostream_ << ']';
}

void SimpleFormatter::VisitTypeApplication(const ast::TypeApplication& node) {
  OutputUsingSoourceCode(node.name());
  *ostream_ << '<';
  auto* delimiter = "";
  for (const auto& parameter : node.parameters()) {
    *ostream_ << delimiter;
    delimiter = ", ";
    Format(parameter);
  }
  *ostream_ << '>';
}

void SimpleFormatter::VisitTypeGroup(const ast::TypeGroup& node) {
  *ostream_ << '(';
  Format(node.type());
  *ostream_ << ')';
}

void SimpleFormatter::VisitTypeName(const ast::TypeName& node) {
  OutputUsingSoourceCode(node);
}

void SimpleFormatter::VisitUnionType(const ast::UnionType& node) {
  auto* delimiter = "";
  for (const auto& member : node.members()) {
    *ostream_ << delimiter;
    delimiter = "|";
    Format(member);
  }
}

void SimpleFormatter::VisitUnknownType(const ast::UnknownType& node) {
  *ostream_ << '?';
}

void SimpleFormatter::VisitVoidType(const ast::VoidType& node) {
  *ostream_ << "void";
}

}  // namespace parser
}  // namespace joana
