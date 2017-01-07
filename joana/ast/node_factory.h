// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_FACTORY_H_
#define JOANA_AST_NODE_FACTORY_H_

#include <memory>
#include <utility>
#include <vector>

#include "joana/ast/ast_export.h"
#include "joana/ast/syntax_forward.h"
#include "joana/base/memory/zone.h"

namespace joana {

class SourceCodeRange;

namespace ast {

class Node;
class Syntax;
class SyntaxFactory;

//
// NodeFactory
//
class JOANA_AST_EXPORT NodeFactory final {
 public:
  explicit NodeFactory(Zone* zone);
  ~NodeFactory();

  const Node& NewTuple(const SourceCodeRange& range,
                       const std::vector<const Node*>& nodes);

  // Compilation unit factory members
  const Node& NewExterns(const SourceCodeRange& range,
                         const std::vector<const Node*>& statements);

  const Node& NewModule(const SourceCodeRange& range,
                        const std::vector<const Node*>& statements);

  const Node& NewScript(const SourceCodeRange& range,
                        const std::vector<const Node*>& statements);

  //
  // Nodes factory members
  //
  const Node& NewComment(const SourceCodeRange& range);
  const Node& NewEmpty(const SourceCodeRange& range);
  const Node& NewInvalid(const SourceCodeRange& range, int error_code);

  // Returns new |Name| token with specified |name_id|. This function is used
  // for constructing pseudo name "yield*".
  const Node& NewName(const SourceCodeRange& range, TokenKind name_id);
  const Node& NewName(const SourceCodeRange& range);

  const Node& NewPunctuator(const SourceCodeRange& range, TokenKind kind);
  const Node& NewRegExpSource(const SourceCodeRange& range);

  //
  // Bindings factory members
  //
  const Node& NewArrayBindingPattern(const SourceCodeRange& range,
                                     const std::vector<const Node*>& elements,
                                     const Node& initializer);

  const Node& NewBindingCommaElement(const SourceCodeRange& range);

  const Node& NewBindingInvalidElement(const SourceCodeRange& range);

  const Node& NewBindingNameElement(const SourceCodeRange& range,
                                    const Node& name,
                                    const Node& initializer);

  const Node& NewBindingProperty(const SourceCodeRange& range,
                                 const Node& name,
                                 const Node& element);

  const Node& NewBindingRestElement(const SourceCodeRange& range,
                                    const Node& element);

  const Node& NewObjectBindingPattern(const SourceCodeRange& range,
                                      const std::vector<const Node*>& elements,
                                      const Node& initializer);

  //
  // Declarations factory members
  //
  const Node& NewAnnotation(const SourceCodeRange& range,
                            const Node& annotation,
                            const Node& annotated);

  const Node& NewArrowFunction(const SourceCodeRange& range,
                               FunctionKind kind,
                               const Node& parameter_list,
                               const Node& body);

  const Node& NewClass(const SourceCodeRange& range,
                       const Node& name,
                       const Node& heritage,
                       const Node& body);

  const Node& NewFunction(const SourceCodeRange& range,
                          FunctionKind kind,
                          const Node& name,
                          const Node& parameter_list,
                          const Node& body);

  const Node& NewMethod(const SourceCodeRange& range,
                        MethodKind method_kind,
                        FunctionKind kind,
                        const Node& name,
                        const Node& parameter_list,
                        const Node& body);

  // Expression factory members
  const Node& NewArrayInitializer(const SourceCodeRange& range,
                                  const std::vector<const Node*>& elements);

  const Node& NewAssignmentExpression(const SourceCodeRange& range,
                                      const Node& op,
                                      const Node& left_hand_side,
                                      const Node& right_hand_side);

  const Node& NewBinaryExpression(const SourceCodeRange& range,
                                  const Node& op,
                                  const Node& left_hand_side,
                                  const Node& right_hand_side);

  const Node& NewCallExpression(const SourceCodeRange& range,
                                const Node& callee,
                                const std::vector<const Node*>& argument_list);

  const Node& NewCommaExpression(const SourceCodeRange& range,
                                 const std::vector<const Node*>& expressions);

  // expression '[' expression ']'
  const Node& NewComputedMemberExpression(const SourceCodeRange& range,
                                          const Node& expression,
                                          const Node& name_expression);

  const Node& NewConditionalExpression(const SourceCodeRange& range,
                                       const Node& condition,
                                       const Node& true_expression,
                                       const Node& false_expression);
  const Node& NewDelimiterExpression(const SourceCodeRange& range);
  const Node& NewElisionExpression(const SourceCodeRange& range);
  const Node& NewGroupExpression(const SourceCodeRange& range,
                                 const Node& expression);

