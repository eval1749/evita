// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_FACTORY_H_
#define JOANA_AST_NODE_FACTORY_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/strings/string_piece.h"
#include "joana/ast/ast_export.h"
#include "joana/ast/node_forward.h"
#include "joana/base/memory/zone.h"

namespace joana {

class SourceCodeRange;

namespace ast {

//
// NodeFactory
//
class JOANA_AST_EXPORT NodeFactory final {
 public:
  explicit NodeFactory(Zone* zone);
  ~NodeFactory();

  // Factory member functions
  Module& NewModule(
      const SourceCodeRange& range,
      const std::vector<const Statement*>& statements,
      const std::unordered_map<const Node*, const JsDoc*>& js_doc_map);

  //
  // Tokens factory members
  //
  const Token& NewJsDoc(const SourceCodeRange& range);
  const Token& NewComment(const SourceCodeRange& range);
  const Token& NewEmpty(const SourceCodeRange& range);

  // Returns new |Name| token with specified |name_id|. This function is used
  // for constructing pseudo name "yield*".
  const Name& NewName(const SourceCodeRange& range, NameId name_id);
  const Name& NewName(const SourceCodeRange& range);

  const Punctuator& NewPunctuator(const SourceCodeRange& range,
                                  PunctuatorKind kind);

  //
  // Declarations factory members
  //
  const ArrowFunction& NewArrowFunction(const SourceCodeRange& range,
                                        FunctionKind kind,
                                        const Expression& parameter_list,
                                        const Node& body);

  const Class& NewClass(const SourceCodeRange& range,
                        const Token& name,
                        const Expression& heritage,
                        const Expression& body);

  const Function& NewFunction(const SourceCodeRange& range,
                              FunctionKind kind,
                              const Token& name,
                              const Expression& parameter_list,
                              const Statement& body);

  const Method& NewMethod(const SourceCodeRange& range,
                          MethodKind method_kind,
                          FunctionKind kind,
                          const Expression& name,
                          const Expression& parameter_list,
                          const Statement& body);

  // Expressions factory members
  const Expression& NewArrayLiteralExpression(
      const SourceCodeRange& range,
      const std::vector<const Expression*>& elements);
  const Expression& NewAssignmentExpression(const SourceCodeRange& range,
                                            const Punctuator& op,
                                            const Expression& left_hand_side,
                                            const Expression& right_hand_side);
  const Expression& NewBinaryExpression(const SourceCodeRange& range,
                                        const Token& op,
                                        const Expression& left_hand_side,
                                        const Expression& right_hand_side);
  const Expression& NewCallExpression(
      const SourceCodeRange& range,
      const Expression& callee,
      const std::vector<const Expression*>& arguments);
  const Expression& NewCommaExpression(
      const SourceCodeRange& range,
      const std::vector<const Expression*>& expressions);

  // expression '[' expression ']'
  const Expression& NewComputedMemberExpression(
      const SourceCodeRange& range,
      const Expression& expression,
      const Expression& name_expression);

  const Expression& NewConditionalExpression(
      const SourceCodeRange& range,
      const Expression& condition,
      const Expression& true_expression,
      const Expression& false_expression);
  const Expression& NewDeclarationExpression(const Declaration& declaration);
  const Expression& NewDelimiterExpression(const SourceCodeRange& range);
  const Expression& NewElisionExpression(const SourceCodeRange& range);
  const Expression& NewEmptyExpression(const SourceCodeRange& range);
  const Expression& NewGroupExpression(const SourceCodeRange& range,
                                       const Expression& expression);
  const Expression& NewInvalidExpression(const SourceCodeRange& range,
                                         int error_code);
  const Expression& NewLiteralExpression(const Literal& literal);

  const Expression& NewNewExpression(
      const SourceCodeRange& range,
      const Expression& expression,
      const std::vector<const Expression*>& arguments);

  const Expression& NewObjectLiteralExpression(
      const SourceCodeRange& range,
      const std::vector<const Expression*>& elements);

