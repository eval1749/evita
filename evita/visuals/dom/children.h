// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_CHILDREN_H_
#define EVITA_VISUALS_DOM_CHILDREN_H_

#include "evita/visuals/dom/node.h"

namespace visuals {

class Node;

//////////////////////////////////////////////////////////////////////
//
// Node::Children
//
class Node::Children final {
 public:
  class Iterator final {
   public:
    explicit Iterator(Node* node);
    Iterator(const Iterator& other);
    ~Iterator();

    Node* operator*() const;
    Node* operator->() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    Node* node_;
  };

  explicit Children(const ContainerNode& container);
  Children(const Children& other);
  ~Children();

  Children& operator=(const Node::Children& other);

  Iterator begin() const;
  Iterator end() const;

 private:
  const ContainerNode* container_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_CHILDREN_H_
