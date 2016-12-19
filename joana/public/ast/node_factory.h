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
  Empty& NewEmpty(const SourceCodeRange& range);

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
                                  const Expression& parameter_list,
                                  const Node& body);

  Class& NewClass(const SourceCodeRange& range,
                  const Token& name,
                  const Expression& heritage,
                  const Expression& body);

  Function& NewFunction(const SourceCodeRange& range,
                        FunctionKind kind,
                        const Token& name,
                        const Expression& parameter_list,
                        const Statement& body);

  Method& NewMethod(const SourceCodeRange& range,
                    MethodKind is_static,
                    FunctionKind kind,
                    const Expression& name,
                    const Expression& parameter_list,
                    const Statement& body);

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

  // expression '[' expression ']'
  ComputedMemberExpression& NewComputedMemberExpression(
      const SourceCodeRange& range,
      const Expression& expression,
      const Expression& name_expression);

  ConditionalExpression& NewConditionalExpression(
      const SourceCodeRange& range,
      const Expression& condition,
      const Expression& true_expression,
      const Expression& false_expression);
  DeclarationExpression& NewDeclarationExpression(
      const Declaration& declaration);
  ElisionExpression& NewElisionExpression(const SourceCodeRange& range);
  EmptyExpression& NewEmptyExpression(const SourceCodeRange& range);
  GroupExpression& NewGroupExpression(const SourceCodeRange& range,
                                      const Expression& expression);
  InvalidExpression& NewInvalidExpression(const Node& node, int error_code);
  LiteralExpression& NewLiteralExpression(const Literal& literal);

  NewExpression& NewNewExpression(const SourceCodeRange& range,
                                  const Expression& expression,
                                  const std::vector<Expression*>& arguments);

  ObjectLiteralExpression& NewObjectLiteralExpression(
      const SourceCodeRange& range,
      const std::vector<Expression*>& elements);

  MemberExpression& NewMemberExpression(const SourceCodeRange& range,
                                        const Expression& expression,
                                        const Name& name);
  PropertyDefinitionExpression& NewPropertyDefinitionExpression(
      const SourceCodeRange& range,
      const Expression& name,
      const Expression& value);

  ReferenceExpression& NewReferenceExpression(const Name& name);

  RegExpLiteralExpression& NewRegExpLiteralExpression(
      const SourceCodeRange& range,
      const RegExp& regexp,
      const Token& flags);

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

  // RegExp
  RegExp& NewAnyCharRegExp(const SourceCodeRange& range);

  RegExp& NewAssertionRegExp(const SourceCodeRange& range,
                             RegExpAssertionKind kind);

  RegExp& NewCaptureRegExp(const SourceCodeRange& range, const RegExp& pattern);

  RegExp& NewCharSetRegExp(const SourceCodeRange& range);

  RegExp& NewComplementCharSetRegExp(const SourceCodeRange& range);

  RegExp& NewGreedyRepeatRegExp(const SourceCodeRange& range,
                                const RegExp& pattern,
                                const RegExpRepeat& repeat);

  RegExp& NewInvalidRegExp(const SourceCodeRange& range, int error_code);

  RegExp& NewLazyRepeatRegExp(const SourceCodeRange& range,
                              const RegExp& pattern,
                              const RegExpRepeat& repeat);

  RegExp& NewLiteralRegExp(const SourceCodeRange& range);

  RegExp& NewLookAheadRegExp(const SourceCodeRange& range,
                             const RegExp& pattern);

  RegExp& NewLookAheadNotRegExp(const SourceCodeRange& range,
                                const RegExp& pattern);

  RegExp& NewOrRegExp(const SourceCodeRange& range,
                      const std::vector<RegExp*> patterns);

  RegExp& NewSequenceRegExp(const SourceCodeRange& range,
                            const std::vector<RegExp*> patterns);

  // Statements factory members
  BlockStatement& NewBlockStatement(const SourceCodeRange& range,
                                    const std::vector<Statement*>& statements);

  BreakStatement& NewBreakStatement(const SourceCodeRange& range,
                                    const Token& label);

  CaseClause& NewCaseClause(const SourceCodeRange& range,
                            const Expression& expression,
                            const Statement& statement);

  ConstStatement& NewConstStatement(const SourceCodeRange& range,
                                    const Expression& expression);
  ContinueStatement& NewContinueStatement(const SourceCodeRange& range,
                                          const Token& label);
  DeclarationStatement& NewDeclarationStatement(const Declaration& declaration);

  DoStatement& NewDoStatement(const SourceCodeRange& range,
                              const Statement& statement,
                              const Expression& expression);

  EmptyStatement& NewEmptyStatement(const SourceCodeRange& range);

  ExpressionStatement& NewExpressionStatement(const Expression& expression);

  ForStatement& NewForStatement(const SourceCodeRange& range,
                                const Token& keyword,
                                const Expression& init,
                                const Expression& condition,
                                const Expression& step,
                                const Statement& body);

  ForInStatement& NewForInStatement(const SourceCodeRange& range,
                                    const Token& keyword,
                                    const Expression& expression,
                                    const Statement& body);

  ForOfStatement& NewForOfStatement(const SourceCodeRange& range,
                                    const Token& keyword,
                                    const Expression& binding,
                                    const Expression& expression,
                                    const Statement& body);

  IfElseStatement& NewIfElseStatement(const SourceCodeRange& range,
                                      const Expression& condition,
                                      const Statement& then_clause,
                                      const Statement& else_clause);

  IfStatement& NewIfStatement(const SourceCodeRange& range,
                              const Expression& condition,
                              const Statement& then_clause);

  InvalidStatement& NewInvalidStatement(const Node& node, int error_code);

  LabeledStatement& NewLabeledStatement(const SourceCodeRange& range,
                                        const Name& label,
                                        const Statement& statement);

  LetStatement& NewLetStatement(const SourceCodeRange& range,
                                const Expression& expression);
  ReturnStatement& NewReturnStatement(const SourceCodeRange& range,
                                      const Expression& condition);

  SwitchStatement& NewSwitchStatement(const SourceCodeRange& range,
                                      const Expression& expression,
                                      const std::vector<Statement*>& clauses);

  ThrowStatement& NewThrowStatement(const SourceCodeRange& range,
                                    const Expression& expression);

  TryCatchFinallyStatement& NewTryCatchFinallyStatement(
      const SourceCodeRange& range,
      const Statement& try_block,
      const Expression& catch_parameter,
      const Statement& catch_block,
      const Statement& finally_block);
  TryCatchStatement& NewTryCatchStatement(const SourceCodeRange& range,
                                          const Statement& try_block,
                                          const Expression& catch_parameter,
                                          const Statement& catch_block);
  TryFinallyStatement& NewTryFinallyStatement(const SourceCodeRange& range,
                                              const Statement& try_block,
                                              const Statement& finally_block);
  VarStatement& NewVarStatement(const SourceCodeRange& range,
                                const Expression& expression);

  WhileStatement& NewWhileStatement(const SourceCodeRange& range,
                                    const Expression& expression,
                                    const Statement& statement);

  WithStatement& NewWithStatement(const SourceCodeRange& range,
                                  const Expression& expression,
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
