// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/ancestors_or_self.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::AncestorsOrSelfOf
//
Box::AncestorsOrSelfOf::AncestorsOrSelfOf(const Box& box) : box_(&box) {}
Box::AncestorsOrSelfOf::~AncestorsOrSelfOf() {}

Box::AncestorsOrSelfOf::Iterator Box::AncestorsOrSelfOf::begin() const {
  return Iterator(const_cast<Box*>(box_));
}

Box::AncestorsOrSelfOf::Iterator Box::AncestorsOrSelfOf::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Box::AncestorsOrSelfOf::Iterator
//
Box::AncestorsOrSelfOf::Iterator::Iterator(Box* box) : box_(box) {}

Box::AncestorsOrSelfOf::Iterator::Iterator(const Iterator& other)
    : box_(other.box_) {}

Box::AncestorsOrSelfOf::Iterator::~Iterator() {}

Box* Box::AncestorsOrSelfOf::Iterator::operator*() const {
  DCHECK(box_);
  return box_;
}

Box* Box::AncestorsOrSelfOf::Iterator::operator->() const {
  DCHECK(box_);
  return box_;
}

Box::AncestorsOrSelfOf::Iterator& Box::AncestorsOrSelfOf::Iterator::
operator++() {
  DCHECK(box_);
  box_ = box_->parent();
  return *this;
}

bool Box::AncestorsOrSelfOf::Iterator::operator==(const Iterator& other) const {
  return box_ == other.box_;
}

bool Box::AncestorsOrSelfOf::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals
