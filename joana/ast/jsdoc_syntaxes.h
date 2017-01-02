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
// JsDocDocumentSyntax
//
class JOANA_AST_EXPORT JsDocDocumentSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocDocument, Syntax);

 public:
  ~JsDocDocumentSyntax() final;

 private:
  JsDocDocumentSyntax();

  DISALLOW_COPY_AND_ASSIGN(JsDocDocumentSyntax);
};

//
// JsDocTagSyntax
//
class JOANA_AST_EXPORT JsDocTagSyntax final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocTag, Syntax);

 public:
  ~JsDocTagSyntax() final;

  static const Node& NameOf(const Node& node);
  static ChildNodes OperandsOf(const Node& node);

 private:
  JsDocTagSyntax();

  DISALLOW_COPY_AND_ASSIGN(JsDocTagSyntax);
};

//
// JsDocTextSyntax
//
class JOANA_AST_EXPORT JsDocTextSyntax final : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(JsDocText, Syntax);

 public:
  ~JsDocTextSyntax() final;

 private:
  JsDocTextSyntax();

  DISALLOW_COPY_AND_ASSIGN(JsDocTextSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_JSDOC_SYNTAXES_H_