  const Expression& NewMemberExpression(const SourceCodeRange& range,
                                        const Expression& expression,
                                        const Name& name);
  const Expression& NewPropertyDefinitionExpression(
      const SourceCodeRange& range,
      const Expression& name,
      const Expression& value);

  const Expression& NewReferenceExpression(const Name& name);

  const Expression& NewRegExpLiteralExpression(const SourceCodeRange& range,
                                               const RegExp& regexp,
                                               const Token& flags);

  const Expression& NewUnaryExpression(const SourceCodeRange& range,
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
  const Statement& NewBlockStatement(
      const SourceCodeRange& range,
      const std::vector<const Statement*>& statements);

  const Statement& NewBreakStatement(const SourceCodeRange& range,
                                     const Token& label);

  const Statement& NewCaseClause(const SourceCodeRange& range,
                                 const Expression& expression,
                                 const Statement& statement);

  const Statement& NewConstStatement(const SourceCodeRange& range,
                                     const Expression& expression);

  const Statement& NewContinueStatement(const SourceCodeRange& range,
                                        const Token& label);

  const Statement& NewDeclarationStatement(const Declaration& declaration);

  const Statement& NewDoStatement(const SourceCodeRange& range,
                                  const Statement& statement,
                                  const Expression& expression);

  const Statement& NewEmptyStatement(const SourceCodeRange& range);

  const Statement& NewExpressionStatement(const Expression& expression);

  const Statement& NewForStatement(const SourceCodeRange& range,
                                   const Token& keyword,
                                   const Expression& init,
                                   const Expression& condition,
                                   const Expression& step,
                                   const Statement& body);

  const Statement& NewForInStatement(const SourceCodeRange& range,
                                     const Token& keyword,
                                     const Expression& expression,
                                     const Statement& body);

  const Statement& NewForOfStatement(const SourceCodeRange& range,
                                     const Token& keyword,
                                     const Expression& binding,
                                     const Expression& expression,
                                     const Statement& body);

  const Statement& NewIfElseStatement(const SourceCodeRange& range,
                                      const Expression& condition,
                                      const Statement& then_clause,
                                      const Statement& else_clause);

  const Statement& NewIfStatement(const SourceCodeRange& range,
                                  const Expression& condition,
                                  const Statement& then_clause);

  const Statement& NewInvalidStatement(const SourceCodeRange& range,
                                       int error_code);

  const Statement& NewLabeledStatement(const SourceCodeRange& range,
                                       const Name& label,
                                       const Statement& statement);

  const Statement& NewLetStatement(const SourceCodeRange& range,
                                   const Expression& expression);

  const Statement& NewReturnStatement(const SourceCodeRange& range,
                                      const Expression& condition);

  const Statement& NewSwitchStatement(
      const SourceCodeRange& range,
      const Expression& expression,
      const std::vector<const Statement*>& clauses);

  const Statement& NewThrowStatement(const SourceCodeRange& range,
                                     const Expression& expression);

  const Statement& NewTryCatchFinallyStatement(
      const SourceCodeRange& range,
      const Statement& try_block,
      const Expression& catch_parameter,
      const Statement& catch_block,
      const Statement& finally_block);

  const Statement& NewTryCatchStatement(const SourceCodeRange& range,
                                        const Statement& try_block,
                                        const Expression& catch_parameter,
                                        const Statement& catch_block);

  const Statement& NewTryFinallyStatement(const SourceCodeRange& range,
                                          const Statement& try_block,
                                          const Statement& finally_block);

  const Statement& NewVarStatement(const SourceCodeRange& range,
                                   const Expression& expression);

  const Statement& NewWhileStatement(const SourceCodeRange& range,
                                     const Expression& expression,
                                     const Statement& statement);

  const Statement& NewWithStatement(const SourceCodeRange& range,
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

#endif  // JOANA_AST_NODE_FACTORY_H_
