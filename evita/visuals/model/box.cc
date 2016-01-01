// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box.h"

#include "base/logging.h"
#include "evita/visuals/model/container_box.h"

namespace visuals {

namespace {
int last_box_sequence_id;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Box
//
Box::Box() : sequence_id_(++last_box_sequence_id) {}
Box::~Box() {}

FloatRect Box::content_bounds() const {
  return FloatRect(FloatPoint() + border_.top_left() + padding_.top_left(),
                   bounds_.size() - border_.top_left() - padding_.top_left() -
                       border_.bottom_right() - padding_.bottom_right());
}

bool Box::IsDescendantOf(const Box& other) const {
  for (auto runner = parent(); runner; runner = runner->parent()) {
    if (runner == &other)
      return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& ostream, const Box& box) {
  return ostream << box.class_name() << '.' << box.sequence_id() << ' '
                 << box.bounds();
}

std::ostream& operator<<(std::ostream& ostream, const Box* box) {
  if (!box)
    return ostream << "nullptr";
  return ostream << *box;
}

}  // namespace visuals
