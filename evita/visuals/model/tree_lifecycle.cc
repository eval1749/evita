// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/model/tree_lifecycle.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// TreeLifecycle::Scope
//
TreeLifecycle::Scope::Scope(TreeLifecycle* lifecycle,
                            State from_state,
                            State to_state)
    : lifecycle_(lifecycle), to_state_(to_state) {
  lifecycle_->AdvanceTo(from_state);
}

TreeLifecycle::Scope::~Scope() {
  lifecycle_->AdvanceTo(to_state_);
}

//////////////////////////////////////////////////////////////////////
//
// TreeLifecycle
//
TreeLifecycle::TreeLifecycle() : state_(State::VisualUpdatePending) {}
TreeLifecycle::~TreeLifecycle() {}

void TreeLifecycle::AdvanceTo(State new_state) {
  DCHECK(static_cast<int>(new_state) == static_cast<int>(state_) + 1)
      << "Can't advance to " << new_state << " from " << state_;
  state_ = new_state;
}

bool TreeLifecycle::IsAtLeast(State state) const {
  return static_cast<int>(state_) >= static_cast<int>(state);
}

void TreeLifecycle::Reset() {
  state_ = State::VisualUpdatePending;
}

std::ostream& operator<<(std::ostream& ostream,
                         const TreeLifecycle& lifecycle) {
  return ostream << lifecycle.state();
}

std::ostream& operator<<(std::ostream& ostream, TreeLifecycle::State state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

}  // namespace visuals
