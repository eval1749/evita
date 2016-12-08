// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>

#include "joana/public/ast/node_factory.h"

#include "joana/public/ast/comment.h"
#include "joana/public/ast/expressions.h"
#include "joana/public/ast/invalid.h"
#include "joana/public/ast/literals.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/name.h"
#include "joana/public/ast/punctuator.h"
#include "joana/public/ast/statements.h"
#include "joana/public/ast/template.h"

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
  last_id_ = static_cast<int>(NameId::StartKeyword);
#define V(name, camel, upper) Register(base::StringPiece16(L## #name));
  FOR_EACH_JAVASCRIPT_KEYWORD(V)
#undef V

  last_id_ = static_cast<int>(NameId::StartKnown);
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

Template& NodeFactory::NewTemplate(const SourceCodeRange& range) {
  return *new (zone_) Template(range);
}

// Expressions
LiteralExpression& NodeFactory::NewLiteralExpression(const Literal& literal) {
  return *new (zone_) LiteralExpression(literal);
}

// Literals
BooleanLiteral& NodeFactory::NewCommen(const SourceCodeRange& range,
                                       bool value) {
  return *new (zone_) BooleanLiteral(range, value);
}

NullLiteral& NodeFactory::NewNullLiteral(const SourceCodeRange& range) {
  return *new (zone_) NullLiteral(range);
}

NumericLiteral& NodeFactory::NewNumericLiteral(const SourceCodeRange& range,
                                               double value) {
  return *new (zone_) NumericLiteral(range, value);
}

StringLiteral& NodeFactory::NewStringLiteral(const SourceCodeRange& range,
                                             base::StringPiece16 data) {
  return *new (zone_) StringLiteral(range, data);
}

UndefinedLiteral& NodeFactory::NewUndefinedLiteral(
    const SourceCodeRange& range) {
  return *new (zone_) UndefinedLiteral(range);
}

// Statements factory members
EmptyStatement& NodeFactory::NewEmptyStatement(const Punctuator& semi_colon) {
  return *new (zone_) EmptyStatement(semi_colon);
}

}  // namespace ast
}  // namespace joana
