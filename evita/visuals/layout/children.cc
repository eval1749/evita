// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/children.h"

#include "base/logging.h"
#include "evita/visuals/layout/container_box.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Box::Children
//
Box::Children::Children(const ContainerBox& container)
    : container_(&container) {}

Box::Children::Children(const Box::Children& other)
    : container_(other.container_) {}

Box::Children::~Children() {}

Box::Children& Box::Children::operator=(const Box::Children& other) {
  container_ = other.container_;
  return *this;
}

Box::Children::Iterator Box::Children::begin() const {
  return Iterator(container_->first_child());
}

Box::Children::Iterator Box::Children::end() const {
  return Iterator(nullptr);
}

//////////////////////////////////////////////////////////////////////
//
// Box::Children::Iterator
//
Box::Children::Iterator::Iterator(Box* box) : box_(box) {}

Box::Children::Iterator::Iterator(const Iterator& other) : box_(other.box_) {}

Box::Children::Iterator::~Iterator() {}

Box* Box::Children::Iterator::operator*() const {
  DCHECK(box_);
  return box_;
}

Box* Box::Children::Iterator::operator->() const {
  DCHECK(box_);
  return box_;
}

Box::Children::Iterator& Box::Children::Iterator::operator++() {
  DCHECK(box_);
  box_ = box_->next_sibling();
  return *this;
}

bool Box::Children::Iterator::operator==(const Iterator& other) const {
  if (box_ && other.box_)
    DCHECK_EQ(box_->parent(), other.box_->parent());
  return box_ == other.box_;
}

bool Box::Children::Iterator::operator!=(const Iterator& other) const {
  return !operator==(other);
}

}  // namespace visuals
