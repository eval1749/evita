// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_EMPTY_NODE_VISITOR_H_
#define JOANA_AST_EMPTY_NODE_VISITOR_H_

#include "joana/ast/node_visitor.h"

#include "joana/ast/node_forward.h"

namespace joana {
namespace ast {

//
// EmptyNodeVisitor
//
class JOANA_AST_EXPORT EmptyNodeVisitor : public NodeVisitor {
 public:
  ~EmptyNodeVisitor();

 protected:
  EmptyNodeVisitor();

 private:
#define V(name) void Visit##name(const ast::name& node) override;
  FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

  DISALLOW_COPY_AND_ASSIGN(EmptyNodeVisitor);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_EMPTY_NODE_VISITOR_H_
