// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/parser/parser.h"

#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/public/ast/declarations.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/node_factory.h"
#include "joana/public/ast/tokens.h"
#include "joana/public/source_code.h"

namespace joana {
namespace internal {

//
// Operator Precedence - ascending ordered by operator precedence; low
// precedence to high precedence.
//
enum class Parser::OperatorPrecedence {
  Assignment,
  LogicalOr,
  LogicalAnd,
  BitwiseOr,
  BitwiseXor,
  BitwiseAnd,
  Equality,
  Relational,
  Shift,
  Additive,
  Multiplicative,
  Exponentiation,
  None,
};

namespace {

// Convert |++| and |--| token to a token which represent post update
// operator.
ast::Punctuator& ConvertToPostOperator(ast::NodeFactory* factory,
                                       const ast::Node& op) {
  if (op == ast::PunctuatorKind::PlusPlus) {
    return factory->NewPunctuator(op.range(),
                                  ast::PunctuatorKind::PostPlusPlus);
  }
  if (op == ast::PunctuatorKind::MinusMinus) {
    return factory->NewPunctuator(op.range(),
                                  ast::PunctuatorKind::PostMinusMinus);
  }
  NOTREACHED() << op;
  return factory->NewPunctuator(op.range(), ast::PunctuatorKind::Invalid);
}

bool IsUnaryOperator(const ast::Token& token) {
  return token == ast::NameId::Await || token == ast::NameId::Delete ||
         token == ast::NameId::TypeOf || token == ast::NameId::Void ||
         token == ast::PunctuatorKind::BitNot ||
         token == ast::PunctuatorKind::DotDotDot ||
         token == ast::PunctuatorKind::LogicalNot ||
         token == ast::PunctuatorKind::Minus ||
         token == ast::PunctuatorKind::Plus;
}

bool IsUpdateOperator(const ast::Token& token) {
  return token == ast::PunctuatorKind::PlusPlus ||
         token == ast::PunctuatorKind::MinusMinus;
}

bool CanStartExpression(const ast::Token& token) {
  return !token.Is<ast::Punctuator>() || IsUnaryOperator(token) ||
         IsUpdateOperator(token);
}

}  // namespace

Parser::OperatorPrecedence Parser::CategoryOf(const ast::Token& token) const {
  static const OperatorPrecedence CategoryMap[] = {
#define V(string, capital, upper, category) \
  Parser::OperatorPrecedence::category,
      FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
  };

  if (token == ast::NameId::InstanceOf || token == ast::NameId::In)
    return Parser::OperatorPrecedence::Relational;
  auto* const punctuator = token.TryAs<ast::Punctuator>();
  if (!punctuator)
    return Parser::OperatorPrecedence::None;
  const auto kind = punctuator->kind();
  const auto& it = std::begin(CategoryMap) + static_cast<size_t>(kind);
  DCHECK(it >= std::begin(CategoryMap)) << punctuator;
  DCHECK(it < std::end(CategoryMap)) << punctuator;
  return *it;
}

Parser::OperatorPrecedence Parser::HigherPrecedenceOf(
    OperatorPrecedence category) const {
  DCHECK_NE(category, OperatorPrecedence::None);
  return static_cast<OperatorPrecedence>(static_cast<int>(category) + 1);
}

ast::Expression& Parser::NewDeclarationExpression(
    const ast::Declaration& declaration) {
  return node_factory().NewDeclarationExpression(declaration);
}

ast::Expression& Parser::NewElisionExpression() {
  return node_factory().NewElisionExpression(lexer_->location());
}

ast::Expression& Parser::NewInvalidExpression(ErrorCode error_code) {
  auto& token = ComputeInvalidToken(error_code);
  AddError(token, error_code);
  return node_factory().NewInvalidExpression(token,
                                             static_cast<int>(error_code));
}

ast::Expression& Parser::NewLiteralExpression(const ast::Literal& literal) {
  return node_factory().NewLiteralExpression(literal);
}

ast::Expression& Parser::NewUnaryExpression(const ast::Token& op,
                                            const ast::Expression& expression) {
  return node_factory().NewUnaryExpression(GetSourceCodeRange(), op,
                                           expression);
}

// Parse argument list after consuming left parenthesis.
std::vector<ast::Expression*> Parser::ParseArgumentList() {
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return {};
  std::vector<ast::Expression*> arguments;
  while (HasToken()) {
    arguments.push_back(&ParseExpression());
    if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
      return arguments;
    if (PeekToken() == ast::PunctuatorKind::SemiColon) {
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_RPAREN);
      return arguments;
    }
    if (!ConsumeTokenIf(ast::PunctuatorKind::Comma))
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_COMMA);
  }
  AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_RPAREN);
  return arguments;
}

ast::Expression& Parser::ParseArrayLiteralExpression() {
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBracket);
  SourceCodeRangeScope scope(this);
  ConsumeToken();
  std::vector<ast::Expression*> elements;
  auto has_expression = false;
  while (HasToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBracket)) {
      return node_factory().NewArrayLiteralExpression(GetSourceCodeRange(),
                                                      elements);
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Comma)) {
      if (!has_expression)
        elements.push_back(&NewElisionExpression());
      has_expression = false;
      continue;
    }
    has_expression = true;
    elements.push_back(&ParseAssignmentExpression());
  }
  return node_factory().NewArrayLiteralExpression(GetSourceCodeRange(),
                                                  elements);
}

// Yet another entry pointer used for parsing computed property name.
ast::Expression& Parser::ParseAssignmentExpression() {
  if (!HasToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  SourceCodeRangeScope scope(this);
  auto& left_hand_side = ParseConditionalExpression();
  if (!HasToken() || !PeekToken().Is<ast::Punctuator>())
    return left_hand_side;
  if (CategoryOf(PeekToken()) != OperatorPrecedence::Assignment)
    return left_hand_side;
  const auto& op = ConsumeToken().As<ast::Punctuator>();
  auto& right_hand_side = ParseAssignmentExpression();
  return node_factory().NewAssignmentExpression(
      GetSourceCodeRange(), op, left_hand_side, right_hand_side);
}

ast::Expression& Parser::ParseBinaryExpression(OperatorPrecedence category) {
  SourceCodeRangeScope scope(this);
  if (category == OperatorPrecedence::None)
    return ParseUnaryExpression();
  const auto lower_category = HigherPrecedenceOf(category);
  auto* left = &ParseBinaryExpression(lower_category);
  while (HasToken() && CategoryOf(PeekToken()) == category) {
    auto& op = ConsumeToken().As<ast::Punctuator>();
    auto& right = ParseBinaryExpression(lower_category);
    left = &node_factory().NewBinaryExpression(GetSourceCodeRange(), op, *left,
                                               right);
  }
  return *left;
}

ast::Expression& Parser::ParseCommaExpression() {
  SourceCodeRangeScope scope(this);
  std::vector<ast::Expression*> expressions;
  expressions.push_back(&ParseAssignmentExpression());
  while (ConsumeTokenIf(ast::PunctuatorKind::Comma))
    expressions.push_back(&ParseAssignmentExpression());
  if (expressions.size() == 1)
    return *expressions.front();
  return node_factory().NewCommaExpression(GetSourceCodeRange(), expressions);
}

ast::Expression& Parser::ParseConditionalExpression() {
  SourceCodeRangeScope scope(this);
  auto& expression = ParseBinaryExpression(OperatorPrecedence::LogicalOr);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Question))
    return expression;
  auto& true_expression = ParseAssignmentExpression();
  ExpectToken(ast::PunctuatorKind::Colon,
              ErrorCode::ERROR_EXPRESSION_CONDITIONAL_EXPECT_COLON);
  auto& false_expression = ParseAssignmentExpression();
  return node_factory().NewConditionalExpression(
      GetSourceCodeRange(), expression, true_expression, false_expression);
}

// The entry point of parsing an expression.
ast::Expression& Parser::ParseExpression() {
  if (!HasToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  SourceCodeRangeScope scope(this);
  return ParseCommaExpression();
}

ast::Expression& Parser::ParseFunctionExpression(ast::FunctionKind kind) {
  return NewDeclarationExpression(ParseFunction(kind));
}

ast::Expression& Parser::ParseLeftHandSideExpression() {
  SourceCodeRangeScope scope(this);
  if (PeekToken() == ast::NameId::New)
    return ParseNewExpression();
  auto* expression = &ParsePrimaryExpression();
  while (HasToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftBracket)) {
      auto& name_expression = ParseExpression();
      ExpectToken(ast::PunctuatorKind::RightBracket,
                  ErrorCode::ERROR_EXPRESSION_LHS_EXPECT_RBRACKET);
      expression = &node_factory().NewMemberExpression(
          GetSourceCodeRange(), *expression, name_expression);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Dot)) {
      if (!HasToken() || !PeekToken().Is<ast::Name>()) {
        AddError(ErrorCode::ERROR_EXPRESSION_LHS_EXPECT_NAME);
        return *expression;
      }
      auto& name = ConsumeToken().As<ast::Name>();
      expression = &node_factory().NewPropertyExpression(GetSourceCodeRange(),
                                                         *expression, name);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis)) {
      const auto& arguments = ParseArgumentList();
      expression = &node_factory().NewCallExpression(GetSourceCodeRange(),
                                                     *expression, arguments);
      continue;
    }
    // TODO(eval1749): NYI MembeExpression TemplateLiteral
    return *expression;
  }
  return *expression;
}

ast::Expression& Parser::ParseMethodExpression(ast::FunctionKind kind) {
  return NewDeclarationExpression(ParseMethod(kind));
}

ast::Expression& Parser::ParseNameAsExpression(const ast::Name& name) {
  SourceCodeRangeScope scope(this);
  switch (static_cast<ast::NameId>(name.number())) {
    case ast::NameId::Async:
      ExpectToken(ast::NameId::Function,
                  ErrorCode::ERROR_FUNCTION_EXPECT_FUNCTION);
      return ParseFunctionExpression(ast::FunctionKind::Async);
    case ast::NameId::False:
      return NewLiteralExpression(
          node_factory().NewBooleanLiteral(name, false));
    case ast::NameId::Function:
      if (ConsumeTokenIf(ast::PunctuatorKind::Times))
        return ParseFunctionExpression(ast::FunctionKind::Generator);
      return ParseFunctionExpression(ast::FunctionKind::Normal);
    case ast::NameId::Null:
      return NewLiteralExpression(node_factory().NewNullLiteral(name));
    case ast::NameId::Super:
    case ast::NameId::This:
      return node_factory().NewReferenceExpression(name);
    case ast::NameId::True:
      return NewLiteralExpression(node_factory().NewBooleanLiteral(name, true));
  }
  if (name.IsKeyword())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  if (ConsumeTokenIf(ast::PunctuatorKind::Arrow)) {
    auto& statement = ParseArrowFunctionBody();
    auto& parameter = node_factory().NewReferenceExpression(name);
    return NewDeclarationExpression(node_factory().NewArrowFunction(
        GetSourceCodeRange(), ast::FunctionKind::Normal, parameter, statement));
  }
  return node_factory().NewReferenceExpression(name);
}

ast::Expression& Parser::ParseNewExpression() {
  SourceCodeRangeScope scope(this);
  auto& token_new = ConsumeToken().As<ast::Name>();
  if (HasToken() && PeekToken() == ast::PunctuatorKind::Dot) {
    if (!HasToken() || PeekToken() != ast::NameId::Target) {
      return NewInvalidExpression(
          ErrorCode::ERROR_EXPRESSION_NEW_EXPECT_TARGET);
    }
    auto& token_target = ConsumeToken().As<ast::Name>();
    return node_factory().NewPropertyExpression(
        GetSourceCodeRange(), node_factory().NewReferenceExpression(token_new),
        token_target);
  }
  auto& expression = ParseLeftHandSideExpression();
  if (!HasToken() || PeekToken() != ast::PunctuatorKind::LeftParenthesis)
    return node_factory().NewNewExpression(GetSourceCodeRange(), expression,
                                           {});
  ConsumeToken();
  const auto& arguments = ParseArgumentList();
  return node_factory().NewNewExpression(GetSourceCodeRange(), expression,
                                         arguments);
}

ast::Expression& Parser::ParseObjectLiteralExpression() {
  SourceCodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBrace) << PeekToken();
  ConsumeToken();
  std::vector<ast::Expression*> members;
  auto need_comma = false;
  while (HasToken()) {
    SourceCodeRangeScope scope(this);
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    if (ConsumeTokenIf(ast::PunctuatorKind::Comma)) {
      if (!need_comma)
        members.push_back(&NewElisionExpression());
      need_comma = true;
      continue;
    }
    need_comma = false;
    if (ConsumeTokenIf(ast::NameId::Async)) {
      members.push_back(&ParseMethodExpression(ast::FunctionKind::Async));
      continue;
    }
    if (ConsumeTokenIf(ast::NameId::Get)) {
      members.push_back(&ParseMethodExpression(ast::FunctionKind::Getter));
      continue;
    }
    if (ConsumeTokenIf(ast::NameId::Set)) {
      members.push_back(&ParseMethodExpression(ast::FunctionKind::Setter));
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Times)) {
      members.push_back(&ParseMethodExpression(ast::FunctionKind::Generator));
      continue;
    }
    auto& property_name = ParsePropertyName();
    if (PeekToken() == ast::PunctuatorKind::LeftParenthesis) {
      auto& parameter_list = ParseParameterList();
      auto& method_body = ParseFunctionBody();
      auto& method = node_factory().NewMethod(
          GetSourceCodeRange(), ast::FunctionKind::Normal, property_name,
          parameter_list, method_body);
      members.push_back(&NewDeclarationExpression(method));
      continue;
    }
    need_comma = true;
    if (PeekToken() == ast::PunctuatorKind::RightBrace) {
      members.push_back(&property_name);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Comma)) {
      members.push_back(&property_name);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Colon)) {
      auto& expression = ParseAssignmentExpression();
      members.push_back(&node_factory().NewPropertyDefinitionExpression(
          GetSourceCodeRange(), property_name, expression));
      continue;
    }
    if (PeekToken() == ast::PunctuatorKind::Equal) {
      auto& op = ConsumeToken().As<ast::Punctuator>();
      auto& expression = ParseAssignmentExpression();
      members.push_back(&node_factory().NewAssignmentExpression(
          GetSourceCodeRange(), op, property_name, expression));
      continue;
    }
    AddError(ErrorCode::ERROR_PROPERTY_INVALID_TOKEN);
    ConsumeToken();
    need_comma = false;
  }
  return node_factory().NewObjectLiteralExpression(GetSourceCodeRange(),
                                                   members);
}

ast::Expression& Parser::ParseParenthesis() {
  SourceCodeRangeScope scope(this);
  ConsumeToken();
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis)) {
    auto& parameter = node_factory().NewEmptyExpression(GetSourceCodeRange());
    ExpectToken(ast::PunctuatorKind::Arrow,
                ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_ARROW);
    auto& statement = ParseArrowFunctionBody();
    return NewDeclarationExpression(node_factory().NewArrowFunction(
        GetSourceCodeRange(), ast::FunctionKind::Normal, parameter, statement));
  }
  auto& sub_expression = ParseExpression();
  ExpectToken(ast::PunctuatorKind::RightParenthesis,
              ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_RPAREN);
  auto& expression =
      node_factory().NewGroupExpression(GetSourceCodeRange(), sub_expression);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Arrow))
    return expression;
  auto& statement = ParseArrowFunctionBody();
  return NewDeclarationExpression(node_factory().NewArrowFunction(
      GetSourceCodeRange(), ast::FunctionKind::Normal, expression, statement));
}

// Yet another entry point called by statement parser.
const ast::Expression& Parser::ParseParenthesisExpression() {
  const auto& location = lexer_->location();
  auto& expression = ParseExpression();
  if (auto* grouping = expression.TryAs<ast::GroupExpression>())
    return grouping->expression();
  AddError(location, ErrorCode::ERROR_STATEMENT_EXPECT_LPAREN);
  return expression;
}

// The entry point for parsing property name.
ast::Expression& Parser::ParsePrimaryExpression() {
  if (!HasToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  const auto& token = PeekToken();
  if (token.Is<ast::Literal>())
    return NewLiteralExpression(ConsumeToken().As<ast::Literal>());
  if (token.Is<ast::Name>())
    return ParseNameAsExpression(ConsumeToken().As<ast::Name>());
  if (token == ast::PunctuatorKind::LeftParenthesis)
    return ParseParenthesis();
  if (token == ast::PunctuatorKind::LeftBracket)
    return ParseArrayLiteralExpression();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseObjectLiteralExpression();
  // TODO(eval1749): NYI class expression
  // TODO(eval1749): NYI regular expression literal
  // TODO(eval1749): NYI template literal
  Advance();
  return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
}

ast::Expression& Parser::ParseUnaryExpression() {
  SourceCodeRangeScope scope(this);
  if (IsUnaryOperator(PeekToken())) {
    auto& token = ConsumeToken();
    auto& expression = ParseUnaryExpression();
    return NewUnaryExpression(token, expression);
  }
  if (PeekToken() == ast::NameId::Yield)
    return ParseYieldExpression();
  return ParseUpdateExpression();
}

ast::Expression& Parser::ParseUpdateExpression() {
  SourceCodeRangeScope scope(this);
  if (IsUpdateOperator(PeekToken())) {
    auto& op = ConsumeToken().As<ast::Punctuator>();
    auto& expression = ParseLeftHandSideExpression();
    return NewUnaryExpression(op, expression);
  }
  auto& expression = ParseLeftHandSideExpression();
  if (!HasToken() || !IsUpdateOperator(PeekToken()))
    return expression;
  auto& op = ConvertToPostOperator(&node_factory(), ConsumeToken());
  return NewUnaryExpression(op, expression);
}

ast::Expression& Parser::ParseYieldExpression() {
  SourceCodeRangeScope scope(this);
  auto& keyword = ConsumeToken().As<ast::Name>();
  if (!HasToken())
    return NewUnaryExpression(keyword, NewElisionExpression());
  if (PeekToken() == ast::PunctuatorKind::Times) {
    auto& yield_star = node_factory().NewName(
        SourceCodeRange::Merge(keyword.range(), lexer_->location()),
        ast::NameId::YieldStar);
    ConsumeToken();
    return NewUnaryExpression(yield_star, ParseAssignmentExpression());
  }
  if (CanStartExpression(PeekToken()))
    return NewUnaryExpression(keyword, ParseAssignmentExpression());
  return NewUnaryExpression(keyword, NewElisionExpression());
}

}  // namespace internal
}  // namespace joana
