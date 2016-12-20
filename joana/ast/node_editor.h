// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_EDITOR_H_
#define JOANA_AST_NODE_EDITOR_H_

#include "base/macros.h"
#include "joana/ast/ast_export.h"

namespace joana {
namespace ast {

class ContainerNode;
class Node;

class JOANA_AST_EXPORT NodeEditor final {
 public:
  NodeEditor();
  ~NodeEditor();

  NodeEditor& AppendChild(ContainerNode* container, Node* new_child);
  NodeEditor& InsertBefore(ContainerNode* container,
                           Node* new_child,
                           Node* ref_child);
  NodeEditor& RemoveChild(ContainerNode* container, Node* old_child);
  NodeEditor& RemoveNode(Node* old_child);
  NodeEditor& ReplaceChild(ContainerNode* container,
                           Node* new_child,
                           Node* old_child);

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeEditor);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_EDITOR_H_
