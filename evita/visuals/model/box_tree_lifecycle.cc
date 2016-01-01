// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/model/box_tree_lifecycle.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTreeLifecycle::Scope
//
BoxTreeLifecycle::Scope::Scope(BoxTreeLifecycle* lifecycle,
                               State from_state,
                               State to_state)
    : lifecycle_(lifecycle), to_state_(to_state) {
  lifecycle_->AdvanceTo(from_state);
}

BoxTreeLifecycle::Scope::~Scope() {
  lifecycle_->AdvanceTo(to_state_);
}

//////////////////////////////////////////////////////////////////////
//
// BoxTreeLifecycle
//
BoxTreeLifecycle::BoxTreeLifecycle() : state_(State::VisualUpdatePending) {}
BoxTreeLifecycle::~BoxTreeLifecycle() {}

void BoxTreeLifecycle::AdvanceTo(State new_state) {
  DCHECK(static_cast<int>(new_state) == static_cast<int>(state_) + 1)
      << "Can't advance to " << new_state << " from " << state_;
  state_ = new_state;
}

bool BoxTreeLifecycle::AllowsTreeMutaions() const {
  return state_ == State::VisualUpdatePending || state_ == State::PaintClean;
}

bool BoxTreeLifecycle::IsAtLeast(State state) const {
  return static_cast<int>(state_) >= static_cast<int>(state);
}

void BoxTreeLifecycle::Reset() {
  state_ = State::VisualUpdatePending;
}

std::ostream& operator<<(std::ostream& ostream,
                         const BoxTreeLifecycle& lifecycle) {
  return ostream << lifecycle.state();
}

std::ostream& operator<<(std::ostream& ostream, BoxTreeLifecycle::State state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

}  // namespace visuals
