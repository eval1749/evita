// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_FACTORY_H_
#define JOANA_PUBLIC_AST_NODE_FACTORY_H_

#include <memory>

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

  Name& NewName(const SourceCodeRange& range);

  Punctuator& NewPunctuator(const SourceCodeRange& range, PunctuatorKind kind);

  Template& NewTemplate(const SourceCodeRange& range);

  // Expressions factory members
  InvalidExpression& NewInvalidExpression(const Node& node, int error_code);
  LiteralExpression& NewLiteralExpression(const Literal& literal);

  // Literals factory members
  BooleanLiteral& NewBooleanLiteral(const Name& name, bool value);

  NullLiteral& NewNullLiteral(const Name& name);

  NumericLiteral& NewNumericLiteral(const SourceCodeRange& range, double value);

  StringLiteral& NewStringLiteral(const SourceCodeRange& range,
                                  base::StringPiece16 data);

  UndefinedLiteral& NewUndefinedLiteral(const Name& name);

  // Statements factory members
  BlockStatement& NewBlockStatement(const Punctuator& left_brace);
  EmptyStatement& NewEmptyStatement(const Punctuator& semi_colon);

 private:
  class NameIdMap;

  std::unique_ptr<NameIdMap> name_id_map_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_PUBLIC_AST_NODE_FACTORY_H_
