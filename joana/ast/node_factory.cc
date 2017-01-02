// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/ast/node_factory.h"

#include "joana/ast/bindings.h"
#include "joana/ast/compilation_units.h"
#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/jsdoc_syntaxes.h"
#include "joana/ast/literals.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/syntax_factory.h"
#include "joana/ast/tokens.h"
#include "joana/ast/types.h"

namespace joana {
namespace ast {

namespace {

void* AllocateNode(Zone* zone, size_t arity) {
  return zone->Allocate(sizeof(Node) + sizeof(Node*) * arity);
}

}  // namespace

//
// NodeFactory::NameIdMap
//
class NodeFactory::NameIdMap {
 public:
  NameIdMap();
  ~NameIdMap();

  int Register(base::StringPiece16 name);

 private:
  void Populate();

  std::unordered_map<base::StringPiece16, int, base::StringPiece16Hash> map_;
  int last_id_ = 0;

  DISALLOW_COPY_AND_ASSIGN(NameIdMap);
};

NodeFactory::NameIdMap::NameIdMap() {
  Populate();
}

NodeFactory::NameIdMap::~NameIdMap() = default;

void NodeFactory::NameIdMap::Populate() {
  last_id_ = static_cast<int>(NameId::StartOfKeyword);
#define V(name, camel, upper) Register(base::StringPiece16(L## #name));
  FOR_EACH_JAVASCRIPT_KEYWORD(V)
#undef V

  last_id_ = static_cast<int>(NameId::StartOfKnownWord);
#define V(name, camel, upper) Register(base::StringPiece16(L## #name));
  FOR_EACH_JAVASCRIPT_KNOWN_WORD(V)
#undef V

  last_id_ = static_cast<int>(NameId::StartOfJsDocTagName);
#define V(name, camel, upper) Register(base::StringPiece16(L##"@" #name));
  FOR_EACH_JSDOC_TAG_NAME(V)
#undef V
}

int NodeFactory::NameIdMap::Register(base::StringPiece16 name) {
  const auto& it = map_.find(name);
  if (it != map_.end())
    return it->second;
  ++last_id_;
  map_.emplace(name, last_id_);
  return last_id_;
}

//
// NodeFactory implementations
//
NodeFactory::NodeFactory(Zone* zone)
    : name_id_map_(new NameIdMap()),
      syntax_factory_(new SyntaxFactory(zone)),
      zone_(*zone) {}

NodeFactory::~NodeFactory() = default;

const Node& NodeFactory::NewNode(const SourceCodeRange& range,
                                 const Syntax& tag,
                                 const std::vector<const Node*>& nodes) {
  const auto size = nodes.size();
  auto* const node = new (AllocateNode(&zone_, size)) Node(range, tag, size);
  auto* runner = &node->nodes_[0];
  for (const auto& child : nodes) {
    *runner = child;
    ++runner;
  }
  return *node;
}

const Node& NodeFactory::NewNode(const SourceCodeRange& range,
                                 const Syntax& tag,
                                 const Node& node0,
                                 const std::vector<const Node*>& nodes) {
  const auto size = nodes.size() + 1;
  auto* const node = new (AllocateNode(&zone_, size)) Node(range, tag, size);
  auto* runner = &node->nodes_[0];
  *runner = &node0;
  ++runner;
  for (const auto& child : nodes) {
    *runner = child;
    ++runner;
  }
  return *node;
}

const Node& NodeFactory::NewNode0(const SourceCodeRange& range,
                                  const Syntax& tag) {
  auto* node = new (AllocateNode(&zone_, 0)) Node(range, tag, 0);
  node->nodes_[0] = nullptr;
  return *node;
}

const Node& NodeFactory::NewNode1(const SourceCodeRange& range,
                                  const Syntax& tag,
                                  const Node& node0) {
  auto* const node = new (AllocateNode(&zone_, 1)) Node(range, tag, 1);
  node->nodes_[0] = &node0;
  return *node;
}

const Node& NodeFactory::NewNode2(const SourceCodeRange& range,
                                  const Syntax& tag,
                                  const Node& node0,
                                  const Node& node1) {
  auto* const node = new ((AllocateNode(&zone_, 2))) Node(range, tag, 2);
  node->nodes_[0] = &node0;
  node->nodes_[1] = &node1;
  return *node;
}

const Node& NodeFactory::NewNode3(const SourceCodeRange& range,
                                  const Syntax& tag,
                                  const Node& node0,
                                  const Node& node1,
                                  const Node& node2) {
  auto* const node = new (AllocateNode(&zone_, 3)) Node(range, tag, 3);
  node->nodes_[0] = &node0;
  node->nodes_[1] = &node1;
  node->nodes_[2] = &node2;
  return *node;
}

const Node& NodeFactory::NewNode4(const SourceCodeRange& range,
                                  const Syntax& tag,
                                  const Node& node0,
                                  const Node& node1,
                                  const Node& node2,
                                  const Node& node3) {
  auto* const node = new (AllocateNode(&zone_, 4)) Node(range, tag, 4);
  node->nodes_[0] = &node0;
  node->nodes_[1] = &node1;
  node->nodes_[2] = &node2;
  node->nodes_[3] = &node3;
  return *node;
}

const Node& NodeFactory::NewTuple(const SourceCodeRange& range,
                                  const std::vector<const Node*>& nodes) {
  return NewNode(range, syntax_factory_->NewTuple(), nodes);
}

// Compilation units
const Node& NodeFactory::NewExterns(
    const SourceCodeRange& range,
    const std::vector<const Node*>& statements) {
  return NewNode(range, syntax_factory_->NewExterns(), statements);
}

const Node& NodeFactory::NewModule(const SourceCodeRange& range,
                                   const std::vector<const Node*>& statements) {
  return NewNode(range, syntax_factory_->NewModule(), statements);
}

const Node& NodeFactory::NewScript(const SourceCodeRange& range,
                                   const std::vector<const Node*>& statements) {
  return NewNode(range, syntax_factory_->NewScript(), statements);
}

// Tokens
const Node& NodeFactory::NewComment(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewComment());
}

const Node& NodeFactory::NewEmpty(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewEmpty());
}

