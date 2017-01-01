// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOANA_AST_NODE_TRAVERSAL_H_
#define JOANA_AST_NODE_TRAVERSAL_H_

#include <iterator>

#include "joana/ast/ast_export.h"
#include "joana/ast/syntax_forward.h"

namespace joana {
namespace ast {

class Node;
class NodeTraversal;

//
// ChildNodes
//
class JOANA_AST_EXPORT ChildNodes final {
 public:
  class JOANA_AST_EXPORT Iterator final
      : public std::iterator<std::input_iterator_tag, const Node> {
   public:
    Iterator(const Iterator& other);
    ~Iterator();

    reference operator*() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    friend class ChildNodes;

    Iterator(const ChildNodes& owner, size_t index);

    size_t index_;
    const ChildNodes* owner_;
  };

  ChildNodes(const ChildNodes& other);
  ~ChildNodes();

  Iterator begin() const;
  bool empty() const;
  Iterator end() const;
  size_t size() const;

 private:
  friend class NodeTraversal;

  ChildNodes(const Node& container, size_t start);

  const Node* container_;
  size_t start_;
};

//
// NodeTraversal
//
class JOANA_AST_EXPORT NodeTraversal final {
 public:
  NodeTraversal() = delete;
  ~NodeTraversal() = delete;

  // Returns child node list generator starting from |index|th child.
  static ChildNodes ChildNodesFrom(const Node& container, size_t index);

  // Returns child node list generator starting from first child.
  static ChildNodes ChildNodesOf(const Node& container);
};

}  // namespace ast
}  // namespace joana

#endif  // JOANA_AST_NODE_TRAVERSAL_H_
