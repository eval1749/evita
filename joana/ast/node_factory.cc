// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/ast/node_factory.h"

#include "joana/ast/declarations.h"
#include "joana/ast/expressions.h"
#include "joana/ast/literals.h"
#include "joana/ast/module.h"
#include "joana/ast/regexp.h"
#include "joana/ast/statements.h"
#include "joana/ast/tokens.h"

namespace joana {
namespace ast {

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
    : name_id_map_(new NameIdMap()), zone_(zone) {}

NodeFactory::~NodeFactory() = default;

// Nodes
Comment& NodeFactory::NewComment(const SourceCodeRange& range) {
  return *new (zone_) Comment(range);
}

Empty& NodeFactory::NewEmpty(const SourceCodeRange& range) {
  return *new (zone_) Empty(range);
}

Invalid& NodeFactory::NewInvalid(const SourceCodeRange& range, int error_code) {
  return *new (zone_) Invalid(range, error_code);
}

Module& NodeFactory::NewModule(const SourceCodeRange& range) {
  return *new (zone_) Module(range);
}

Name& NodeFactory::NewName(const SourceCodeRange& range, NameId name_id) {
  DCHECK_EQ(name_id, NameId::YieldStar);
  return *new (zone_) Name(range, static_cast<int>(name_id));
}

Name& NodeFactory::NewName(const SourceCodeRange& range) {
  return *new (zone_) Name(range, name_id_map_->Register(range.GetString()));
}

Punctuator& NodeFactory::NewPunctuator(const SourceCodeRange& range,
                                       PunctuatorKind kind) {
  return *new (zone_) Punctuator(range, kind);
}

// Declarations
ArrowFunction& NodeFactory::NewArrowFunction(const SourceCodeRange& range,
                                             FunctionKind kind,
                                             const Expression& parameter_list,
                                             const ArrowFunctionBody& body) {
  return *new (zone_)
      ArrowFunction(range, kind, const_cast<Expression*>(&parameter_list),
                    const_cast<ArrowFunctionBody*>(&body));
}

Class& NodeFactory::NewClass(const SourceCodeRange& range,
                             const Token& name,
                             const Expression& heritage,
                             const Expression& body) {
  return *new (zone_)
      Class(range, const_cast<Token*>(&name),
            const_cast<Expression*>(&heritage), const_cast<Expression*>(&body));
}

Function& NodeFactory::NewFunction(const SourceCodeRange& range,
                                   FunctionKind kind,
                                   const Token& name,
                                   const Expression& parameter_list,
                                   const Statement& body) {
  return *new (zone_) Function(range, kind, const_cast<Token*>(&name),
                               const_cast<Expression*>(&parameter_list),
                               const_cast<Statement*>(&body));
}

Method& NodeFactory::NewMethod(const SourceCodeRange& range,
                               MethodKind is_static,
                               FunctionKind kind,
                               const Expression& name,
                               const Expression& parameter_list,
                               const Statement& method_body) {
  return *new (zone_)
      Method(range, is_static, kind, const_cast<Expression*>(&name),
             const_cast<Expression*>(&parameter_list),
             const_cast<Statement*>(&method_body));
}

// Expressions
ArrayLiteralExpression& NodeFactory::NewArrayLiteralExpression(
    const SourceCodeRange& range,
    const std::vector<Expression*>& elements) {
  auto* const list = new (zone_) ExpressionList(zone_, elements);
  return *new (zone_) ArrayLiteralExpression(range, *list);
}

AssignmentExpression& NodeFactory::NewAssignmentExpression(
    const SourceCodeRange& range,
    const Punctuator& op,
    const Expression& left_hand_side,
    const Expression& right_hand_side) {
  return *new (zone_)
      AssignmentExpression(range, const_cast<Punctuator*>(&op),
                           const_cast<Expression*>(&left_hand_side),
                           const_cast<Expression*>(&right_hand_side));
}

BinaryExpression& NodeFactory::NewBinaryExpression(
    const SourceCodeRange& range,
    const Token& op,
    const Expression& left_hand_side,
    const Expression& right_hand_side) {
  return *new (zone_) BinaryExpression(
      range, const_cast<Token*>(&op), const_cast<Expression*>(&left_hand_side),
      const_cast<Expression*>(&right_hand_side));
}

CallExpression& NodeFactory::NewCallExpression(
    const SourceCodeRange& range,
    const Expression& callee,
    const std::vector<Expression*>& arguments) {
  auto* const list = new (zone_) ExpressionList(zone_, arguments);
  return *new (zone_)
      CallExpression(range, const_cast<Expression*>(&callee), list);
}

CommaExpression& NodeFactory::NewCommaExpression(
    const SourceCodeRange& range,
    const std::vector<Expression*> expressions) {
  auto* const list = new (zone_) ExpressionList(zone_, expressions);
  return *new (zone_) CommaExpression(range, list);
}

ComputedMemberExpression& NodeFactory::NewComputedMemberExpression(
    const SourceCodeRange& range,
    const Expression& expression,
    const Expression& name_expression) {
  return *new (zone_)
      ComputedMemberExpression(range, const_cast<Expression*>(&expression),
                               const_cast<Expression*>(&name_expression));
}

ConditionalExpression& NodeFactory::NewConditionalExpression(
    const SourceCodeRange& range,
    const Expression& condition,
    const Expression& true_expression,
    const Expression& false_expression) {
  return *new (zone_)
      ConditionalExpression(range, const_cast<Expression*>(&condition),
                            const_cast<Expression*>(&true_expression),
                            const_cast<Expression*>(&false_expression));
}

DeclarationExpression& NodeFactory::NewDeclarationExpression(
    const Declaration& declaration) {
  return *new (zone_)
      DeclarationExpression(const_cast<Declaration*>(&declaration));
}

ElisionExpression& NodeFactory::NewElisionExpression(
    const SourceCodeRange& range) {
  DCHECK_EQ(range.start(), range.end()) << range;
  return *new (zone_) ElisionExpression(range);
}

EmptyExpression& NodeFactory::NewEmptyExpression(const SourceCodeRange& range) {
  return *new (zone_) EmptyExpression(range);
}

GroupExpression& NodeFactory::NewGroupExpression(const SourceCodeRange& range,
                                                 const Expression& expression) {
  return *new (zone_)
      GroupExpression(range, const_cast<Expression*>(&expression));
}

InvalidExpression& NodeFactory::NewInvalidExpression(const Node& node,
                                                     int error_code) {
  return *new (zone_) InvalidExpression(node, error_code);
}

LiteralExpression& NodeFactory::NewLiteralExpression(const Literal& literal) {
  return *new (zone_) LiteralExpression(const_cast<Literal*>(&literal));
}

NewExpression& NodeFactory::NewNewExpression(
    const SourceCodeRange& range,
    const Expression& expression,
    const std::vector<Expression*>& arguments) {
  auto* const list = new (zone_) ExpressionList(zone_, arguments);
  return *new (zone_)
      NewExpression(range, const_cast<Expression*>(&expression), list);
}

ObjectLiteralExpression& NodeFactory::NewObjectLiteralExpression(
    const SourceCodeRange& range,
    const std::vector<Expression*>& elements) {
  auto* const list = new (zone_) ExpressionList(zone_, elements);
  return *new (zone_) ObjectLiteralExpression(range, list);
}

MemberExpression& NodeFactory::NewMemberExpression(const SourceCodeRange& range,
                                                   const Expression& expression,
                                                   const Name& name) {
  return *new (zone_) MemberExpression(
      range, const_cast<Expression*>(&expression), const_cast<Name*>(&name));
}

PropertyDefinitionExpression& NodeFactory::NewPropertyDefinitionExpression(
    const SourceCodeRange& range,
    const Expression& name,
    const Expression& value) {
  return *new (zone_) PropertyDefinitionExpression(
      range, const_cast<Expression*>(&name), const_cast<Expression*>(&value));
}

ReferenceExpression& NodeFactory::NewReferenceExpression(const Name& name) {
  return *new (zone_) ReferenceExpression(const_cast<Name*>(&name));
}

RegExpLiteralExpression& NodeFactory::NewRegExpLiteralExpression(
    const SourceCodeRange& range,
    const RegExp& regexp,
    const Token& flags) {
  return *new (zone_) RegExpLiteralExpression(
      range, const_cast<RegExp*>(&regexp), const_cast<Token*>(&flags));
}

UnaryExpression& NodeFactory::NewUnaryExpression(const SourceCodeRange& range,
                                                 const Token& op,
                                                 const Expression& expression) {
  return *new (zone_) UnaryExpression(range, const_cast<Token*>(&op),
                                      const_cast<Expression*>(&expression));
}

// Literals
BooleanLiteral& NodeFactory::NewBooleanLiteral(const Name& name, bool value) {
  return *new (zone_) BooleanLiteral(name, value);
}

NullLiteral& NodeFactory::NewNullLiteral(const Name& name) {
  return *new (zone_) NullLiteral(name);
}

NumericLiteral& NodeFactory::NewNumericLiteral(const SourceCodeRange& range,
                                               double value) {
  return *new (zone_) NumericLiteral(range, value);
}

StringLiteral& NodeFactory::NewStringLiteral(const SourceCodeRange& range,
                                             base::StringPiece16 data) {
  return *new (zone_) StringLiteral(range, data);
}

UndefinedLiteral& NodeFactory::NewUndefinedLiteral(const Name& name) {
  return *new (zone_) UndefinedLiteral(name);
}

// RegExp
RegExp& NodeFactory::NewAnyCharRegExp(const SourceCodeRange& range) {
  return *new (zone_) AnyCharRegExp(range);
}

RegExp& NodeFactory::NewAssertionRegExp(const SourceCodeRange& range,
                                        RegExpAssertionKind kind) {
  return *new (zone_) AssertionRegExp(range, kind);
}

RegExp& NodeFactory::NewCaptureRegExp(const SourceCodeRange& range,
                                      const RegExp& pattern) {
  return *new (zone_) CaptureRegExp(range, const_cast<RegExp*>(&pattern));
}

RegExp& NodeFactory::NewCharSetRegExp(const SourceCodeRange& range) {
  return *new (zone_) CharSetRegExp(range);
}

RegExp& NodeFactory::NewComplementCharSetRegExp(const SourceCodeRange& range) {
  return *new (zone_) ComplementCharSetRegExp(range);
}

RegExp& NodeFactory::NewGreedyRepeatRegExp(const SourceCodeRange& range,
                                           const ast::RegExp& pattern,
                                           const RegExpRepeat& repeat) {
  return *new (zone_)
      GreedyRepeatRegExp(range, const_cast<RegExp*>(&pattern), repeat);
}

RegExp& NodeFactory::NewInvalidRegExp(const SourceCodeRange& range,
                                      int error_code) {
  return *new (zone_) InvalidRegExp(range, error_code);
}

RegExp& NodeFactory::NewLazyRepeatRegExp(const SourceCodeRange& range,
                                         const ast::RegExp& pattern,
                                         const RegExpRepeat& repeat) {
  return *new (zone_)
      LazyRepeatRegExp(range, const_cast<RegExp*>(&pattern), repeat);
}

RegExp& NodeFactory::NewLiteralRegExp(const SourceCodeRange& range) {
  return *new (zone_) LiteralRegExp(range);
}

RegExp& NodeFactory::NewLookAheadRegExp(const SourceCodeRange& range,
                                        const ast::RegExp& pattern) {
  return *new (zone_) LookAheadRegExp(range, const_cast<RegExp*>(&pattern));
}

RegExp& NodeFactory::NewLookAheadNotRegExp(const SourceCodeRange& range,
                                           const ast::RegExp& pattern) {
  return *new (zone_) LookAheadNotRegExp(range, const_cast<RegExp*>(&pattern));
}

RegExp& NodeFactory::NewOrRegExp(const SourceCodeRange& range,
                                 const std::vector<RegExp*> patterns) {
  auto* const list = new (zone_) RegExpList(zone_, patterns);
  return *new (zone_) OrRegExp(range, list);
}

RegExp& NodeFactory::NewSequenceRegExp(const SourceCodeRange& range,
                                       const std::vector<RegExp*> patterns) {
  auto* const list = new (zone_) RegExpList(zone_, patterns);
  return *new (zone_) SequenceRegExp(range, list);
}

// Statements factory members
BlockStatement& NodeFactory::NewBlockStatement(
    const SourceCodeRange& range,
    const std::vector<Statement*>& statements) {
  auto* const list = new (zone_) StatementList(zone_, statements);
  return *new (zone_) BlockStatement(range, list);
}

BreakStatement& NodeFactory::NewBreakStatement(const SourceCodeRange& range,
                                               const Token& label) {
  return *new (zone_) BreakStatement(range, const_cast<Token*>(&label));
}

CaseClause& NodeFactory::NewCaseClause(const SourceCodeRange& range,
                                       const Expression& expression,
                                       const Statement& statement) {
  return *new (zone_) CaseClause(range, const_cast<Expression*>(&expression),
                                 const_cast<Statement*>(&statement));
}

ConstStatement& NodeFactory::NewConstStatement(const SourceCodeRange& range,
                                               const Expression& expression) {
  return *new (zone_)
      ConstStatement(range, const_cast<Expression*>(&expression));
}

ContinueStatement& NodeFactory::NewContinueStatement(
    const SourceCodeRange& range,
    const Token& label) {
  return *new (zone_) ContinueStatement(range, const_cast<Token*>(&label));
}

DeclarationStatement& NodeFactory::NewDeclarationStatement(
    const Declaration& declaration) {
  return *new (zone_)
      DeclarationStatement(const_cast<Declaration*>(&declaration));
}

DoStatement& NodeFactory::NewDoStatement(const SourceCodeRange& range,
                                         const Statement& statement,
                                         const Expression& expression) {
  return *new (zone_)
      DoStatement(range, const_cast<ast::Statement*>(&statement),
                  const_cast<Expression*>(&expression));
}

EmptyStatement& NodeFactory::NewEmptyStatement(const SourceCodeRange& range) {
  return *new (zone_) EmptyStatement(range);
}

ExpressionStatement& NodeFactory::NewExpressionStatement(
    const Expression& expression) {
  return *new (zone_) ExpressionStatement(const_cast<Expression*>(&expression));
}

ForStatement& NodeFactory::NewForStatement(const SourceCodeRange& range,
                                           const Token& keyword,
                                           const Expression& init,
                                           const Expression& condition,
                                           const Expression& step,
                                           const Statement& body) {
  return *new (zone_) ForStatement(
      range, const_cast<Token*>(&keyword), const_cast<Expression*>(&init),
      const_cast<Expression*>(&condition), const_cast<Expression*>(&step),
      const_cast<Statement*>(&body));
}

ForInStatement& NodeFactory::NewForInStatement(const SourceCodeRange& range,
                                               const Token& keyword,
                                               const Expression& expression,
                                               const Statement& body) {
  return *new (zone_) ForInStatement(range, const_cast<Token*>(&keyword),
                                     const_cast<Expression*>(&expression),
                                     const_cast<Statement*>(&body));
}

ForOfStatement& NodeFactory::NewForOfStatement(const SourceCodeRange& range,
                                               const Token& keyword,
                                               const Expression& binding,
                                               const Expression& expression,
                                               const Statement& body) {
  return *new (zone_) ForOfStatement(
      range, const_cast<Token*>(&keyword), const_cast<Expression*>(&binding),
      const_cast<Expression*>(&expression), const_cast<Statement*>(&body));
}

IfElseStatement& NodeFactory::NewIfElseStatement(const SourceCodeRange& range,
                                                 const Expression& expression,
                                                 const Statement& then_clause,
                                                 const Statement& else_clause) {
  return *new (zone_)
      IfElseStatement(range, const_cast<Expression*>(&expression),
                      const_cast<Statement*>(&then_clause),
                      const_cast<Statement*>(&else_clause));
}

IfStatement& NodeFactory::NewIfStatement(const SourceCodeRange& range,
                                         const Expression& expression,
                                         const Statement& then_clause) {
  return *new (zone_) IfStatement(range, const_cast<Expression*>(&expression),
                                  const_cast<Statement*>(&then_clause));
}

InvalidStatement& NodeFactory::NewInvalidStatement(const Node& node,
                                                   int error_code) {
  return *new (zone_) InvalidStatement(node.range(), error_code);
}

LabeledStatement& NodeFactory::NewLabeledStatement(const SourceCodeRange& range,
                                                   const Name& label,
                                                   const Statement& statement) {
  return *new (zone_) LabeledStatement(range, const_cast<Name*>(&label),
                                       const_cast<Statement*>(&statement));
}

LetStatement& NodeFactory::NewLetStatement(const SourceCodeRange& range,
                                           const Expression& expression) {
  return *new (zone_) LetStatement(range, const_cast<Expression*>(&expression));
}

ReturnStatement& NodeFactory::NewReturnStatement(const SourceCodeRange& range,
                                                 const Expression& expression) {
  return *new (zone_)
      ReturnStatement(range, const_cast<Expression*>(&expression));
}

SwitchStatement& NodeFactory::NewSwitchStatement(
    const SourceCodeRange& range,
    const Expression& expression,
    const std::vector<Statement*>& clauses) {
  auto* const list = new (zone_) StatementList(zone_, clauses);
  return *new (zone_)
      SwitchStatement(range, const_cast<Expression*>(&expression), list);
}

ThrowStatement& NodeFactory::NewThrowStatement(const SourceCodeRange& range,
                                               const Expression& expression) {
  return *new (zone_)
      ThrowStatement(range, const_cast<Expression*>(&expression));
}

TryCatchFinallyStatement& NodeFactory::NewTryCatchFinallyStatement(
    const SourceCodeRange& range,
    const Statement& try_block,
    const Expression& catch_parameter,
    const Statement& catch_block,
    const Statement& finally_block) {
  return *new (zone_)
      TryCatchFinallyStatement(range, const_cast<Statement*>(&try_block),
                               const_cast<Expression*>(&catch_parameter),
                               const_cast<Statement*>(&catch_block),
                               const_cast<Statement*>(&finally_block));
}

TryCatchStatement& NodeFactory::NewTryCatchStatement(
    const SourceCodeRange& range,
    const Statement& try_block,
    const Expression& catch_parameter,
    const Statement& catch_block) {
  return *new (zone_)
      TryCatchStatement(range, const_cast<Statement*>(&try_block),
                        const_cast<Expression*>(&catch_parameter),
                        const_cast<Statement*>(&catch_block));
}

TryFinallyStatement& NodeFactory::NewTryFinallyStatement(
    const SourceCodeRange& range,
    const Statement& try_block,
    const Statement& finally_block) {
  return *new (zone_)
      TryFinallyStatement(range, const_cast<Statement*>(&try_block),
                          const_cast<Statement*>(&finally_block));
}

VarStatement& NodeFactory::NewVarStatement(const SourceCodeRange& range,
                                           const Expression& expression) {
  return *new (zone_) VarStatement(range, const_cast<Expression*>(&expression));
}

WhileStatement& NodeFactory::NewWhileStatement(const SourceCodeRange& range,
                                               const Expression& expression,
                                               const Statement& statement) {
  return *new (zone_)
      WhileStatement(range, const_cast<Expression*>(&expression),
                     const_cast<Statement*>(&statement));
}

WithStatement& NodeFactory::NewWithStatement(const SourceCodeRange& range,
                                             const Expression& expression,
                                             const Statement& statement) {
  return *new (zone_) WithStatement(range, const_cast<Expression*>(&expression),
                                    const_cast<Statement*>(&statement));
}

}  // namespace ast
}  // namespace joana