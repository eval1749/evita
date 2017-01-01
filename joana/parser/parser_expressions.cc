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
         token == ast::PunctuatorKind::LeftBracket || token.is_literal();
}

bool CanHaveJsDoc(const ast::Node& expression) {
  return expression == ast::SyntaxCode::GroupExpression ||
         expression == ast::SyntaxCode::ArrowFunction ||
         expression == ast::SyntaxCode::Function ||
         expression == ast::SyntaxCode::Method;
}

ast::FunctionKind FunctionKindOf(const ast::Node& token) {
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
const ast::Node& ConvertToPostOperator(ast::NodeFactory* factory,
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

bool IsKeywordOperator(const ast::Node& token) {
  if (token != ast::SyntaxCode::Name)
    return false;
  return token == ast::NameId::Await || token == ast::NameId::Delete ||
         token == ast::NameId::TypeOf || token == ast::NameId::Void;
}

bool IsUnaryOperator(const ast::Node& token) {
  if (token != ast::SyntaxCode::Punctuator)
    return false;
  return token == ast::PunctuatorKind::BitNot ||
         token == ast::PunctuatorKind::DotDotDot ||
         token == ast::PunctuatorKind::LogicalNot ||
         token == ast::PunctuatorKind::Minus ||
         token == ast::PunctuatorKind::Plus;
}

bool IsUpdateOperator(const ast::Node& token) {
  return token == ast::PunctuatorKind::PlusPlus ||
         token == ast::PunctuatorKind::MinusMinus;
}

}  // namespace

Parser::OperatorPrecedence Parser::CategoryOf(const ast::Node& token) const {
  static const OperatorPrecedence CategoryMap[] = {
#define V(string, capital, upper, category) \
  Parser::OperatorPrecedence::category,
      FOR_EACH_JAVASCRIPT_PUNCTUATOR(V)
#undef V
  };

  if (token == ast::NameId::InstanceOf || token == ast::NameId::In)
    return Parser::OperatorPrecedence::Relational;
  if (token != ast::SyntaxCode::Punctuator)
    return Parser::OperatorPrecedence::None;
  const auto kind = ast::PunctuatorSyntax::KindOf(token);
  const auto& it = std::begin(CategoryMap) + static_cast<size_t>(kind);
  DCHECK(it >= std::begin(CategoryMap)) << token;
  DCHECK(it < std::end(CategoryMap)) << token;
  return *it;
}

const ast::Node& Parser::ConvertExpressionToBindingElement(
    const ast::Node& expression,
    const ast::Node* initializer) {
  // TODO(eval1749): We should associate |BindingElement| to |JsDocDocument|.
  if (expression == ast::SyntaxCode::ReferenceExpression) {
    return node_factory().NewBindingNameElement(
        expression.range(), ast::ReferenceExpressionSyntax::NameOf(expression),
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
    const auto& property_name = ast::PropertySyntax::NameOf(expression);
    if (property_name == ast::SyntaxCode::ReferenceExpression) {
      return node_factory().NewBindingProperty(
          expression.range(),
          ast::ReferenceExpressionSyntax::NameOf(property_name),
          ConvertExpressionToBindingElement(
              ast::PropertySyntax::ValueOf(expression), nullptr));
    }
  } else if (expression == ast::SyntaxCode::AssignmentExpression) {
    if (ast::AssignmentExpressionSyntax::OperatorOf(expression) ==
        ast::PunctuatorKind::Equal) {
      return ConvertExpressionToBindingElement(
          ast::AssignmentExpressionSyntax::LeftHandSideOf(expression),
          &ast::AssignmentExpressionSyntax::RightHandSideOf(expression));
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
  ExpectPunctuator(ast::PunctuatorKind::RightBracket,
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

const ast::Node& Parser::NewUnaryKeywordExpression(
    const ast::Node& op,
    const ast::Node& expression) {
  return node_factory().NewUnaryKeywordExpression(GetSourceCodeRange(), op,
                                                  expression);
}

const ast::Node& Parser::NewUnaryExpression(const ast::Node& op,
                                            const ast::Node& expression) {
  return node_factory().NewUnaryExpression(GetSourceCodeRange(), op,
                                           expression);
}

const ast::Node& Parser::ParseJsDocAsExpression() {
  auto& jsdoc = ConsumeToken();
  auto& expression = ParsePrimaryExpression();
  if (!CanHaveJsDoc(expression)) {
    AddError(jsdoc, ErrorCode::ERROR_EXPRESSION_UNEXPECT_ANNOTATION);
    return expression;
  }
  AssociateJsDoc(jsdoc, expression);
  return expression;
}

// Parse argument list after consuming left parenthesis.
std::vector<const ast::Node*> Parser::ParseArgumentList() {
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis))
    return {};
  std::vector<const ast::Node*> arguments;
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

const ast::Node& Parser::ParseArrayInitializer() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBracket);
  ConsumeToken();
  std::vector<const ast::Node*> elements;
  auto has_expression = false;
  while (CanPeekToken()) {
    if (ConsumeTokenIf(ast::PunctuatorKind::RightBracket)) {
      return node_factory().NewArrayInitializer(GetSourceCodeRange(), elements);
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
      left = &node_factory().NewBinaryKeywordExpression(GetSourceCodeRange(),
                                                        op, *left, right);
    }
  }
  return *left;
}

const ast::Node& Parser::ParseCommaExpression() {
  NodeRangeScope scope(this);
  std::vector<const ast::Node*> expressions;
  expressions.push_back(&ParseAssignmentExpression());
  while (ConsumeTokenIf(ast::PunctuatorKind::Comma))
    expressions.push_back(&ParseAssignmentExpression());
  if (expressions.size() == 1)
    return *expressions.front();
  return node_factory().NewCommaExpression(GetSourceCodeRange(), expressions);
}

const ast::Node& Parser::ParseConditionalExpression() {
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

const ast::Node& Parser::ParseNameAsExpression() {
  NodeRangeScope scope(this);
  auto& name = ConsumeToken();
  switch (static_cast<ast::NameId>(name.name_id())) {
    case ast::NameId::Async:
      if (CanPeekToken() && PeekToken() == ast::NameId::Function) {
        ConsumeToken();
        return ParseFunction(ast::FunctionKind::Async);
      }
      break;
    case ast::NameId::False:
      return node_factory().NewBooleanLiteral(name, false);
    case ast::NameId::Function:
      if (ConsumeTokenIf(ast::PunctuatorKind::Times))
        return ParseFunction(ast::FunctionKind::Generator);
      return ParseFunction(ast::FunctionKind::Normal);
    case ast::NameId::Null:
      return node_factory().NewNullLiteral(name);
    case ast::NameId::Super:
    case ast::NameId::This:
      return node_factory().NewReferenceExpression(name);
    case ast::NameId::True:
      return node_factory().NewBooleanLiteral(name, true);
  }
  if (ast::NameSyntax::IsKeyword(name))
    return NewInvalidExpression(name, ErrorCode::ERROR_EXPRESSION_INVALID);
  if (ConsumeTokenIf(ast::PunctuatorKind::Arrow)) {
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
  if (PeekToken() == ast::NameId::New) {
    auto& name_new = ConsumeToken();
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

const ast::Node& Parser::ParseObjectInitializer() {
  NodeRangeScope scope(this);
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftBrace);
  ConsumeToken();
  std::vector<const ast::Node*> members;
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
    if (PeekToken() == ast::SyntaxCode::JsDocDocument) {
      // TODO(eval1749): We should handle jsdoc in object literal.
      ConsumeToken();
      continue;
    }
    if (ConsumeTokenIf(ast::PunctuatorKind::Times)) {
      members.push_back(&ParseMethod(ast::MethodKind::NonStatic,
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
  DCHECK_EQ(PeekToken(), ast::PunctuatorKind::LeftParenthesis);
  ConsumeToken();
  if (ConsumeTokenIf(ast::PunctuatorKind::RightParenthesis)) {
    auto& parameter_list =
        node_factory().NewParameterList(GetSourceCodeRange(), {});
    ExpectPunctuator(ast::PunctuatorKind::Arrow,
                     ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_ARROW);
    auto& statement = ParseArrowFunctionBody();
    return node_factory().NewArrowFunction(GetSourceCodeRange(),
                                           ast::FunctionKind::Normal,
                                           parameter_list, statement);
  }
  auto& sub_expression = ParseExpression();
  ExpectPunctuator(ast::PunctuatorKind::RightParenthesis,
                   ErrorCode::ERROR_EXPRESSION_PRIMARY_EXPECT_RPAREN);
  auto& expression =
      node_factory().NewGroupExpression(GetSourceCodeRange(), sub_expression);
  if (!ConsumeTokenIf(ast::PunctuatorKind::Arrow))
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
  if (token == ast::NameId::Class)
    return ParseClass();
  if (token == ast::SyntaxCode::Name)
    return ParseNameAsExpression();
  if (token == ast::PunctuatorKind::LeftParenthesis)
    return ParseParenthesis();
  if (token == ast::PunctuatorKind::LeftBracket)
    return ParseArrayInitializer();
  if (token == ast::PunctuatorKind::LeftBrace)
    return ParseObjectInitializer();
  if (token == ast::PunctuatorKind::Divide ||
      token == ast::PunctuatorKind::DivideEqual) {
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
    DCHECK(property_name == ast::SyntaxCode::ReferenceExpression)
        << property_name;
    return ParseMethod(method_kind, function_kind);
  }

  if (PeekToken() == ast::PunctuatorKind::LeftParenthesis) {
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

  if (PeekToken() == ast::PunctuatorKind::RightBrace)
    return property_name;

  if (ConsumeTokenIf(ast::PunctuatorKind::Comma))
    return property_name;

  if (ConsumeTokenIf(ast::PunctuatorKind::Colon)) {
    auto& expression = ParseAssignmentExpression();
    return node_factory().NewProperty(GetSourceCodeRange(), property_name,
                                      expression);
  }

  if (PeekToken() == ast::PunctuatorKind::Equal) {
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
    return node_factory().NewReferenceExpression(ConsumeToken());
  }
  if (PeekToken().is_literal())
    return ConsumeToken();
  return ParsePrimaryExpression();
}

const ast::Node& Parser::ParseRegExpLiteral() {
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
    return NewUnaryKeywordExpression(token, expression);
  }
  if (PeekToken() == ast::NameId::Yield)
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
    return NewUnaryKeywordExpression(keyword, NewElisionExpression());
  if (PeekToken() == ast::PunctuatorKind::Times) {
    auto& yield_star = node_factory().NewName(
        SourceCodeRange::Merge(keyword.range(), lexer_->location()),
        ast::NameId::YieldStar);
    ConsumeToken();
    return NewUnaryKeywordExpression(yield_star, ParseAssignmentExpression());
  }
  if (PeekToken() == ast::PunctuatorKind::Semicolon)
    return NewUnaryKeywordExpression(keyword, NewElisionExpression());
  return NewUnaryKeywordExpression(keyword, ParseAssignmentExpression());
}

}  // namespace parser
}  // namespace joana