const Node& NodeFactory::NewInvalid(const SourceCodeRange& range,
                                    int error_code) {
  return NewNode0(range, syntax_factory_->NewInvalid(error_code));
}

const Node& NodeFactory::NewName(const SourceCodeRange& range, NameId name_id) {
  DCHECK_EQ(name_id, NameId::YieldStar);
  return NewNode0(range, syntax_factory_->NewName(static_cast<int>(name_id)));
}

const Node& NodeFactory::NewName(const SourceCodeRange& range) {
  const auto name_id = name_id_map_->Register(range.GetString());
  return NewNode0(range, syntax_factory_->NewName(name_id));
}

const Node& NodeFactory::NewPunctuator(const SourceCodeRange& range,
                                       PunctuatorKind kind) {
  return NewNode0(range, syntax_factory_->NewPunctuator(kind));
}

// Bindings
const Node& NodeFactory::NewArrayBindingPattern(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements,
    const Node& initializer) {
  return NewNode(range, syntax_factory_->NewArrayBindingPattern(), initializer,
                 elements);
}

const Node& NodeFactory::NewBindingCommaElement(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewBindingCommaElement());
}

const Node& NodeFactory::NewBindingInvalidElement(
    const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewBindingInvalidElement());
}

const Node& NodeFactory::NewBindingNameElement(const SourceCodeRange& range,
                                               const Node& name,
                                               const Node& initializer) {
  return NewNode2(range, syntax_factory_->NewBindingNameElement(), name,
                  initializer);
}

const Node& NodeFactory::NewBindingProperty(const SourceCodeRange& range,
                                            const Node& name,
                                            const Node& element) {
  return NewNode2(range, syntax_factory_->NewBindingProperty(), name, element);
}

