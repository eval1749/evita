// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_MODEL_ANCESTORS_OR_SELF_H_
#define EVITA_VISUALS_MODEL_ANCESTORS_OR_SELF_H_

#include "evita/visuals/model/box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::AncestorsOrSelfOf
//
class Box::AncestorsOrSelfOf final {
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

  explicit AncestorsOrSelfOf(const Box& box);
  ~AncestorsOrSelfOf();

  Iterator begin() const;
  Iterator end() const;

 private:
  const Box* box_;

  DISALLOW_COPY_AND_ASSIGN(AncestorsOrSelfOf);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_MODEL_ANCESTORS_OR_SELF_H_
