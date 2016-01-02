// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_DESCENDANTS_OR_SELF_H_
#define EVITA_VISUALS_DOM_DESCENDANTS_OR_SELF_H_

#include "evita/visuals/dom/node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::DescendantsOrSelf
//
class Node::DescendantsOrSelf final {
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

  explicit DescendantsOrSelf(const Node& node);
  ~DescendantsOrSelf();

  Iterator begin() const;
  Iterator end() const;

 private:
  const Node* node_;

  DISALLOW_COPY_AND_ASSIGN(DescendantsOrSelf);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_DESCENDANTS_OR_SELF_H_
