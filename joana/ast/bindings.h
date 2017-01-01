// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_BINDINGS_H_
#define JOANA_AST_BINDINGS_H_

#include "joana/ast/syntax.h"

namespace joana {
namespace ast {

class ChildNodes;
class Node;

DECLARE_AST_SYNTAX_0(BindingCommaElement)
DECLARE_AST_SYNTAX_0(BindingInvalidElement)
DECLARE_AST_SYNTAX_0(BindingNameElement)
DECLARE_AST_SYNTAX_0(BindingProperty)
DECLARE_AST_SYNTAX_0(BindingRestElement)

//
// ArrayBindingPatternSyntax
//
class JOANA_AST_EXPORT ArrayBindingPatternSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ArrayBindingPatternSyntax, Syntax);

 public:
  ~ArrayBindingPatternSyntax() final;

  static ChildNodes ElementsOf(const Node& node);
  static const Node& InitializerOf(const Node& node);

 private:
  ArrayBindingPatternSyntax();

  DISALLOW_COPY_AND_ASSIGN(ArrayBindingPatternSyntax);
};

//
// ObjectBindingPatternSyntax
//
class JOANA_AST_EXPORT ObjectBindingPatternSyntax final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ObjectBindingPatternSyntax, Syntax);

 public:
  ~ObjectBindingPatternSyntax() final;

  static ChildNodes ElementsOf(const Node& node);
  static const Node& InitializerOf(const Node& node);

 private:
  ObjectBindingPatternSyntax();

  DISALLOW_COPY_AND_ASSIGN(ObjectBindingPatternSyntax);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_BINDINGS_H_
