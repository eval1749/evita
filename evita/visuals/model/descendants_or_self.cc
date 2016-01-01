// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/descendants_or_self.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"
#include "evita/visuals/model/box_traversal.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::DescendantsOrSelf
//
Box::DescendantsOrSelf::DescendantsOrSelf(const Box& box) : box_(&box) {}
Box::DescendantsOrSelf::~DescendantsOrSelf() {}

Box::DescendantsOrSelf::Iterator Box::DescendantsOrSelf::begin() const {
  return Iterator(const_cast<Box*>(box_));
}

Box::DescendantsOrSelf::Iterator Box::DescendantsOrSelf::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Box::DescendantsOrSelf::Iterator
//
Box::DescendantsOrSelf::Iterator::Iterator(Box* box) : box_(box) {}

Box::DescendantsOrSelf::Iterator::Iterator(const Iterator& other)
    : box_(other.box_) {}

Box::DescendantsOrSelf::Iterator::~Iterator() {}

Box* Box::DescendantsOrSelf::Iterator::operator*() const {
  DCHECK(box_);
  return box_;
}

Box* Box::DescendantsOrSelf::Iterator::operator->() const {
  DCHECK(box_);
  return box_;
}

Box::DescendantsOrSelf::Iterator& Box::DescendantsOrSelf::Iterator::
operator++() {
  DCHECK(box_);
  box_ = BoxTraversal::NextOf(*box_);
  return *this;
}

bool Box::DescendantsOrSelf::Iterator::operator==(const Iterator& other) const {
  return box_ == other.box_;
}

bool Box::DescendantsOrSelf::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals
