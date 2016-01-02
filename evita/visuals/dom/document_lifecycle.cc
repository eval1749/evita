// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/dom/document_lifecycle.h"

#include "base/logging.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DocumentLifecycle::Scope
//
DocumentLifecycle::Scope::Scope(DocumentLifecycle* lifecycle,
                                State from_state,
                                State to_state)
    : lifecycle_(lifecycle), to_state_(to_state) {
  lifecycle_->AdvanceTo(from_state);
}

DocumentLifecycle::Scope::~Scope() {
  lifecycle_->AdvanceTo(to_state_);
}

//////////////////////////////////////////////////////////////////////
//
// DocumentLifecycle
//
DocumentLifecycle::DocumentLifecycle() : state_(State::VisualUpdatePending) {}
DocumentLifecycle::~DocumentLifecycle() {}

void DocumentLifecycle::AdvanceTo(State new_state) {
  DCHECK(static_cast<int>(new_state) == static_cast<int>(state_) + 1)
      << "Can't advance to " << new_state << " from " << state_;
  state_ = new_state;
}

bool DocumentLifecycle::AllowsTreeMutaions() const {
  return state_ == State::VisualUpdatePending || state_ == State::PaintClean;
}

bool DocumentLifecycle::IsAtLeast(State state) const {
  return static_cast<int>(state_) >= static_cast<int>(state);
}

void DocumentLifecycle::Reset() {
  state_ = State::VisualUpdatePending;
}

std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle& lifecycle) {
  return ostream << lifecycle.state();
}

std::ostream& operator<<(std::ostream& ostream,
                         DocumentLifecycle::State state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  return ostream << (it < std::end(texts) ? *it : "???");
}

}  // namespace visuals
