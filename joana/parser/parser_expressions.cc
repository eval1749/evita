// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/parser/parser.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/literals.h"
#include "joana/ast/node_factory.h"
#include "joana/ast/node_traversal.h"
#include "joana/ast/regexp.h"
#include "joana/ast/tokens.h"
#include "joana/base/source_code.h"
#include "joana/parser/lexer/lexer.h"
#include "joana/parser/parser_error_codes.h"
#include "joana/parser/regexp/regexp_parser.h"

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

bool CanBePropertyName(const ast::Node& token) {
  return token == ast::SyntaxCode::Name ||
         token == ast::TokenKind::LeftBracket || token.is_literal();
}

bool CanHaveJsDoc(const ast::Node& expression) {
  return expression == ast::SyntaxCode::GroupExpression ||
         expression == ast::SyntaxCode::ArrowFunction ||
         expression == ast::SyntaxCode::Function ||
         expression == ast::SyntaxCode::Method;
}

ast::FunctionKind FunctionKindOf(const ast::Node& token) {
  if (token == ast::TokenKind::Async)
    return ast::FunctionKind::Async;
  if (token == ast::TokenKind::Get)
    return ast::FunctionKind::Getter;
  if (token == ast::TokenKind::Set)
    return ast::FunctionKind::Setter;
  return ast::FunctionKind::Normal;
}

// Convert |++| and |--| token to a token which represent post update
// operator.
const ast::Node& ConvertToPostOperator(ast::NodeFactory* factory,
                                       const ast::Node& op) {
  if (op == ast::TokenKind::PlusPlus) {
    return factory->NewPunctuator(op.range(), ast::TokenKind::PostPlusPlus);
  }
  if (op == ast::TokenKind::MinusMinus) {
    return factory->NewPunctuator(op.range(), ast::TokenKind::PostMinusMinus);
  }
  NOTREACHED() << op;
  return factory->NewPunctuator(op.range(), ast::TokenKind::Invalid);
}

bool IsKeywordOperator(const ast::Node& token) {
  if (token != ast::SyntaxCode::Name)
    return false;
  return token == ast::TokenKind::Await || token == ast::TokenKind::Delete ||
         token == ast::TokenKind::TypeOf || token == ast::TokenKind::Void;
}

bool IsUnaryOperator(const ast::Node& token) {
  if (token != ast::SyntaxCode::Punctuator)
    return false;
  return token == ast::TokenKind::BitNot ||
         token == ast::TokenKind::DotDotDot ||
         token == ast::TokenKind::LogicalNot ||
         token == ast::TokenKind::Minus || token == ast::TokenKind::Plus;
}

bool IsUpdateOperator(const ast::Node& token) {
  return token == ast::TokenKind::PlusPlus ||
         token == ast::TokenKind::MinusMinus;
}

}  // namespace

Parser::OperatorPrecedence Parser::CategoryOf(const ast::Node& token) const {
  static const OperatorPrecedence CategoryMap[] = {
#define V(string, capital, upper, category) \
  Parser::OperatorPrecedence::category,
      FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
  };

  if (token == ast::TokenKind::InstanceOf || token == ast::TokenKind::In)
    return Parser::OperatorPrecedence::Relational;
  if (token != ast::SyntaxCode::Punctuator)
    return Parser::OperatorPrecedence::None;
  const auto kind = ast::Punctuator::KindOf(token);
  const auto& it = std::begin(CategoryMap) + static_cast<size_t>(kind);
  DCHECK(it >= std::begin(CategoryMap)) << token;
  DCHECK(it < std::end(CategoryMap)) << token;
  return *it;
}

const ast::Node& Parser::ConvertExpressionToBindingElement(
    const ast::Node& expression,
    const ast::Node* initializer) {
  if (expression == ast::SyntaxCode::ReferenceExpression) {
    return node_factory().NewBindingNameElement(
        expression.range(), ast::ReferenceExpression::NameOf(expression),
        initializer ? *initializer : NewElisionExpression(expression));
  }
  if (expression == ast::SyntaxCode::ArrayInitializer) {
    std::vector<const ast::Node*> elements;
    for (const auto& element : ast::NodeTraversal::ChildNodesOf(expression))
      elements.push_back(&ConvertExpressionToBindingElement(element, nullptr));
    return node_factory().NewArrayBindingPattern(
        expression.range(), elements,
        initializer ? *initializer : NewElisionExpression(expression));
  }
  if (expression == ast::SyntaxCode::ObjectInitializer) {
    std::vector<const ast::Node*> members;
    for (const auto& member : ast::NodeTraversal::ChildNodesOf(expression))
      members.push_back(&ConvertExpressionToBindingElement(member, nullptr));
    return node_factory().NewObjectBindingPattern(
        expression.range(), members,
        initializer ? *initializer : NewElisionExpression(expression));
  }
  if (expression == ast::SyntaxCode::Property) {
    const auto& property_name = ast::Property::NameOf(expression);
    if (property_name == ast::SyntaxCode::ReferenceExpression) {
      return node_factory().NewBindingProperty(
          expression.range(), ast::ReferenceExpression::NameOf(property_name),
          ConvertExpressionToBindingElement(ast::Property::ValueOf(expression),
                                            nullptr));
    }
  } else if (expression == ast::SyntaxCode::AssignmentExpression) {
    if (ast::AssignmentExpression::OperatorOf(expression) ==
        ast::TokenKind::Equal) {
      return ConvertExpressionToBindingElement(
          ast::AssignmentExpression::LeftHandSideOf(expression),
          &ast::AssignmentExpression::RightHandSideOf(expression));
    }
  }
  return node_factory().NewBindingInvalidElement(expression.range());
}

std::vector<const ast::Node*> Parser::ConvertExpressionToBindingElements(
    const ast::Node& expression) {
  if (expression != ast::SyntaxCode::CommaExpression)
    return {&ConvertExpressionToBindingElement(expression, nullptr)};
  std::vector<const ast::Node*> parameters;
  for (const auto& member : ast::NodeTraversal::ChildNodesOf(expression))
    parameters.push_back(&ConvertExpressionToBindingElement(member, nullptr));
  return parameters;
}

const ast::Node& Parser::HandleComputedMember(const ast::Node& expression) {
  auto& name_expression = ParseExpression();
  ExpectPunctuator(ast::TokenKind::RightBracket,
                   ErrorCode::ERROR_EXPRESSION_EXPECT_RBRACKET);
  return node_factory().NewComputedMemberExpression(
      GetSourceCodeRange(), expression, name_expression);
}

const ast::Node& Parser::HandleMember(const ast::Node& expression) {
  if (!CanPeekToken() || PeekToken() != ast::SyntaxCode::Name) {
    AddError(ErrorCode::ERROR_EXPRESSION_EXPECT_NAME);
    return expression;
  }
  auto& name = ConsumeToken();
  return node_factory().NewMemberExpression(GetSourceCodeRange(), expression,
                                            name);
}

const ast::Node& Parser::HandleNewExpression(
    const ast::Node& passed_expression) {
  auto* expression = &passed_expression;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::TokenKind::LeftBracket)) {
      expression = &HandleComputedMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::TokenKind::Dot)) {
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

const ast::Node& Parser::NewDelimiterExpression(const ast::Node& delimiter) {
  return node_factory().NewDelimiterExpression(delimiter.range());
}

const ast::Node& Parser::NewElisionExpression(const ast::Node& node) {
  return node_factory().NewElisionExpression(
      SourceCodeRange::CollapseToEnd(node.range()));
}

const ast::Node& Parser::NewElisionExpression() {
  DCHECK(last_token_);
  return NewElisionExpression(*last_token_);
}

const ast::Node& Parser::NewInvalidExpression(const SourceCodeRange& range,
                                              ErrorCode error_code) {
  AddError(range, error_code);
  return node_factory().NewInvalid(range, static_cast<int>(error_code));
}

const ast::Node& Parser::NewInvalidExpression(const ast::Node& token,
                                              ErrorCode error_code) {
  return NewInvalidExpression(token.range(), error_code);
}

const ast::Node& Parser::NewInvalidExpression(ErrorCode error_code) {
  if (CanPeekToken())
    return NewInvalidExpression(PeekToken(), error_code);
  return NewInvalidExpression(source_code().end(), error_code);
}

const ast::Node& Parser::NewUnaryExpression(const ast::Node& op,
                                            const ast::Node& expression) {
  return node_factory().NewUnaryExpression(GetSourceCodeRange(), op,
                                           expression);
}

const ast::Node& Parser::ParseJsDocAsExpression() {
  NodeRangeScope scope(this);
  auto& jsdoc = ConsumeToken();
  auto& expression = ParsePrimaryExpression();
  if (!CanHaveJsDoc(expression))
    AddError(jsdoc, ErrorCode::ERROR_EXPRESSION_UNEXPECT_ANNOTATION);
  return node_factory().NewAnnotation(GetSourceCodeRange(), jsdoc, expression);
}

// Parse argument list after consuming left parenthesis.
std::vector<const ast::Node*> Parser::ParseArgumentList() {
  if (ConsumeTokenIf(ast::TokenKind::RightParenthesis))
    return {};
  std::vector<const ast::Node*> arguments;
  while (CanPeekToken()) {
    arguments.push_back(&ParseExpression());
    if (!CanPeekToken())
      break;
    if (ConsumeTokenIf(ast::TokenKind::RightParenthesis))
      return arguments;
    if (PeekToken() == ast::TokenKind::Semicolon) {
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_RPAREN);
      return arguments;
    }
    if (!ConsumeTokenIf(ast::TokenKind::Comma))
      AddError(ErrorCode::ERROR_EXPRESSION_ARGUMENT_LIST_EXPECT_COMMA);
  }
  return arguments;
}

const ast::Node& Parser::ParseArrayInitializer() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::TokenKind::LeftBracket);
  ConsumeToken();
  std::vector<const ast::Node*> elements;
  auto has_expression = false;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::TokenKind::RightBracket)) {
      return node_factory().NewArrayInitializer(GetSourceCodeRange(), elements);
    }
    if (ConsumeTokenIf(ast::TokenKind::Comma)) {
      if (!has_expression)
        elements.push_back(&NewElisionExpression());
      has_expression = false;
      continue;
    }
    has_expression = true;
    elements.push_back(&ParseAssignmentExpression());
  }
  return node_factory().NewArrayInitializer(GetSourceCodeRange(), elements);
}

// Yet another entry pointer used for parsing computed property name.
const ast::Node& Parser::ParseAssignmentExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  NodeRangeScope scope(this);
  auto& left_hand_side = ParseConditionalExpression();
  if (!CanPeekToken() || PeekToken() != ast::SyntaxCode::Punctuator)
    return left_hand_side;
  if (CategoryOf(PeekToken()) != OperatorPrecedence::Assignment)
    return left_hand_side;
  const auto& op = ConsumeToken();
  auto& right_hand_side = ParseAssignmentExpression();
  return node_factory().NewAssignmentExpression(
      GetSourceCodeRange(), op, left_hand_side, right_hand_side);
}

const ast::Node& Parser::ParseBinaryExpression(OperatorPrecedence category) {
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
    if (op == ast::SyntaxCode::Punctuator) {
      left = &node_factory().NewBinaryExpression(GetSourceCodeRange(), op,
                                                 *left, right);
    } else {
      left = &node_factory().NewBinaryExpression(GetSourceCodeRange(), op,
                                                 *left, right);
    }
  }
  return *left;
}

const ast::Node& Parser::ParseCommaExpression() {
  NodeRangeScope scope(this);
  std::vector<const ast::Node*> expressions;
  expressions.push_back(&ParseAssignmentExpression());
  while (ConsumeTokenIf(ast::TokenKind::Comma))
    expressions.push_back(&ParseAssignmentExpression());
  if (expressions.size() == 1)
    return *expressions.front();
  return node_factory().NewCommaExpression(GetSourceCodeRange(), expressions);
}

const ast::Node& Parser::ParseConditionalExpression() {
  NodeRangeScope scope(this);
  auto& expression = ParseBinaryExpression(OperatorPrecedence::LogicalOr);
  if (!ConsumeTokenIf(ast::TokenKind::Question))
    return expression;
  auto& true_expression = ParseAssignmentExpression();
  ExpectPunctuator(ast::TokenKind::Colon,
                   ErrorCode::ERROR_EXPRESSION_CONDITIONAL_EXPECT_COLON);
  auto& false_expression = ParseAssignmentExpression();
  return node_factory().NewConditionalExpression(
      GetSourceCodeRange(), expression, true_expression, false_expression);
}

// The entry point of parsing an expression.
const ast::Node& Parser::ParseExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  NodeRangeScope scope(this);
  return ParseCommaExpression();
}

// The entry point of parsing a class heritage.
//  LeftHandSideExpression ::= NewExpression | CallExpression
const ast::Node& Parser::ParseLeftHandSideExpression() {
  NodeRangeScope scope(this);
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  auto* expression = &ParseNewExpression();
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::TokenKind::LeftBracket)) {
      expression = &HandleComputedMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::TokenKind::Dot)) {
      expression = &HandleMember(*expression);
      continue;
    }
    if (ConsumeTokenIf(ast::TokenKind::LeftParenthesis)) {
      const auto& arguments = ParseArgumentList();
      expression = &node_factory().NewCallExpression(GetSourceCodeRange(),
                                                     *expression, arguments);
      continue;
    }
    return *expression;
  }
  return *expression;
}

const ast::Node& Parser::ParseNameAsExpression() {
  NodeRangeScope scope(this);
  auto& name = ConsumeToken();
  switch (ast::Token::KindOf(name)) {
    case ast::TokenKind::Async:
      if (CanPeekToken() && PeekToken() == ast::TokenKind::Function) {
        ConsumeToken();
        return ParseFunction(ast::FunctionKind::Async);
      }
      break;
    case ast::TokenKind::False:
      return node_factory().NewBooleanLiteral(name, false);
    case ast::TokenKind::Function:
      if (ConsumeTokenIf(ast::TokenKind::Times))
        return ParseFunction(ast::FunctionKind::Generator);
      return ParseFunction(ast::FunctionKind::Normal);
    case ast::TokenKind::Null:
      return node_factory().NewNullLiteral(name);
    case ast::TokenKind::Super:
    case ast::TokenKind::This:
      return node_factory().NewReferenceExpression(name);
    case ast::TokenKind::True:
      return node_factory().NewBooleanLiteral(name, true);
  }
  if (ast::Name::IsKeyword(name))
    return NewInvalidExpression(name, ErrorCode::ERROR_EXPRESSION_INVALID);
  if (ConsumeTokenIf(ast::TokenKind::Arrow)) {
    auto& statement = ParseArrowFunctionBody();
    auto& parameter = node_factory().NewReferenceExpression(name);
    return node_factory().NewArrowFunction(
        GetSourceCodeRange(), ast::FunctionKind::Normal, parameter, statement);
  }
  return node_factory().NewReferenceExpression(name);
}

// NewExpression ::= MemberExpression | 'new' NewExpression
const ast::Node& Parser::ParseNewExpression() {
  NodeRangeScope scope(this);
  if (PeekToken() == ast::TokenKind::New) {
    auto& name_new = ConsumeToken();
    if (!CanPeekToken()) {
      return NewInvalidExpression(
          ErrorCode::ERROR_EXPRESSION_EXPECT_EXPRESSION);
    }
    if (PeekToken() == ast::TokenKind::Dot)
      return HandleNewExpression(
          node_factory().NewReferenceExpression(name_new));
    auto& member_expression = ParseNewExpression();
    if (ConsumeTokenIf(ast::TokenKind::LeftParenthesis)) {
      const auto& arguments = ParseArgumentList();
      return node_factory().NewNewExpression(GetSourceCodeRange(),
                                             member_expression, arguments);
    }
    return HandleNewExpression(node_factory().NewNewExpression(
        GetSourceCodeRange(), member_expression, {}));
  }
  return HandleNewExpression(ParsePrimaryExpression());
}

const ast::Node& Parser::ParseObjectInitializer() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::TokenKind::LeftBrace);
  ConsumeToken();
  std::vector<const ast::Node*> members;
  while (CanPeekToken()) {
    NodeRangeScope scope(this);
    if (ConsumeTokenIf(ast::TokenKind::RightBrace))
      break;
    if (PeekToken() == ast::TokenKind::Comma) {
      members.push_back(&NewDelimiterExpression(ConsumeToken()));
      continue;
    }
    if (PeekToken() == ast::TokenKind::Semicolon) {
      members.push_back(&NewDelimiterExpression(ConsumeToken()));
      continue;
    }
    if (PeekToken() == ast::SyntaxCode::JsDocDocument) {
      // TODO(eval1749): We should handle jsdoc in object literal.
      ConsumeToken();
      continue;
    }
    if (ConsumeTokenIf(ast::TokenKind::Times)) {
      members.push_back(&ParseMethod(ast::MethodKind::NonStatic,
                                     ast::FunctionKind::Generator));
      continue;
    }

    if (PeekToken() == ast::TokenKind::Static) {
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
      if (ConsumeTokenIf(ast::TokenKind::Times)) {
        // 'static' '*' PropertyName
        members.push_back(&ParseMethod(ast::MethodKind::Static,
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
  return node_factory().NewObjectInitializer(GetSourceCodeRange(), members);
}

const ast::Node& Parser::ParseParenthesis() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::TokenKind::LeftParenthesis);
  ConsumeToken();
  if (ConsumeTokenIf(ast::TokenKind::RightParenthesis)) {
    auto& parameter_list =
        node_factory().NewParameterList(GetSourceCodeRange(), {});
    ExpectPunctuator(ast::TokenKind::Arrow,
                     ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_ARROW);
    auto& statement = ParseArrowFunctionBody();
    return node_factory().NewArrowFunction(GetSourceCodeRange(),
                                           ast::FunctionKind::Normal,
                                           parameter_list, statement);
  }
  auto& sub_expression = ParseExpression();
  ExpectPunctuator(ast::TokenKind::RightParenthesis,
                   ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_RPAREN);
  auto& expression =
      node_factory().NewGroupExpression(GetSourceCodeRange(), sub_expression);
  if (!ConsumeTokenIf(ast::TokenKind::Arrow))
    return expression;
  auto& statement = ParseArrowFunctionBody();
  return node_factory().NewArrowFunction(
      GetSourceCodeRange(), ast::FunctionKind::Normal,
      node_factory().NewParameterList(
          expression.range(),
          ConvertExpressionToBindingElements(sub_expression)),
      statement);
}

// The entry point for parsing property name.
const ast::Node& Parser::ParsePrimaryExpression() {
  if (!CanPeekToken())
    return NewInvalidExpression(ErrorCode::ERROR_EXPRESSION_INVALID);
  const auto& token = PeekToken();
  if (token == ast::SyntaxCode::JsDocDocument)
    return ParseJsDocAsExpression();
  if (token.is_literal())
    return ConsumeToken();
  if (token == ast::TokenKind::Class)
    return ParseClass();
  if (token == ast::SyntaxCode::Name)
    return ParseNameAsExpression();
  if (token == ast::TokenKind::LeftParenthesis)
    return ParseParenthesis();
  if (token == ast::TokenKind::LeftBracket)
    return ParseArrayInitializer();
  if (token == ast::TokenKind::LeftBrace)
    return ParseObjectInitializer();
  if (token == ast::TokenKind::Divide || token == ast::TokenKind::DivideEqual) {
    return ParseRegExpLiteral();
  }
  // TODO(eval1749): NYI template literal
  return NewInvalidExpression(ConsumeToken(),
                              ErrorCode::ERROR_EXPRESSION_INVALID);
}

const ast::Node& Parser::ParsePropertyAfterName(
    const ast::Node& property_name,
    ast::MethodKind method_kind,
    ast::FunctionKind function_kind) {
  DCHECK(CanPeekToken());

  if (function_kind != ast::FunctionKind::Normal &&
      CanBePropertyName(PeekToken())) {
    // 'async' PropertyName '(' ParameterList ')' '{' StatementList '}'
    // 'get' PropertyName '(' ParameterList ')' '{' StatementList '}'
    // 'set' PropertyName '(' ParameterList ')' '{' StatementList '}'
    return ParseMethod(method_kind, function_kind);
  }

  if (PeekToken() == ast::TokenKind::LeftParenthesis) {
    // PropertyName '(' ParameterList ')' '{' StatementList '}'
    auto& parameter_list = ParseParameterList();
    auto& method_body = ParseFunctionBody();
    auto& method = node_factory().NewMethod(
        GetSourceCodeRange(), method_kind, ast::FunctionKind::Normal,
        property_name, parameter_list, method_body);
    return method;
  }

  if (method_kind == ast::MethodKind::Static)
    AddError(ErrorCode::ERROR_PROPERTY_INVALID_STATIC);

  if (PeekToken() == ast::TokenKind::RightBrace)
    return property_name;

  if (ConsumeTokenIf(ast::TokenKind::Comma))
    return property_name;

  if (ConsumeTokenIf(ast::TokenKind::Colon)) {
    auto& expression = ParseAssignmentExpression();
    return node_factory().NewProperty(GetSourceCodeRange(), property_name,
                                      expression);
  }

  if (PeekToken() == ast::TokenKind::Equal) {
    auto& op = ConsumeToken();
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
const ast::Node& Parser::ParsePropertyName() {
  if (!CanPeekToken()) {
    return NewInvalidExpression(
        ErrorCode::ERROR_PROPERTY_INVALID_PROPERTY_NAME);
  }
  if (PeekToken() == ast::SyntaxCode::Name) {
    // Note: we can use any name as property name including keywords.
    return ConsumeToken();
  }
  if (PeekToken().is_literal())
    return ConsumeToken();
  return ParsePrimaryExpression();
}

const ast::Node& Parser::ParseRegExpLiteral() {
  NodeRangeScope scope(this);
  auto& source = lexer_->ExtendTokenAsRegExp();
  // Consume |RegExpSource| node.
  ConsumeToken();

  // Skip starting "/"
  const auto regexp_start = source.range().start() + 1;

  // Skip ending "/" if available
  const auto regexp_end = source_code().CharAt(source.range().end() - 1) == '/'
                              ? source.range().end() - 1
                              : source.range().end();
  const auto& regexp =
      RegExpParser(&context_, source_code().Slice(regexp_start, regexp_end),
                   options_)
          .Parse();
  if (is_separated_by_newline_) {
    if (options_.disable_automatic_semicolon()) {
      AddError(GetSourceCodeRange(),
               ErrorCode::ERROR_EXPRESSION_UNEXPECT_NEWLINE);
    }
    return node_factory().NewRegExpLiteralExpression(GetSourceCodeRange(),
                                                     regexp, NewEmptyName());
  }
  auto& flags = CanPeekToken() && PeekToken() == ast::SyntaxCode::Name
                    ? ConsumeToken()
                    : NewEmptyName();
  return node_factory().NewRegExpLiteralExpression(GetSourceCodeRange(), regexp,
                                                   flags);
}

const ast::Node& Parser::ParseUnaryExpression() {
  NodeRangeScope scope(this);
  if (IsUnaryOperator(PeekToken())) {
    auto& token = ConsumeToken();
    auto& expression = ParseUnaryExpression();
    return NewUnaryExpression(token, expression);
  }
  if (IsKeywordOperator(PeekToken())) {
    auto& token = ConsumeToken();
    auto& expression = ParseUnaryExpression();
    return NewUnaryExpression(token, expression);
  }
  if (PeekToken() == ast::TokenKind::Yield)
    return ParseYieldExpression();
  return ParseUpdateExpression();
}

const ast::Node& Parser::ParseUpdateExpression() {
  NodeRangeScope scope(this);
  if (IsUpdateOperator(PeekToken())) {
    auto& op = ConsumeToken();
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

const ast::Node& Parser::ParseYieldExpression() {
  NodeRangeScope scope(this);
  auto& keyword = ConsumeToken();
  if (!CanPeekToken())
    return NewUnaryExpression(keyword, NewElisionExpression());
  if (PeekToken() == ast::TokenKind::Times) {
    auto& yield_star = node_factory().NewName(
        SourceCodeRange::Merge(keyword.range(), lexer_->location()),
        ast::TokenKind::YieldStar);
    ConsumeToken();
    return NewUnaryExpression(yield_star, ParseAssignmentExpression());
  }
  if (PeekToken() == ast::TokenKind::Semicolon)
    return NewUnaryExpression(keyword, NewElisionExpression());
  return NewUnaryExpression(keyword, ParseAssignmentExpression());
}

}  // namespace parser
}  // namespace joana
