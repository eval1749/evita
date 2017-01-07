// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_SYNTAX_VISITOR_H_
#define JOANA_AST_SYNTAX_VISITOR_H_

#include "base/macros.h"
#include "joana/ast/ast_export.h"
#include "joana/ast/syntax_forward.h"

namespace joana {
namespace ast {

class Node;

//
// SyntaxVisitor
//
class JOANA_AST_EXPORT SyntaxVisitor {
 public:
  ~SyntaxVisitor();

  void Visit(const ast::Node& node);
  void VisitChildNodes(const ast::Node& node);
  virtual void VisitDefault(const ast::Node& node);

#define V(name) \
  virtual void VisitInternal(const name& syntax, const ast::Node& node);
  FOR_EACH_AST_SYNTAX(V)
#undef V

 protected:
  SyntaxVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(SyntaxVisitor);
};

void JOANA_AST_EXPORT DepthFirstTraverse(SyntaxVisitor* visitor,
                                         const Node& start_node);

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_SYNTAX_VISITOR_H_