  const Node& NewNewExpression(const SourceCodeRange& range,
                               const Node& expression,
                               const std::vector<const Node*>& argument_list);

  const Node& NewObjectInitializer(const SourceCodeRange& range,
                                   const std::vector<const Node*>& elements);

  const Node& NewMemberExpression(const SourceCodeRange& range,
                                  const Node& expression,
                                  const Node& name);

  const Node& NewParameterList(const SourceCodeRange& range,
                               const std::vector<const Node*>& expressions);

  const Node& NewProperty(const SourceCodeRange& range,
                          const Node& name,
                          const Node& value);

  const Node& NewReferenceExpression(const Node& name);

  const Node& NewRegExpLiteralExpression(const SourceCodeRange& range,
                                         const Node& regexp,
                                         const Node& flags);

  const Node& NewUnaryExpression(const SourceCodeRange& range,
                                 const Node& op,
                                 const Node& expression);

  // JsDoc factory members
  const Node& NewJsDocDocument(const SourceCodeRange& range,
                               const std::vector<const Node*>& nodes);

  const Node& NewJsDocTag(const SourceCodeRange& range,
                          const Node& name,
                          const std::vector<const Node*>& operands);

  const Node& NewJsDocText(const SourceCodeRange& range);

  // Literals factory members
  const Node& NewBooleanLiteral(const Node& name, bool value);

  const Node& NewNullLiteral(const Node& name);

  const Node& NewNumericLiteral(const SourceCodeRange& range, double value);

  const Node& NewStringLiteral(const SourceCodeRange& range);

  const Node& NewUndefinedLiteral(const Node& name);

  // RegExp
  const Node& NewAnyCharRegExp(const SourceCodeRange& range);

  const Node& NewAssertionRegExp(const SourceCodeRange& range,
                                 RegExpAssertionKind kind);

  const Node& NewCaptureRegExp(const SourceCodeRange& range,
                               const Node& pattern);

  const Node& NewCharSetRegExp(const SourceCodeRange& range);

  const Node& NewComplementCharSetRegExp(const SourceCodeRange& range);

  const Node& NewEmptyRegExp(const SourceCodeRange& range);

  const Node& NewInvalidRegExp(const SourceCodeRange& range, int error_code);

  const Node& NewLiteralRegExp(const SourceCodeRange& range);

  const Node& NewLookAheadRegExp(const SourceCodeRange& range,
                                 const Node& pattern);

  const Node& NewLookAheadNotRegExp(const SourceCodeRange& range,
                                    const Node& pattern);

  const Node& NewOrRegExp(const SourceCodeRange& range,
                          const std::vector<const Node*> patterns);

  const Node& NewRegExpRepeat(const SourceCodeRange& range,
                              RegExpRepeatMethod method,
                              int min,
                              int max);

  const Node& NewRepeatRegExp(const SourceCodeRange& range,
                              const Node& pattern,
                              const Node& repeat);

  const Node& NewSequenceRegExp(const SourceCodeRange& range,
                                const std::vector<const Node*> patterns);

  // Statement factory members
  const Node& NewBlockStatement(const SourceCodeRange& range,
                                const std::vector<const Node*>& statements);

  const Node& NewBreakStatement(const SourceCodeRange& range,
                                const Node& label);

  const Node& NewCaseClause(const SourceCodeRange& range,
                            const Node& expression,
                            const Node& statement);

  const Node& NewConstStatement(const SourceCodeRange& range,
                                const std::vector<const Node*>& elements);

  const Node& NewContinueStatement(const SourceCodeRange& range,
                                   const Node& label);

  const Node& NewDoStatement(const SourceCodeRange& range,
                             const Node& statement,
                             const Node& expression);

  const Node& NewEmptyStatement(const SourceCodeRange& range);

  const Node& NewExpressionStatement(const SourceCodeRange& range,
                                     const Node& expression);

  const Node& NewForStatement(const SourceCodeRange& range,
                              const Node& keyword,
                              const Node& init,
                              const Node& condition,
                              const Node& step,
                              const Node& body);

  const Node& NewForInStatement(const SourceCodeRange& range,
                                const Node& keyword,
                                const Node& expression,
                                const Node& body);

  const Node& NewForOfStatement(const SourceCodeRange& range,
                                const Node& keyword,
                                const Node& binding,
                                const Node& expression,
                                const Node& body);

  const Node& NewIfElseStatement(const SourceCodeRange& range,
                                 const Node& condition,
                                 const Node& then_clause,
                                 const Node& else_clause);

  const Node& NewIfStatement(const SourceCodeRange& range,
                             const Node& condition,
                             const Node& then_clause);

  const Node& NewInvalidStatement(const SourceCodeRange& range, int error_code);

  const Node& NewLabeledStatement(const SourceCodeRange& range,
                                  const Node& label,
                                  const Node& statement);

  const Node& NewLetStatement(const SourceCodeRange& range,
                              const std::vector<const Node*>& elements);

  const Node& NewReturnStatement(const SourceCodeRange& range,
                                 const Node& condition);

  const Node& NewSwitchStatement(const SourceCodeRange& range,
                                 const Node& expression,
                                 const std::vector<const Node*>& clauses);

  const Node& NewThrowStatement(const SourceCodeRange& range,
                                const Node& expression);

  const Node& NewTryCatchFinallyStatement(const SourceCodeRange& range,
                                          const Node& try_block,
                                          const Node& catch_parameter,
                                          const Node& catch_block,
                                          const Node& finally_block);

  const Node& NewTryCatchStatement(const SourceCodeRange& range,
                                   const Node& try_block,
                                   const Node& catch_parameter,
                                   const Node& catch_block);

  const Node& NewTryFinallyStatement(const SourceCodeRange& range,
                                     const Node& try_block,
                                     const Node& finally_block);

  const Node& NewVarStatement(const SourceCodeRange& range,
                              const std::vector<const Node*>& elements);

  const Node& NewWhileStatement(const SourceCodeRange& range,
                                const Node& expression,
                                const Node& statement);

  const Node& NewWithStatement(const SourceCodeRange& range,
                               const Node& expression,
                               const Node& statement);

  // Type factory members
  const Node& NewAnyType(const SourceCodeRange& range);

  const Node& NewFunctionType(const SourceCodeRange& range,
                              FunctionTypeKind kind,
                              const Node& parameter_list,
                              const Node& return_type);

  const Node& NewInvalidType(const SourceCodeRange& range);

  const Node& NewMemberType(const SourceCodeRange& range,
                            const Node& member,
                            const Node& name);

  const Node& NewNullableType(const SourceCodeRange& range, const Node& type);

  const Node& NewNonNullableType(const SourceCodeRange& range,
                                 const Node& type);

  const Node& NewOptionalType(const SourceCodeRange& range, const Node& type);

  const Node& NewPrimitiveType(const Node& name);

  const Node& NewRecordType(const SourceCodeRange& range,
                            const std::vector<const Node*>& members);

  const Node& NewRestType(const SourceCodeRange& range, const Node& type);

  const Node& NewTupleType(const SourceCodeRange& range,
                           const std::vector<const Node*>& members);

  const Node& NewTypeApplication(const SourceCodeRange& range,
                                 const Node& name,
                                 const Node& argument_list);

  const Node& NewTypeGroup(const SourceCodeRange& range, const Node& type);

  const Node& NewTypeName(const SourceCodeRange& range, const Node& name);

  const Node& NewUnionType(const SourceCodeRange& range,
                           const std::vector<const Node*>& members);

  const Node& NewUnknownType(const SourceCodeRange& range);

  const Node& NewVoidType(const SourceCodeRange& range);

 private:
  class NameIdMap;

  const Node& NewNode(const SourceCodeRange& range,
                      const Syntax& tag,
                      const std::vector<const Node*>& nodes);

  const Node& NewNode(const SourceCodeRange& range,
                      const Syntax& tag,
                      const Node& node0,
                      const std::vector<const Node*>& nodes);

  const Node& NewNode0(const SourceCodeRange& range, const Syntax& tag);

  const Node& NewNode1(const SourceCodeRange& range,
                       const Syntax& tag,
                       const Node& node0);

  const Node& NewNode2(const SourceCodeRange& range,
                       const Syntax& tag,
                       const Node& node0,
                       const Node& node1);

  const Node& NewNode3(const SourceCodeRange& range,
                       const Syntax& tag,
                       const Node& node0,
                       const Node& node1,
                       const Node& node2);

  const Node& NewNode4(const SourceCodeRange& range,
                       const Syntax& tag,
                       const Node& node0,
                       const Node& node1,
                       const Node& node2,
                       const Node& node3);

  std::unique_ptr<NameIdMap> name_id_map_;
  std::unique_ptr<SyntaxFactory> syntax_factory_;
  Zone& zone_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_FACTORY_H_
