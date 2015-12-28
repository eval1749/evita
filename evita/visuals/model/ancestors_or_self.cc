// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/ancestors_or_self.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::AncestorsOrSelf
//
Box::AncestorsOrSelf::AncestorsOrSelf(const Box& box) : box_(&box) {}
Box::AncestorsOrSelf::~AncestorsOrSelf() {}

Box::AncestorsOrSelf::Iterator Box::AncestorsOrSelf::begin() const {
  return Iterator(const_cast<Box*>(box_));
}

Box::AncestorsOrSelf::Iterator Box::AncestorsOrSelf::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Box::AncestorsOrSelf::Iterator
//
Box::AncestorsOrSelf::Iterator::Iterator(Box* box) : box_(box) {}

Box::AncestorsOrSelf::Iterator::Iterator(const Iterator& other)
    : box_(other.box_) {}

Box::AncestorsOrSelf::Iterator::~Iterator() {}

Box* Box::AncestorsOrSelf::Iterator::operator*() const {
  DCHECK(box_);
  return box_;
}

Box* Box::AncestorsOrSelf::Iterator::operator->() const {
  DCHECK(box_);
  return box_;
}

Box::AncestorsOrSelf::Iterator& Box::AncestorsOrSelf::Iterator::operator++() {
  DCHECK(box_);
  box_ = box_->parent();
  return *this;
}

bool Box::AncestorsOrSelf::Iterator::operator==(const Iterator& other) const {
  return box_ == other.box_;
}

bool Box::AncestorsOrSelf::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals
