// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/parser/parser.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/literals.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/regexp.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"

namespace joana {
namespace parser {

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

bool CanBePropertyName(const ast::Token& token) {
  return token.Is<ast::Name>() || token == ast::PunctuatorKind::LeftBracket ||
         token.Is<ast::Literal>();
}

bool CanHaveJsDoc(const ast::Expression& expression) {
  if (expression.Is<ast::GroupExpression>())
    return true;
  return expression.Is<ast::DeclarationExpression>();
}

ast::FunctionKind FunctionKindOf(const ast::Token& token) {
  if (token == ast::NameId::Async)
    return ast::FunctionKind::Async;
  if (token == ast::NameId::Get)
    return ast::FunctionKind::Getter;
  if (token == ast::NameId::Set)
    return ast::FunctionKind::Setter;
  return ast::FunctionKind::Normal;
}

// Convert |++| and |--| token to a token which represent post update
// operator.
const ast::Punctuator& ConvertToPostOperator(ast::NodeFactory* factory,
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

// "yield" is not unary operator since Since an expression after "yield" is
// optional,
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

const ast::Expression& Parser::HandleComputedMember(
    const ast::Expression& expression) {
  auto& name_expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightBracket,
                   ErrorCode::ERROR_EXPRESSION_EXPECT_RBRACKET);
  return node_factory().NewComputedMemberExpression(
      GetSourceCodeRange(), expression, name_expression);
}

const ast::Expression& Parser::HandleMember(const ast::Expression& expression) {
  if (!CanPeekToken() || !PeekToken().Is<ast::Name>()) {
    AddError(ErrorCode::ERROR_EXPRESSION_EXPECT_NAME);
    return expression;
  }
  auto& name = ConsumeToken().As<ast::Name>();
  return node_factory().NewMemberExpression(GetSourceCodeRange(), expression,
                                            name);
}

const ast::Expression& Parser::HandleNewExpression(
    const ast::Expression& passed_expression) {
  auto* expression = &passed_expression;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftBracket)) {
      expression = &HandleComputedMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Dot)) {
      expression = &HandleMember(*expression);
      continue;
    }
    // TODO(eval1749): NYI MembeExpression TemplateLiteral
    return *expression;
  }
  return *expression;
}

Parser::OperatorPrecedence Parser::HigherPrecedenceOf(
    OperatorPrecedence category) const {
  DCHECK_NE(category, OperatorPrecedence::None);
  return static_cast<OperatorPrecedence>(static_cast<int>(category) + 1);
}

const ast::Expression& Parser::NewDeclarationExpression(
    const ast::Declaration& declaration) {
  return node_factory().NewDeclarationExpression(declaration);
}

const ast::Expression& Parser::NewDelimiterExpression(
    const ast::Token& delimiter) {
  return node_factory().NewDelimiterExpression(delimiter.range());
}

const ast::Expression& Parser::NewElisionExpression() {
  return node_factory().NewElisionExpression(
      SourceCodeRange::CollapseToEnd(last_token_->range()));
}

const ast::Expression& Parser::NewEmptyExpression() {
  return node_factory().NewEmptyExpression(GetSourceCodeRange());
}

const ast::Expression& Parser::NewInvalidExpression(
    const SourceCodeRange& range,
    ErrorCode error_code) {
  AddError(range, error_code);
  return node_factory().NewInvalidExpression(range,
                                             static_cast<int>(error_code));
}

const ast::Expression& Parser::NewInvalidExpression(const ast::Token& token,
                                                    ErrorCode error_code) {
  return NewInvalidExpression(token.range(), error_code);
}

const ast::Expression& Parser::NewInvalidExpression(ErrorCode error_code) {
  if (CanPeekToken())
    return NewInvalidExpression(PeekToken(), error_code);
  return NewInvalidExpression(source_code().end(), error_code);
}

const ast::Expression& Parser::NewLiteralExpression(
    const ast::Literal& literal) {
  return node_factory().NewLiteralExpression(literal);
}

const ast::Expression& Parser::NewUnaryExpression(
    const ast::Token& op,
    const ast::Expression& expression) {
  return node_factory().NewUnaryExpression(GetSourceCodeRange(), op,
                                           expression);
}

const ast::Expression& Parser::ParseJsDocAsExpression() {
  auto& js_doc = ConsumeToken().As<ast::JsDoc>();
  auto& expression = ParsePrimaryExpression();
  if (!CanHaveJsDoc(expression)) {
    AddError(js_doc, ErrorCode::ERROR_EXPRESSION_UNEXPECT_ANNOTATION);
    return expression;
  }
  AssociateJsDoc(js_doc, expression);
  return expression;
}

// Parse argument list after consuming left parenthesis.
std::vector<const ast::Expression*> Parser::ParseArgumentList() {
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return {};
  std::vector<const ast::Expression*> arguments;
  while (CanPeekToken()) {
    arguments.push_back(&ParseExpression());
    if (!CanPeekToken())
      break;
    if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
      return arguments;
    if (PeekToken() == ast::PunctuatorKind::Semicolon) {
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_RPAREN);
      return arguments;
    }
    if (!ConsumeTokenIf(ast::PunctuatorKind::Comma))
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_COMMA);
  }
  return arguments;
}

const ast::Expression& Parser::ParseArrayLiteralExpression() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBracket);
  ConsumeToken();
  std::vector<const ast::Expression*> elements;
  auto has_expression = false;
  while (CanPeekToken()) {
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
const ast::Expression& Parser::ParseAssignmentExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  NodeRangeScope scope(this);
  auto& left_hand_side = ParseConditionalExpression();
  if (!CanPeekToken() || !PeekToken().Is<ast::Punctuator>())
    return left_hand_side;
  if (CategoryOf(PeekToken()) != OperatorPrecedence::Assignment)
    return left_hand_side;
  const auto& op = ConsumeToken().As<ast::Punctuator>();
  auto& right_hand_side = ParseAssignmentExpression();
  return node_factory().NewAssignmentExpression(
      GetSourceCodeRange(), op, left_hand_side, right_hand_side);
}

const ast::Expression& Parser::ParseBinaryExpression(
    OperatorPrecedence category) {
  NodeRangeScope scope(this);
  if (category == OperatorPrecedence::None)
    return ParseUnaryExpression();
  const auto lower_category = HigherPrecedenceOf(category);
  auto* left = &ParseBinaryExpression(lower_category);
  while (CanPeekToken() && CategoryOf(PeekToken()) == category) {
    auto& op = ConsumeToken();
    if (!CanPeekToken()) {
      AddError(ErrorCode::ERROR_EXPRESSION_EXPECT_EXPRESSION);
      return *left;
    }
    auto& right = ParseBinaryExpression(lower_category);
    left = &node_factory().NewBinaryExpression(GetSourceCodeRange(), op, *left,
                                               right);
  }
  return *left;
}

const ast::Expression& Parser::ParseCommaExpression() {
  NodeRangeScope scope(this);
  std::vector<const ast::Expression*> expressions;
  expressions.push_back(&ParseAssignmentExpression());
  while (ConsumeTokenIf(ast::PunctuatorKind::Comma))
    expressions.push_back(&ParseAssignmentExpression());
  if (expressions.size() == 1)
    return const_cast<ast::Expression&>(*expressions.front());
  return node_factory().NewCommaExpression(GetSourceCodeRange(), expressions);
}

const ast::Expression& Parser::ParseConditionalExpression() {
  NodeRangeScope scope(this);
  auto& expression = ParseBinaryExpression(OperatorPrecedence::LogicalOr);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Question))
    return expression;
  auto& true_expression = ParseAssignmentExpression();
  ExpectPunctuator(ast::PunctuatorKind::Colon,
                   ErrorCode::ERROR_EXPRESSION_CONDITIONAL_EXPECT_COLON);
  auto& false_expression = ParseAssignmentExpression();
  return node_factory().NewConditionalExpression(
      GetSourceCodeRange(), expression, true_expression, false_expression);
}

// The entry point of parsing an expression.
const ast::Expression& Parser::ParseExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  NodeRangeScope scope(this);
  return ParseCommaExpression();
}

const ast::Expression& Parser::ParseFunctionExpression(ast::FunctionKind kind) {
  return NewDeclarationExpression(ParseFunction(kind));
}

// The entry point of parsing a class heritage.
//  LeftHandSideExpression ::= NewExpression | CallExpression
const ast::Expression& Parser::ParseLeftHandSideExpression() {
  NodeRangeScope scope(this);
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  auto* expression = &ParseNewExpression();
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftBracket)) {
      expression = &HandleComputedMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Dot)) {
      expression = &HandleMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis)) {
      const auto& arguments = ParseArgumentList();
      expression = &node_factory().NewCallExpression(GetSourceCodeRange(),
                                                     *expression, arguments);
      continue;
    }
    return *expression;
  }
  return *expression;
}

const ast::Expression& Parser::ParseMethodExpression(
    ast::MethodKind method_kind,
    ast::FunctionKind kind) {
  return NewDeclarationExpression(ParseMethod(method_kind, kind));
}

const ast::Expression& Parser::ParseNameAsExpression() {
  NodeRangeScope scope(this);
  auto& name = ConsumeToken().As<ast::Name>();
  switch (static_cast<ast::NameId>(name.number())) {
    case ast::NameId::Async:
      if (CanPeekToken() && PeekToken() == ast::NameId::Function) {
        ConsumeToken();
        return ParseFunctionExpression(ast::FunctionKind::Async);
      }
      break;
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
    return NewInvalidExpression(name, ErrorCode::ERROR_EXPRESSION_INVALID);
  if (ConsumeTokenIf(ast::PunctuatorKind::Arrow)) {
    auto& statement = ParseArrowFunctionBody();
    auto& parameter = node_factory().NewReferenceExpression(name);
    return NewDeclarationExpression(node_factory().NewArrowFunction(
        GetSourceCodeRange(), ast::FunctionKind::Normal, parameter, statement));
  }
  return node_factory().NewReferenceExpression(name);
}

// NewExpression ::= MemberExpression | 'new' NewExpression
const ast::Expression& Parser::ParseNewExpression() {
  NodeRangeScope scope(this);
  if (PeekToken() == ast::NameId::New) {
    auto& name_new = ConsumeToken().As<ast::Name>();
    if (!CanPeekToken()) {
      return NewInvalidExpression(
          ErrorCode::ERROR_EXPRESSION_EXPECT_EXPRESSION);
    }
    if (PeekToken() == ast::PunctuatorKind::Dot)
      return HandleNewExpression(
          node_factory().NewReferenceExpression(name_new));
    auto& member_expression = ParseNewExpression();
    if (ConsumeTokenIf(ast::PunctuatorKind::LeftParenthesis)) {
      const auto& arguments = ParseArgumentList();
      return node_factory().NewNewExpression(GetSourceCodeRange(),
                                             member_expression, arguments);
    }
    return HandleNewExpression(node_factory().NewNewExpression(
        GetSourceCodeRange(), member_expression, {}));
  }
  return HandleNewExpression(ParsePrimaryExpression());
}

const ast::Expression& Parser::ParseObjectLiteralExpression() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBrace);
  ConsumeToken();
  std::vector<const ast::Expression*> members;
  while (CanPeekToken()) {
    NodeRangeScope scope(this);
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBrace))
      break;
    if (PeekToken() == ast::PunctuatorKind::Comma) {
      members.push_back(&NewDelimiterExpression(ConsumeToken()));
      continue;
    }
    if (PeekToken() == ast::PunctuatorKind::Semicolon) {
      members.push_back(&NewDelimiterExpression(ConsumeToken()));
      continue;
    }
    if (PeekToken().Is<ast::JsDoc>()) {
      // TODO(eval1749): We should handle js_doc in object literal.
      ConsumeToken();
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Times)) {
      members.push_back(&ParseMethodExpression(ast::MethodKind::NonStatic,
                                               ast::FunctionKind::Generator));
      continue;
    }

    if (PeekToken() == ast::NameId::Static) {
      // 'static' can be a property name and method name.
      auto& property_name_static = ParsePropertyName();
      if (!CanPeekToken())
        break;
      if (CanBePropertyName(PeekToken())) {
        // 'static' PropertyName
        const auto function_kind = FunctionKindOf(PeekToken());
        auto& property_name = ParsePropertyName();
        if (!CanPeekToken())
          break;
        members.push_back(&ParsePropertyAfterName(
            property_name, ast::MethodKind::Static, function_kind));
        continue;
      }
      if (ConsumeTokenIf(ast::PunctuatorKind::Times)) {
        // 'static' '*' PropertyName
        members.push_back(&ParseMethodExpression(ast::MethodKind::Static,
                                                 ast::FunctionKind::Generator));
        continue;
      }
      // Found property or method named 'static'.
      members.push_back(&ParsePropertyAfterName(property_name_static,
                                                ast::MethodKind::NonStatic,
                                                ast::FunctionKind::Normal));
      continue;
    }

    const auto function_kind = FunctionKindOf(PeekToken());
    auto& property_name = ParsePropertyName();
    if (!CanPeekToken())
      break;
    members.push_back(&ParsePropertyAfterName(
        property_name, ast::MethodKind::NonStatic, function_kind));
  }
  return node_factory().NewObjectLiteralExpression(GetSourceCodeRange(),
                                                   members);
}

const ast::Expression& Parser::ParseParenthesis() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftParenthesis);
  ConsumeToken();
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis)) {
    auto& parameter = NewEmptyExpression();
    ExpectPunctuator(ast::PunctuatorKind::Arrow,
                     ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_ARROW);
    auto& statement = ParseArrowFunctionBody();
    return NewDeclarationExpression(node_factory().NewArrowFunction(
        GetSourceCodeRange(), ast::FunctionKind::Normal, parameter, statement));
  }
  auto& sub_expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                   ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_RPAREN);
  auto& expression =
      node_factory().NewGroupExpression(GetSourceCodeRange(), sub_expression);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Arrow))
    return expression;
  auto& statement = ParseArrowFunctionBody();
  return NewDeclarationExpression(node_factory().NewArrowFunction(
      GetSourceCodeRange(), ast::FunctionKind::Normal, expression, statement));
}

// The entry point for parsing property name.
const ast::Expression& Parser::ParsePrimaryExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  const auto& token = PeekToken();
  if (token.Is<ast::JsDoc>())
    return ParseJsDocAsExpression();
  if (token.Is<ast::Literal>())
    return NewLiteralExpression(ConsumeToken().As<ast::Literal>());
  if (token == ast::NameId::Class)
    return NewDeclarationExpression(ParseClass());
  if (token.Is<ast::Name>())
    return ParseNameAsExpression();
  if (token == ast::PunctuatorKind::LeftParenthesis)
    return ParseParenthesis();
  if (token == ast::PunctuatorKind::LeftBracket)
    return ParseArrayLiteralExpression();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseObjectLiteralExpression();
  if (token == ast::PunctuatorKind::Divide ||
      token == ast::PunctuatorKind::DivideEqual) {
    return ParseRegExpLiteral();
  }
  // TODO(eval1749): NYI template literal
  return NewInvalidExpression(ConsumeToken(),
                              ErrorCode::ERROR_EXPRESSION_INVALID);
}

const ast::Expression& Parser::ParsePropertyAfterName(
    const ast::Expression& property_name,
    ast::MethodKind method_kind,
    ast::FunctionKind function_kind) {
  DCHECK(CanPeekToken());

  if (function_kind != ast::FunctionKind::Normal &&
      CanBePropertyName(PeekToken())) {
    // 'async' PropertyName '(' ParameterList ')' '{' StatementList '}'
    // 'get' PropertyName '(' ParameterList ')' '{' StatementList '}'
    // 'set' PropertyName '(' ParameterList ')' '{' StatementList '}'
    DCHECK(property_name.Is<ast::ReferenceExpression>()) << property_name;
    return ParseMethodExpression(method_kind, function_kind);
  }

  if (PeekToken() == ast::PunctuatorKind::LeftParenthesis) {
    // PropertyName '(' ParameterList ')' '{' StatementList '}'
    auto& parameter_list = ParseParameterList();
    auto& method_body = ParseFunctionBody();
    auto& method = node_factory().NewMethod(
        GetSourceCodeRange(), method_kind, ast::FunctionKind::Normal,
        property_name, parameter_list, method_body);
    return NewDeclarationExpression(method);
  }

  if (method_kind == ast::MethodKind::Static)
    AddError(ErrorCode::ERROR_PROPERTY_INVALID_STATIC);

  if (PeekToken() == ast::PunctuatorKind::RightBrace)
    return property_name;

  if (ConsumeTokenIf(ast::PunctuatorKind::Comma))
    return property_name;

  if (ConsumeTokenIf(ast::PunctuatorKind::Colon)) {
    auto& expression = ParseAssignmentExpression();
    return node_factory().NewPropertyDefinitionExpression(
        GetSourceCodeRange(), property_name, expression);
  }

  if (PeekToken() == ast::PunctuatorKind::Equal) {
    auto& op = ConsumeToken().As<ast::Punctuator>();
    auto& expression = ParseAssignmentExpression();
    return node_factory().NewAssignmentExpression(GetSourceCodeRange(), op,
                                                  property_name, expression);
  }

  return NewInvalidExpression(ConsumeToken(),
                              ErrorCode::ERROR_PROPERTY_INVALID_TOKEN);
}

// PropertyName ::= LiteralPropertyName | ComputedPropertyName
// LiteralPropertyName ::= IdentifierName | StringLiteral | NumericLiteral
// ComputedPropertyName ::= '[' AssignmentExpression ']'
const ast::Expression& Parser::ParsePropertyName() {
  if (!CanPeekToken()) {
    return NewInvalidExpression(
        ErrorCode::ERROR_PROPERTY_INVALID_PROPERTY_NAME);
  }
  if (PeekToken().Is<ast::Name>()) {
    // Note: we can use any name as property name including keywords.
    return node_factory().NewReferenceExpression(
        ConsumeToken().As<ast::Name>());
  }
  if (PeekToken().Is<ast::Literal>())
    return NewLiteralExpression(ConsumeToken().As<ast::Literal>());
  return ParsePrimaryExpression();
}

const ast::Expression& Parser::ParseRegExpLiteral() {
  NodeRangeScope scope(this);
  auto& regexp = lexer_->ConsumeRegExp();
  // Consume |RegExp| node.
  ConsumeToken();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon()) {
      AddError(GetSourceCodeRange(),
               ErrorCode::ERROR_EXPRESSION_UNEXPECT_NEWLINE);
    }
    return node_factory().NewRegExpLiteralExpression(GetSourceCodeRange(),
                                                     regexp, NewEmptyName());
  }
  auto& flags = CanPeekToken() && PeekToken().Is<ast::Name>() ? ConsumeToken()
                                                              : NewEmptyName();
  return node_factory().NewRegExpLiteralExpression(GetSourceCodeRange(), regexp,
                                                   flags);
}

const ast::Expression& Parser::ParseUnaryExpression() {
  NodeRangeScope scope(this);
  if (IsUnaryOperator(PeekToken())) {
    auto& token = ConsumeToken();
    auto& expression = ParseUnaryExpression();
    return NewUnaryExpression(token, expression);
  }
  if (PeekToken() == ast::NameId::Yield)
    return ParseYieldExpression();
  return ParseUpdateExpression();
}

const ast::Expression& Parser::ParseUpdateExpression() {
  NodeRangeScope scope(this);
  if (IsUpdateOperator(PeekToken())) {
    auto& op = ConsumeToken().As<ast::Punctuator>();
    auto& expression = ParseLeftHandSideExpression();
    return NewUnaryExpression(op, expression);
  }
  auto& expression = ParseLeftHandSideExpression();
  if (!CanPeekToken() || !IsUpdateOperator(PeekToken()))
    return expression;
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon())
      AddError(ErrorCode::ERROR_EXPRESSION_UNEXPECT_NEWLINE);
    return expression;
  }
  auto& op = ConvertToPostOperator(&node_factory(), ConsumeToken());
  return NewUnaryExpression(op, expression);
}

const ast::Expression& Parser::ParseYieldExpression() {
  NodeRangeScope scope(this);
  auto& keyword = ConsumeToken().As<ast::Name>();
  if (!CanPeekToken())
    return NewUnaryExpression(keyword, NewElisionExpression());
  if (PeekToken() == ast::PunctuatorKind::Times) {
    auto& yield_star = node_factory().NewName(
        SourceCodeRange::Merge(keyword.range(), lexer_->location()),
        ast::NameId::YieldStar);
    ConsumeToken();
    return NewUnaryExpression(yield_star, ParseAssignmentExpression());
  }
  if (PeekToken() == ast::PunctuatorKind::Semicolon)
    return NewUnaryExpression(keyword, NewElisionExpression());
  return NewUnaryExpression(keyword, ParseAssignmentExpression());
}

}  // namespace parser
}  // namespace joana
