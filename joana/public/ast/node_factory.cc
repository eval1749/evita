// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/public/ast/node_factory.h"

#include "joana/public/ast/comment.h"
#include "joana/public/ast/expressions/literal_expression.h"
#include "joana/public/ast/invalid.h"
#include "joana/public/ast/literals/boolean_literal.h"
#include "joana/public/ast/literals/null_literal.h"
#include "joana/public/ast/literals/numeric_literal.h"
#include "joana/public/ast/literals/string_literal.h"
#include "joana/public/ast/literals/undefined_literal.h"
#include "joana/public/ast/module.h"
#include "joana/public/ast/name.h"
#include "joana/public/ast/punctuator.h"
#include "joana/public/ast/template.h"

namespace joana {
namespace ast {

NodeFactory::NodeFactory(Zone* zone) : zone_(zone) {}
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
  return *new (zone_) Name(range);
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

}  // namespace ast
}  // namespace joana
