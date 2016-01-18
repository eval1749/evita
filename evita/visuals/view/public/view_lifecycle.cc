// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/view/public/view_lifecycle.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/view/public/view_lifecycle_observer.h"

namespace visuals {

static_assert((static_cast<int>(ViewLifecycle::State::VisualUpdatePending) &
               1) == 0,
              "VisualUpdatePending must be an even.");

static_assert((static_cast<int>(ViewLifecycle::State::LayoutClean) & 1) == 0,
              "LayoutClean must be an even.");

static_assert((static_cast<int>(ViewLifecycle::State::PaintClean) & 1) == 0,
              "PaintClean must be an even.");

static_assert((static_cast<int>(ViewLifecycle::State::StyleClean) & 1) == 0,
              "StyleClean must be an even.");

static_assert((static_cast<int>(ViewLifecycle::State::TreeClean) & 1) == 0,
              "TreeClean must be an even.");

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycle::Scope
//
ViewLifecycle::Scope::Scope(ViewLifecycle* lifecycle, State from_state)
    : lifecycle_(lifecycle), from_state_(from_state) {
  lifecycle_->Advance();
  DCHECK_EQ(lifecycle_->state_, from_state_);
}

ViewLifecycle::Scope::~Scope() {
  lifecycle_->Advance();
}

//////////////////////////////////////////////////////////////////////
//
// ViewLifecycle
//
ViewLifecycle::ViewLifecycle(const Document& document, const css::Media& media)
    : document_(document), media_(media), state_(State::Initialized) {}

ViewLifecycle::~ViewLifecycle() {
  DCHECK_EQ(State::Shutdown, state_);
}

bool ViewLifecycle::operator==(const ViewLifecycle& other) const {
  return this == &other;
}

bool ViewLifecycle::operator==(const ViewLifecycle* other) const {
  return this == other;
}

bool ViewLifecycle::operator!=(const ViewLifecycle& other) const {
  return !operator==(other);
}

bool ViewLifecycle::operator!=(const ViewLifecycle* other) const {
  return !operator==(other);
}

void ViewLifecycle::AddObserver(ViewLifecycleObserver* observer) const {
  observers_.AddObserver(observer);
}

void ViewLifecycle::Advance() {
  DCHECK_NE(State::PaintClean, state_);
  DCHECK_NE(State::InShutdown, state_);
  DCHECK_NE(State::Shutdown, state_);
  ChangeState(static_cast<State>(static_cast<int>(state_) + 1));
}

bool ViewLifecycle::AllowsSelectionChanges() const {
  return AllowsTreeMutaions();
}

bool ViewLifecycle::AllowsTreeMutaions() const {
  const auto value = static_cast<int>(state_);
  return (value & 1) == 0 && value <= static_cast<int>(State::PaintClean);
}

void ViewLifecycle::ChangeState(State new_state) {
  DCHECK_NE(state_, new_state);
  const auto old_state = state_;
  state_ = new_state;
  FOR_EACH_OBSERVER(ViewLifecycleObserver, observers_,
                    DidChangeLifecycleState(new_state, old_state));
}

void ViewLifecycle::FinishShutdown() {
  DCHECK_EQ(State::InShutdown, state_);
  ChangeState(State::Shutdown);
}

bool ViewLifecycle::IsAtLeast(State state) const {
  return static_cast<int>(state_) >= static_cast<int>(state);
}

bool ViewLifecycle::IsStyleClean() const {
  return IsAtLeast(State::StyleClean);
}

bool ViewLifecycle::IsTreeClean() const {
  return IsAtLeast(State::TreeClean);
}

bool ViewLifecycle::IsLayoutClean() const {
  return IsAtLeast(State::LayoutClean);
}

void ViewLifecycle::LimitTo(State limit_state) {
  if (static_cast<int>(state_) <= static_cast<int>(limit_state))
    return;
  ChangeState(limit_state);
}

void ViewLifecycle::RemoveObserver(ViewLifecycleObserver* observer) const {
  observers_.RemoveObserver(observer);
}

void ViewLifecycle::StartOver() {
  if (state_ == State::VisualUpdatePending)
    return;
  ChangeState(State::VisualUpdatePending);
}

void ViewLifecycle::StartShutdown() {
  DCHECK_NE(State::InShutdown, state_);
  DCHECK_NE(State::Shutdown, state_);
  ChangeState(State::InShutdown);
}
// Printers
std::ostream& operator<<(std::ostream& ostream,
                         const ViewLifecycle& lifecycle) {
  return ostream << lifecycle.state();
}

std::ostream& operator<<(std::ostream& ostream,
                         const ViewLifecycle* lifecycle) {
  if (!lifecycle)
    return ostream << "null";
  return ostream << *lifecycle;
}

std::ostream& operator<<(std::ostream& ostream, ViewLifecycle::State state) {
  static const char* texts[] = {
#define V(name) #name,
      FOR_EACH_TREE_LIFECYCLE_STATE(V)
#undef V
  };
  const auto& it = std::begin(texts) + static_cast<size_t>(state);
  if (it < std::begin(texts) || it >= std::end(texts))
    return ostream << "???";
  return ostream << *it;
}

}  // namespace visuals
