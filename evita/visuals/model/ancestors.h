// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_ANCESTORS_H_
#define EVITA_VISUALS_MODEL_ANCESTORS_H_

#include "evita/visuals/model/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::Ancestors
//
class Box::Ancestors final {
 public:
  class Iterator final {
   public:
    explicit Iterator(ContainerBox* box);
    Iterator(const Iterator& other);
    ~Iterator();

    ContainerBox* operator*() const;
    ContainerBox* operator->() const;
    Iterator& operator++();

    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

   private:
    ContainerBox* box_;
  };

  explicit Ancestors(const Box& box);
  ~Ancestors();

  Iterator begin() const;
  Iterator end() const;

 private:
  const ContainerBox* box_;

  DISALLOW_COPY_AND_ASSIGN(Ancestors);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_ANCESTORS_H_
