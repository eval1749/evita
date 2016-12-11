// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/public/ast/node_factory.h"

#include "joana/public/ast/expressions.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/tokens.h"

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

Invalid& NodeFactory::NewInvalid(const SourceCodeRange& range, int error_code) {
  return *new (zone_) Invalid(range, error_code);
}

Module& NodeFactory::NewModule(const SourceCodeRange& range) {
  return *new (zone_) Module(range);
}

Name& NodeFactory::NewName(const SourceCodeRange& range) {
  return *new (zone_) Name(range, name_id_map_->Register(range.GetString()));
}

Punctuator& NodeFactory::NewPunctuator(const SourceCodeRange& range,
                                       PunctuatorKind kind) {
  return *new (zone_) Punctuator(range, kind);
}

// Expressions
InvalidExpression& NodeFactory::NewInvalidExpression(const Node& node,
                                                     int error_code) {
  return *new (zone_) InvalidExpression(node, error_code);
}

LiteralExpression& NodeFactory::NewLiteralExpression(const Literal& literal) {
  return *new (zone_) LiteralExpression(literal);
}

ReferenceExpression& NodeFactory::NewReferenceExpression(const Name& name) {
  return *new (zone_) ReferenceExpression(name);
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

// Statements factory members
BlockStatement& NodeFactory::NewBlockStatement(const Punctuator& left_brace) {
  return *new (zone_) BlockStatement(left_brace);
}

BreakStatement& NodeFactory::NewBreakStatement(const Name& keyword,
                                               const Name& label) {
  return *new (zone_) BreakStatement(keyword, label);
}

BreakStatement& NodeFactory::NewBreakStatement(const Name& keyword) {
  return *new (zone_) BreakStatement(keyword);
}

ContinueStatement& NodeFactory::NewContinueStatement(const Name& keyword,
                                                     const Name& label) {
  return *new (zone_) ContinueStatement(keyword, label);
}

ContinueStatement& NodeFactory::NewContinueStatement(const Name& keyword) {
  return *new (zone_) ContinueStatement(keyword);
}

DoStatement& NodeFactory::NewDoStatement(const Name& keyword,
                                         const Statement& statement,
                                         const Expression& condition) {
  return *new (zone_) DoStatement(keyword, statement, condition);
}

EmptyStatement& NodeFactory::NewEmptyStatement(const Punctuator& semi_colon) {
  return *new (zone_) EmptyStatement(semi_colon);
}

ExpressionStatement& NodeFactory::NewExpressionStatement(
    const Expression& expression) {
  return *new (zone_) ExpressionStatement(expression);
}

IfStatement& NodeFactory::NewIfStatement(const Name& keyword,
                                         const Expression& condition,
                                         const Statement& then_clause,
                                         const Statement& else_clause) {
  return *new (zone_) IfStatement(keyword, condition, then_clause, else_clause);
}

IfStatement& NodeFactory::NewIfStatement(const Name& keyword,
                                         const Expression& condition,
                                         const Statement& then_clause) {
  return *new (zone_) IfStatement(keyword, condition, then_clause);
}

InvalidStatement& NodeFactory::NewInvalidStatement(const Node& node,
                                                   int error_code) {
  return *new (zone_) InvalidStatement(node, error_code);
}

ThrowStatement& NodeFactory::NewThrowStatement(const Name& keyword,
                                               const Expression& expression) {
  return *new (zone_) ThrowStatement(keyword, expression);
}

TryStatement& NodeFactory::NewTryStatement(
    const Name& keyword,
    const BlockStatement& block,
    const Name& catch_name,
    const BlockStatement& catch_block,
    const BlockStatement& finally_block) {
  return *new (zone_)
      TryStatement(keyword, block, catch_name, catch_block, finally_block);
}

TryStatement& NodeFactory::NewTryStatement(const Name& keyword,
                                           const BlockStatement& block,
                                           const Name& catch_name,
                                           const BlockStatement& catch_block) {
  return *new (zone_) TryStatement(keyword, block, catch_name, catch_block);
}

WhileStatement& NodeFactory::NewWhileStatement(const Name& keyword,
                                               const Expression& condition,
                                               const Statement& statement) {
  return *new (zone_) WhileStatement(keyword, condition, statement);
}

}  // namespace ast
}  // namespace joana
