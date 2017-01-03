// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_JSDOC_SYNTAXES_H_
#define JOANA_AST_JSDOC_SYNTAXES_H_

#include <vector>

#include "joana/ast/syntax.h"
#include "joana/ast/syntax_forward.h"

namespace joana {
namespace ast {

class ChildNodes;
class Node;

//
// JsDocDocument
//
class JOANA_AST_EXPORT JsDocDocument final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocDocument, Syntax);

 public:
  ~JsDocDocument() final;

 private:
  JsDocDocument();

  DISALLOW_COPY_AND_ASSIGN(JsDocDocument);
};

//
// JsDocTag
//
class JOANA_AST_EXPORT JsDocTag final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocTag, Syntax);

 public:
  ~JsDocTag() final;

  static const Node& NameOf(const Node& node);
  static ChildNodes OperandsOf(const Node& node);

 private:
  JsDocTag();

  DISALLOW_COPY_AND_ASSIGN(JsDocTag);
};

//
// JsDocText
//
class JOANA_AST_EXPORT JsDocText final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocText, Syntax);

 public:
  ~JsDocText() final;

 private:
  JsDocText();

  DISALLOW_COPY_AND_ASSIGN(JsDocText);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_JSDOC_SYNTAXES_H_
