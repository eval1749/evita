// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"
#include "evita/visuals/style/display.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box
//
Box::Box() {}
Box::~Box() {}

bool Box::IsDescendantOf(const Box& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Box& box) {
  return ostream << box.class_name() << box.bounds();
}

std::ostream& operator<<(std::ostream& ostream, const Box* box) {
  if (!box)
    return ostream << "nullptr";
  return ostream << *box;
}

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
