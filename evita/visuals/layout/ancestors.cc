// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/ancestors.h"

#include "base/logging.h"
#include "evita/visuals/layout/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::Ancestors
//
Box::Ancestors::Ancestors(const Box& box) : box_(box.parent()) {}
Box::Ancestors::~Ancestors() {}

Box::Ancestors::Iterator Box::Ancestors::begin() const {
  return Iterator(const_cast<ContainerBox*>(box_));
}

Box::Ancestors::Iterator Box::Ancestors::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Box::Ancestors::Iterator
//
Box::Ancestors::Iterator::Iterator(ContainerBox* box) : box_(box) {}

Box::Ancestors::Iterator::Iterator(const Iterator& other) : box_(other.box_) {}

Box::Ancestors::Iterator::~Iterator() {}

ContainerBox* Box::Ancestors::Iterator::operator*() const {
  DCHECK(box_);
  return box_;
}

ContainerBox* Box::Ancestors::Iterator::operator->() const {
  DCHECK(box_);
  return box_;
}

Box::Ancestors::Iterator& Box::Ancestors::Iterator::operator++() {
  DCHECK(box_);
  box_ = box_->parent();
  return *this;
}

bool Box::Ancestors::Iterator::operator==(const Iterator& other) const {
  return box_ == other.box_;
}

bool Box::Ancestors::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals
