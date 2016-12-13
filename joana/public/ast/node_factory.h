// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_FACTORY_H_
#define JOANA_PUBLIC_AST_NODE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "joana/public/ast/node_forward.h"
#include "joana/public/memory/zone.h"
#include "joana/public/public_export.h"

namespace joana {

class SourceCodeRange;

namespace ast {

class JOANA_PUBLIC_EXPORT NodeFactory final {
 public:
  explicit NodeFactory(Zone* zone);
  ~NodeFactory();

  // Factory member functions
  Comment& NewComment(const SourceCodeRange& range);

  Invalid& NewInvalid(const SourceCodeRange& range, int error_code);

  Module& NewModule(const SourceCodeRange& range);

  // Returns new |Name| token with specified |name_id|. This function is used
  // for constructing pseudo name "yield*".
  Name& NewName(const SourceCodeRange& range, NameId name_id);
  Name& NewName(const SourceCodeRange& range);

  Punctuator& NewPunctuator(const SourceCodeRange& range, PunctuatorKind kind);

  //
  // Declarations factory members
  //
  ArrowFunction& NewArrowFunction(const SourceCodeRange& range,
                                  FunctionKind kind,
                                  const std::vector<Expression*>& parameters,
                                  const Node& body);

  // Expressions factory members
  ArrayLiteralExpression& NewArrayLiteralExpression(
      const SourceCodeRange& range,
      const std::vector<Expression*>& elements);
  AssignmentExpression& NewAssignmentExpression(
      const SourceCodeRange& range,
      const Punctuator& op,
      const Expression& left_hand_side,
      const Expression& right_hand_side);
  BinaryExpression& NewBinaryExpression(const SourceCodeRange& range,
                                        const Token& op,
                                        const Expression& left_hand_side,
                                        const Expression& right_hand_side);
  CallExpression& NewCallExpression(const SourceCodeRange& range,
                                    const Expression& callee,
                                    const std::vector<Expression*>& arguments);
  CommaExpression& NewCommaExpression(
      const SourceCodeRange& range,
      const std::vector<Expression*> expressions);
  ConditionalExpression& NewConditionalExpression(
      const SourceCodeRange& range,
      const Expression& condition,
      const Expression& true_expression,
      const Expression& false_expression);
  DeclarationExpression& NewDeclarationExpression(
      const Declaration& declaration);
  ElisionExpression& NewElisionExpression(const SourceCodeRange& range);
  GroupExpression& NewGroupExpression(const SourceCodeRange& range,
                                      const Expression& expression);
  InvalidExpression& NewInvalidExpression(const Node& node, int error_code);
  LiteralExpression& NewLiteralExpression(const Literal& literal);
  MemberExpression& NewMemberExpression(const SourceCodeRange& range,
                                        const Expression& expression,
                                        const Expression& name_expression);
  NewExpression& NewNewExpression(const SourceCodeRange& range,
                                  const Expression& expression,
                                  const std::vector<Expression*>& arguments);
  PropertyExpression& NewPropertyExpression(const SourceCodeRange& range,
                                            const Expression& expression,
                                            const Name& name);
  ReferenceExpression& NewReferenceExpression(const Name& name);
  UnaryExpression& NewUnaryExpression(const SourceCodeRange& range,
                                      const Token& op,
                                      const Expression& expression);

  // Literals factory members
  BooleanLiteral& NewBooleanLiteral(const Name& name, bool value);

  NullLiteral& NewNullLiteral(const Name& name);

  NumericLiteral& NewNumericLiteral(const SourceCodeRange& range, double value);

  StringLiteral& NewStringLiteral(const SourceCodeRange& range,
                                  base::StringPiece16 data);

  UndefinedLiteral& NewUndefinedLiteral(const Name& name);

  // Statements factory members
  BlockStatement& NewBlockStatement(const Punctuator& left_brace);
  BreakStatement& NewBreakStatement(const Name& keyword, const Name& label);
  BreakStatement& NewBreakStatement(const Name& keyword);
  CaseClause& NewCaseClause(const Name& keyword,
                            const Expression& expression,
                            const Statement& statement);
  ContinueStatement& NewContinueStatement(const Name& keyword,
                                          const Name& label);
  ContinueStatement& NewContinueStatement(const Name& keyword);
  DoStatement& NewDoStatement(const Name& keyword,
                              const Statement& statement,
                              const Expression& condition);
  EmptyStatement& NewEmptyStatement(const Punctuator& semi_colon);
  ExpressionStatement& NewExpressionStatement(const Expression& expression);
  IfStatement& NewIfStatement(const Name& keyword,
                              const Expression& condition,
                              const Statement& then_clause,
                              const Statement& else_clause);
  IfStatement& NewIfStatement(const Name& keyword,
                              const Expression& condition,
                              const Statement& then_clause);
  InvalidStatement& NewInvalidStatement(const Node& node, int error_code);
  LabeledStatement& NewLabeledStatement(const Name& label,
                                        const Statement& statement);
  SwitchStatement& NewSwitchStatement(const Name& keyword,
                                      const Expression& expression,
                                      const std::vector<Statement*>& clauses);
  ThrowStatement& NewThrowStatement(const Name& keyword,
                                    const Expression& condition);
  TryCatchStatement& NewTryCatchStatement(const Name& keyword,
                                          const Statement& block,
                                          const Name& catch_name,
                                          const Statement& catch_block,
                                          const Statement& finally_block);
  TryCatchStatement& NewTryCatchStatement(const Name& keyword,
                                          const Statement& block,
                                          const Name& catch_name,
                                          const Statement& catch_block);
  TryFinallyStatement& NewTryFinallyStatement(const Name& keyword,
                                              const Statement& block,
                                              const Statement& finally_block);
  WhileStatement& NewWhileStatement(const Name& keyword,
                                    const Expression& condition,
                                    const Statement& statement);

 private:
  class NameIdMap;

  std::unique_ptr<NameIdMap> name_id_map_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_FACTORY_H_