const Node& NodeFactory::NewBindingRestElement(const SourceCodeRange& range,
                                               const Node& element) {
  return NewNode1(range, syntax_factory_->NewBindingRestElement(), element);
}

const Node& NodeFactory::NewObjectBindingPattern(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements,
    const Node& initializer) {
  return NewNode(range, syntax_factory_->NewObjectBindingPattern(), initializer,
                 elements);
}

// Declarations
const Node& NodeFactory::NewAnnotation(const SourceCodeRange& range,
                                       const Node& annotation,
                                       const Node& annotated) {
  DCHECK_EQ(annotation, SyntaxCode::JsDocDocument);
  return NewNode2(range, syntax_factory_->NewAnnotation(), annotation,
                  annotated);
}

const Node& NodeFactory::NewArrowFunction(const SourceCodeRange& range,
                                          FunctionKind kind,
                                          const Node& parameter_list,
                                          const Node& body) {
  DCHECK(parameter_list == SyntaxCode::ParameterList ||
         parameter_list == ast::SyntaxCode::ReferenceExpression)
      << parameter_list;
  return NewNode2(range, syntax_factory_->NewArrowFunction(kind),
                  parameter_list, body);
}

const Node& NodeFactory::NewClass(const SourceCodeRange& range,
                                  const Node& name,
                                  const Node& heritage,
                                  const Node& body) {
  return NewNode3(range, syntax_factory_->NewClass(), name, heritage, body);
}

const Node& NodeFactory::NewFunction(const SourceCodeRange& range,
                                     FunctionKind kind,
                                     const Node& name,
                                     const Node& parameter_list,
                                     const Node& body) {
  DCHECK_EQ(parameter_list, SyntaxCode::ParameterList);
  return NewNode3(range, syntax_factory_->NewFunction(kind), name,
                  parameter_list, body);
}

const Node& NodeFactory::NewMethod(const SourceCodeRange& range,
                                   MethodKind method_kind,
                                   FunctionKind kind,
                                   const Node& name,
                                   const Node& parameter_list,
                                   const Node& method_body) {
  DCHECK_EQ(parameter_list, SyntaxCode::ParameterList);
  return NewNode3(range, syntax_factory_->NewMethod(method_kind, kind), name,
                  parameter_list, method_body);
}

// Expressions
const Node& NodeFactory::NewArrayInitializer(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements) {
  return NewNode(range, syntax_factory_->NewArrayInitializer(), elements);
}

const Node& NodeFactory::NewAssignmentExpression(const SourceCodeRange& range,
                                                 const Node& op,
                                                 const Node& lhs,
                                                 const Node& rhs) {
  return NewNode3(range, syntax_factory_->NewAssignmentExpression(
                             PunctuatorSyntax::KindOf(op)),
                  lhs, op, rhs);
}

const Node& NodeFactory::NewBinaryExpression(const SourceCodeRange& range,
                                             const Node& op,
                                             const Node& lhs,
                                             const Node& rhs) {
  return NewNode3(
      range, syntax_factory_->NewBinaryExpression(PunctuatorSyntax::KindOf(op)),
      lhs, op, rhs);
}

const Node& NodeFactory::NewBinaryKeywordExpression(
    const SourceCodeRange& range,
    const Node& op,
    const Node& lhs,
    const Node& rhs) {
  return NewNode3(range, syntax_factory_->NewBinaryKeywordExpression(
                             static_cast<NameId>(NameSyntax::IdOf(op))),
                  lhs, op, rhs);
}

const Node& NodeFactory::NewCallExpression(
    const SourceCodeRange& range,
    const Node& callee,
    const std::vector<const Node*>& argument_list) {
  return NewNode(range, syntax_factory_->NewCallExpression(), callee,
                 argument_list);
}

const Node& NodeFactory::NewCommaExpression(
    const SourceCodeRange& range,
    const std::vector<const Node*>& expressions) {
  return NewNode(range, syntax_factory_->NewCommaExpression(), expressions);
}

const Node& NodeFactory::NewComputedMemberExpression(
    const SourceCodeRange& range,
    const Node& expression,
    const Node& name_expression) {
  return NewNode2(range, syntax_factory_->NewComputedMemberExpression(),
                  expression, name_expression);
}

const Node& NodeFactory::NewConditionalExpression(
    const SourceCodeRange& range,
    const Node& condition,
    const Node& true_expression,
    const Node& false_expression) {
  return NewNode3(range, syntax_factory_->NewConditionalExpression(), condition,
                  true_expression, false_expression);
}

const Node& NodeFactory::NewDelimiterExpression(const SourceCodeRange& range) {
  DCHECK(!range.IsCollapsed()) << range;
  return NewNode0(range, syntax_factory_->NewDelimiterExpression());
}

const Node& NodeFactory::NewElisionExpression(const SourceCodeRange& range) {
  DCHECK(range.IsCollapsed()) << range;
  return NewNode0(range, syntax_factory_->NewElisionExpression());
}

const Node& NodeFactory::NewGroupExpression(const SourceCodeRange& range,
                                            const Node& expression) {
  return NewNode1(range, syntax_factory_->NewGroupExpression(), expression);
}

const Node& NodeFactory::NewMemberExpression(const SourceCodeRange& range,
                                             const Node& expression,
                                             const Node& name) {
  return NewNode2(range, syntax_factory_->NewMemberExpression(), expression,
                  name);
}

const Node& NodeFactory::NewNewExpression(
    const SourceCodeRange& range,
    const Node& callee,
    const std::vector<const Node*>& argument_list) {
  return NewNode(range, syntax_factory_->NewNewExpression(), callee,
                 argument_list);
}

const Node& NodeFactory::NewObjectInitializer(
    const SourceCodeRange& range,
    const std::vector<const Node*>& properties) {
  return NewNode(range, syntax_factory_->NewObjectInitializer(), properties);
}

const Node& NodeFactory::NewParameterList(
    const SourceCodeRange& range,
    const std::vector<const Node*>& parameters) {
  return NewNode(range, syntax_factory_->NewParameterList(), parameters);
}

const Node& NodeFactory::NewProperty(const SourceCodeRange& range,
                                     const Node& name,
                                     const Node& value) {
  return NewNode2(range, syntax_factory_->NewProperty(), name, value);
}

const Node& NodeFactory::NewReferenceExpression(const Node& name) {
  return NewNode1(name.range(), syntax_factory_->NewReferenceExpression(),
                  name);
}

const Node& NodeFactory::NewRegExpLiteralExpression(
    const SourceCodeRange& range,
    const Node& regexp,
    const Node& flags) {
  return NewNode2(range, syntax_factory_->NewRegExpLiteralExpression(), regexp,
                  flags);
}

const Node& NodeFactory::NewUnaryExpression(const SourceCodeRange& range,
                                            const Node& op,
                                            const Node& expression) {
  return NewNode2(
      range, syntax_factory_->NewUnaryExpression(PunctuatorSyntax::KindOf(op)),
      op, expression);
}

const Node& NodeFactory::NewUnaryKeywordExpression(const SourceCodeRange& range,
                                                   const Node& op,
                                                   const Node& expression) {
  DCHECK(NameSyntax::IsKeyword(op)) << op;
  return NewNode2(range, syntax_factory_->NewUnaryKeywordExpression(
                             static_cast<NameId>(NameSyntax::IdOf(op))),
                  op, expression);
}

// JsDoc
const Node& NodeFactory::NewJsDocDocument(
    const SourceCodeRange& range,
    const std::vector<const Node*>& nodes) {
  return NewNode(range, syntax_factory_->NewJsDocDocument(), nodes);
}

const Node& NodeFactory::NewJsDocTag(const SourceCodeRange& range,
                                     const Node& name,
                                     const std::vector<const Node*>& operands) {
  DCHECK_EQ(name, SyntaxCode::Name);
  return NewNode(range, syntax_factory_->NewJsDocTag(), name, operands);
}

const Node& NodeFactory::NewJsDocText(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewJsDocText());
}

