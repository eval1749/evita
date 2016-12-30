// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_VISITOR_H_
#define JOANA_AST_NODE_VISITOR_H_

#include "base/macros.h"
#include "joana/ast/ast_export.h"
#include "joana/ast/node_forward.h"

namespace joana {
namespace ast {

//
// NoveVisitor
//
class JOANA_AST_EXPORT NodeVisitor {
 public:
  void Visit(const Node& node);

#define V(name) virtual void Visit##name(const name& node) = 0;
  FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

 protected:
  NodeVisitor();
  virtual ~NodeVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeVisitor);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_VISITOR_H_
