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
// ArrayBindingPattern
//
class JOANA_AST_EXPORT ArrayBindingPattern final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ArrayBindingPattern, Syntax);

 public:
  ~ArrayBindingPattern() final;

  static ChildNodes ElementsOf(const Node& node);
  static const Node& InitializerOf(const Node& node);

 private:
  ArrayBindingPattern();

  DISALLOW_COPY_AND_ASSIGN(ArrayBindingPattern);
};

//
// ObjectBindingPattern
//
class JOANA_AST_EXPORT ObjectBindingPattern final
    : public SyntaxTemplate<Syntax> {
  DECLARE_CONCRETE_AST_SYNTAX(ObjectBindingPattern, Syntax);

 public:
  ~ObjectBindingPattern() final;

  static ChildNodes ElementsOf(const Node& node);
  static const Node& InitializerOf(const Node& node);

 private:
  ObjectBindingPattern();

  DISALLOW_COPY_AND_ASSIGN(ObjectBindingPattern);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_BINDINGS_H_