// Literals
const Node& NodeFactory::NewBooleanLiteral(const Node& name, bool value) {
  return NewNode0(name.range(), syntax_factory_->NewBooleanLiteral(value));
}

const Node& NodeFactory::NewNullLiteral(const Node& name) {
  return NewNode0(name.range(), syntax_factory_->NewNullLiteral());
}

const Node& NodeFactory::NewNumericLiteral(const SourceCodeRange& range,
                                           double value) {
  return NewNode0(range, syntax_factory_->NewNumericLiteral(value));
}

const Node& NodeFactory::NewStringLiteral(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewStringLiteral());
}

const Node& NodeFactory::NewUndefinedLiteral(const Node& name) {
  return NewNode0(name.range(), syntax_factory_->NewUndefinedLiteral());
}

// RegExp
const Node& NodeFactory::NewAnyCharRegExp(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewAnyCharRegExp());
}

const Node& NodeFactory::NewAssertionRegExp(const SourceCodeRange& range,
                                            RegExpAssertionKind kind) {
  return NewNode0(range, syntax_factory_->NewAssertionRegExp(kind));
}

const Node& NodeFactory::NewCaptureRegExp(const SourceCodeRange& range,
                                          const Node& pattern) {
  return NewNode1(range, syntax_factory_->NewCaptureRegExp(), pattern);
}

const Node& NodeFactory::NewCharSetRegExp(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewCharSetRegExp());
}

const Node& NodeFactory::NewComplementCharSetRegExp(
    const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewComplementCharSetRegExp());
}

const Node& NodeFactory::NewGreedyRepeatRegExp(const SourceCodeRange& range,
                                               const Node& pattern,
                                               const RegExpRepeat& repeat) {
  return NewNode1(range, syntax_factory_->NewGreedyRepeatRegExp(repeat),
                  pattern);
}

const Node& NodeFactory::NewInvalidRegExp(const SourceCodeRange& range,
                                          int error_code) {
  return NewNode0(range, syntax_factory_->NewInvalidRegExp());
}

const Node& NodeFactory::NewLazyRepeatRegExp(const SourceCodeRange& range,
                                             const Node& pattern,
                                             const RegExpRepeat& repeat) {
  return NewNode1(range, syntax_factory_->NewLazyRepeatRegExp(repeat), pattern);
}

const Node& NodeFactory::NewLiteralRegExp(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewLiteralRegExp());
}

const Node& NodeFactory::NewLookAheadRegExp(const SourceCodeRange& range,
                                            const Node& pattern) {
  return NewNode1(range, syntax_factory_->NewLookAheadRegExp(), pattern);
}

const Node& NodeFactory::NewLookAheadNotRegExp(const SourceCodeRange& range,
                                               const Node& pattern) {
  return NewNode1(range, syntax_factory_->NewLookAheadNotRegExp(), pattern);
}

const Node& NodeFactory::NewOrRegExp(const SourceCodeRange& range,
                                     const std::vector<const Node*> patterns) {
  return NewNode(range, syntax_factory_->NewOrRegExp(), patterns);
}

const Node& NodeFactory::NewSequenceRegExp(
    const SourceCodeRange& range,
    const std::vector<const Node*> patterns) {
  return NewNode(range, syntax_factory_->NewSequenceRegExp(), patterns);
}

// Statements factory members
const Node& NodeFactory::NewBlockStatement(
    const SourceCodeRange& range,
    const std::vector<const Node*>& statements) {
  return NewNode(range, syntax_factory_->NewBlockStatement(), statements);
}

const Node& NodeFactory::NewBreakStatement(const SourceCodeRange& range,
                                           const Node& label) {
  return NewNode1(range, syntax_factory_->NewBreakStatement(), label);
}

const Node& NodeFactory::NewCaseClause(const SourceCodeRange& range,
                                       const Node& expression,
                                       const Node& statement) {
  return NewNode2(range, syntax_factory_->NewCaseClause(), expression,
                  statement);
}

const Node& NodeFactory::NewConstStatement(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements) {
  return NewNode(range, syntax_factory_->NewConstStatement(), elements);
}

