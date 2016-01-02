// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_ANCESTORS_H_
#define EVITA_VISUALS_DOM_ANCESTORS_H_

#include "evita/visuals/dom/node.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Node::Ancestors
//
class Node::Ancestors final {
 public:
  class Iterator final {
   public:
    explicit Iterator(ContainerNode* node);
    Iterator(const Iterator& other);
    ~Iterator();

    ContainerNode* operator*() const;
    ContainerNode* operator->() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    ContainerNode* node_;
  };

  explicit Ancestors(const Node& node);
  ~Ancestors();

  Iterator begin() const;
  Iterator end() const;

 private:
  const ContainerNode* node_;

  DISALLOW_COPY_AND_ASSIGN(Ancestors);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_ANCESTORS_H_
