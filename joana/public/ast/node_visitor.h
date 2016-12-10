// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_PUBLIC_AST_NODE_VISITOR_H_
#define JOANA_PUBLIC_AST_NODE_VISITOR_H_

#include "base/macros.h"
#include "joana/public/ast/node_forward.h"
#include "joana/public/public_export.h"

namespace joana {
namespace ast {

class JOANA_PUBLIC_EXPORT NodeVisitor {
 public:
  void Visit(const Node& node);
  void Visit(Node* node);

#define V(name) virtual void Visit##name(name* node) = 0;
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

#endif  // JOANA_PUBLIC_AST_NODE_VISITOR_H_
