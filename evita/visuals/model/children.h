// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_CHILDREN_H_
#define EVITA_VISUALS_MODEL_CHILDREN_H_

#include "evita/visuals/model/box.h"

namespace visuals {

class Box;

//////////////////////////////////////////////////////////////////////
//
// Box::Children
//
class Box::Children final {
 public:
  class Iterator final {
   public:
    explicit Iterator(Box* box);
    Iterator(const Iterator& other);
    ~Iterator();

    Box* operator*() const;
    Box* operator->() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    Box* box_;
  };

  explicit Children(const ContainerBox& container);
  Children(const Children& other);
  ~Children();

  Children& operator=(const Box::Children& other);

  Iterator begin() const;
  Iterator end() const;

 private:
  const ContainerBox* container_;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_CHILDREN_H_
