// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PARSER_PARSER_H_
#define JOANA_PARSER_PARSER_H_

#include <memory>

#include "base/macros.h"

namespace joana {
namespace ast {
class ContainerNode;
class EditContext;
class Expression;
class Literal;
class Node;
class NodeFactory;
enum class PunctuatorKind;
class Statement;
}
class SourceCodeRange;

namespace internal {

class Lexer;

class Parser final {
 public:
  Parser(ast::EditContext* context, const SourceCodeRange& range);

  ~Parser();

  // Returns root node, either |Module| or |Script|, of abstract syntax tree
  // for source code specified in |range|. The result may be partial result
  // if source code does not match grammar.
  const ast::Node& Run();

 private:
  enum class ErrorCode;

  ast::NodeFactory& node_factory() const;

  void AddError(const ast::Node& token, ErrorCode error_code);
  void AddError(const SourceCodeRange& range, ErrorCode error_code);

  void Advance();
  // Returns true if |Lexer| has a punctuator of |kind| and advance to next
  // token.
  bool AdvanceIf(ast::PunctuatorKind kind);
  ast::Node& GetToken();
  bool HasToken() const;

  // Expressions
  ast::Expression& NewInvalidExpression(const ast::Node& node,
                                        ErrorCode error_code);
  ast::Expression& NewLiteralExpression(const ast::Literal& literal);
  ast::Expression& ParseExpression();
  ast::Expression& ParseExpressionName();

  // Statements
  ast::Statement& NewInvalidStatement(const ast::Node& node,
                                      ErrorCode error_code);
  ast::Statement& ParseStatement();
  ast::Statement& ParseStatementAsync();
  ast::Statement& ParseStatementBreak();
  ast::Statement& ParseStatementConst();
  ast::Statement& ParseStatementContinue();
  ast::Statement& ParseStatementDo();
  ast::Statement& ParseStatementExpression();
  ast::Statement& ParseStatementFor();
  ast::Statement& ParseStatementFunction();
  ast::Statement& ParseStatementIf();
  ast::Statement& ParseStatementKeyword();
  ast::Statement& ParseStatementLet();
  ast::Statement& ParseStatementReturn();
  ast::Statement& ParseStatementSwitch();
  ast::Statement& ParseStatementThrow();
  ast::Statement& ParseStatementVar();
  ast::Statement& ParseStatementWhile();
  ast::Statement& ParseStatementYield();

  ast::EditContext* const context_;
  std::unique_ptr<Lexer> lexer_;
  ast::ContainerNode& root_;

  DISALLOW_COPY_AND_ASSIGN(Parser);
};

}  // namespace internal
}  // namespace joana

#endif  // JOANA_PARSER_PARSER_H_