const Node& NodeFactory::NewContinueStatement(const SourceCodeRange& range,
                                              const Node& label) {
  return NewNode1(range, syntax_factory_->NewContinueStatement(), label);
}

const Node& NodeFactory::NewDoStatement(const SourceCodeRange& range,
                                        const Node& statement,
                                        const Node& expression) {
  return NewNode2(range, syntax_factory_->NewDoStatement(), statement,
                  expression);
}

const Node& NodeFactory::NewEmptyStatement(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewEmptyStatement());
}

const Node& NodeFactory::NewExpressionStatement(const SourceCodeRange& range,
                                                const Node& expression) {
  return NewNode1(range, syntax_factory_->NewExpressionStatement(), expression);
}

const Node& NodeFactory::NewForStatement(const SourceCodeRange& range,
                                         const Node& keyword,
                                         const Node& init,
                                         const Node& condition,
                                         const Node& step,
                                         const Node& body) {
  return NewNode4(range, syntax_factory_->NewForStatement(), init, condition,
                  step, body);
}

const Node& NodeFactory::NewForInStatement(const SourceCodeRange& range,
                                           const Node& keyword,
                                           const Node& expression,
                                           const Node& body) {
  return NewNode2(range, syntax_factory_->NewForInStatement(), expression,
                  body);
}

const Node& NodeFactory::NewForOfStatement(const SourceCodeRange& range,
                                           const Node& keyword,
                                           const Node& binding,
                                           const Node& expression,
                                           const Node& body) {
  return NewNode3(range, syntax_factory_->NewForOfStatement(), binding,
                  expression, body);
}

const Node& NodeFactory::NewIfElseStatement(const SourceCodeRange& range,
                                            const Node& expression,
                                            const Node& then_clause,
                                            const Node& else_clause) {
  return NewNode3(range, syntax_factory_->NewIfElseStatement(), expression,
                  then_clause, else_clause);
}

const Node& NodeFactory::NewIfStatement(const SourceCodeRange& range,
                                        const Node& expression,
                                        const Node& then_clause) {
  return NewNode2(range, syntax_factory_->NewIfStatement(), expression,
                  then_clause);
}

const Node& NodeFactory::NewInvalidStatement(const SourceCodeRange& range,
                                             int error_code) {
  return NewNode0(range, syntax_factory_->NewInvalidStatement());
}

const Node& NodeFactory::NewLabeledStatement(const SourceCodeRange& range,
                                             const Node& label,
                                             const Node& statement) {
  return NewNode1(range, syntax_factory_->NewLabeledStatement(), statement);
}

const Node& NodeFactory::NewLetStatement(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements) {
  return NewNode(range, syntax_factory_->NewLetStatement(), elements);
}

const Node& NodeFactory::NewReturnStatement(const SourceCodeRange& range,
                                            const Node& expression) {
  return NewNode1(range, syntax_factory_->NewReturnStatement(), expression);
}

const Node& NodeFactory::NewSwitchStatement(
    const SourceCodeRange& range,
    const Node& expression,
    const std::vector<const Node*>& clauses) {
  return NewNode(range, syntax_factory_->NewSwitchStatement(), expression,
                 clauses);
}

const Node& NodeFactory::NewThrowStatement(const SourceCodeRange& range,
                                           const Node& expression) {
  return NewNode1(range, syntax_factory_->NewThrowStatement(), expression);
}

const Node& NodeFactory::NewTryCatchFinallyStatement(
    const SourceCodeRange& range,
    const Node& try_block,
    const Node& catch_parameter,
    const Node& catch_block,
    const Node& finally_block) {
  return NewNode4(range, syntax_factory_->NewTryCatchFinallyStatement(),
                  try_block, catch_parameter, catch_block, finally_block);
}

const Node& NodeFactory::NewTryCatchStatement(const SourceCodeRange& range,
                                              const Node& try_block,
                                              const Node& catch_parameter,
                                              const Node& catch_block) {
  return NewNode3(range, syntax_factory_->NewTryCatchStatement(), try_block,
                  catch_parameter, catch_block);
}

const Node& NodeFactory::NewTryFinallyStatement(const SourceCodeRange& range,
                                                const Node& try_block,
                                                const Node& finally_block) {
  return NewNode2(range, syntax_factory_->NewTryFinallyStatement(), try_block,
                  finally_block);
}

const Node& NodeFactory::NewVarStatement(
    const SourceCodeRange& range,
    const std::vector<const Node*>& elements) {
  return NewNode(range, syntax_factory_->NewVarStatement(), elements);
}

const Node& NodeFactory::NewWhileStatement(const SourceCodeRange& range,
                                           const Node& expression,
                                           const Node& statement) {
  return NewNode2(range, syntax_factory_->NewWhileStatement(), expression,
                  statement);
}

const Node& NodeFactory::NewWithStatement(const SourceCodeRange& range,
                                          const Node& expression,
                                          const Node& statement) {
  return NewNode2(range, syntax_factory_->NewWithStatement(), expression,
                  statement);
}

// Type factory members
const Node& NodeFactory::NewAnyType(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewAnyType());
}

const Node& NodeFactory::NewFunctionType(const SourceCodeRange& range,
                                         FunctionTypeKind kind,
                                         const Node& parameter_list,
                                         const Node& return_type) {
  return NewNode2(range, syntax_factory_->NewFunctionType(kind), parameter_list,
                  return_type);
}

const Node& NodeFactory::NewInvalidType(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewInvalidType());
}

const Node& NodeFactory::NewNullableType(const SourceCodeRange& range,
                                         const Node& type) {
  return NewNode1(range, syntax_factory_->NewNullableType(), type);
}

const Node& NodeFactory::NewNonNullableType(const SourceCodeRange& range,
                                            const Node& type) {
  return NewNode1(range, syntax_factory_->NewNonNullableType(), type);
}

const Node& NodeFactory::NewOptionalType(const SourceCodeRange& range,
                                         const Node& type) {
  return NewNode1(range, syntax_factory_->NewOptionalType(), type);
}

const Node& NodeFactory::NewRecordType(
    const SourceCodeRange& range,
    const std::vector<const Node*>& members) {
  return NewNode(range, syntax_factory_->NewRecordType(), members);
}

const Node& NodeFactory::NewRestType(const SourceCodeRange& range,
                                     const Node& type) {
  return NewNode1(range, syntax_factory_->NewRestType(), type);
}

const Node& NodeFactory::NewTupleType(const SourceCodeRange& range,
                                      const std::vector<const Node*>& members) {
  return NewNode(range, syntax_factory_->NewTupleType(), members);
}

const Node& NodeFactory::NewTypeApplication(const SourceCodeRange& range,
                                            const Node& name,
                                            const Node& argument_list) {
  DCHECK_EQ(name, SyntaxCode::TypeName);
  DCHECK_EQ(argument_list, SyntaxCode::Tuple);
  return NewNode2(range, syntax_factory_->NewTypeApplication(), name,
                  argument_list);
}

const Node& NodeFactory::NewTypeGroup(const SourceCodeRange& range,
                                      const Node& type) {
  return NewNode1(range, syntax_factory_->NewTypeGroup(), type);
}

const Node& NodeFactory::NewTypeName(const SourceCodeRange& range,
                                     const Node& name) {
  DCHECK_EQ(name, SyntaxCode::Name);
  return NewNode1(range, syntax_factory_->NewTypeName(), name);
}

const Node& NodeFactory::NewUnionType(const SourceCodeRange& range,
                                      const std::vector<const Node*>& members) {
  return NewNode(range, syntax_factory_->NewUnionType(), members);
}

const Node& NodeFactory::NewUnknownType(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewUnknownType());
}

const Node& NodeFactory::NewVoidType(const SourceCodeRange& range) {
  return NewNode0(range, syntax_factory_->NewVoidType());
}

}  // namespace ast
}  // namespace joana
