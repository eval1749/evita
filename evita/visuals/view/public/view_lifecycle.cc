// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/view/public/view_lifecycle.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"

namespace visuals {

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
ViewLifecycle::ViewLifecycle(const Document& document)
    : document_(document), state_(State::VisualUpdatePending) {
  document_.AddObserver(this);
}

ViewLifecycle::~ViewLifecycle() {
  DCHECK_EQ(State::Shutdown, state_);
  document_.RemoveObserver(this);
}

void ViewLifecycle::Advance() {
  DCHECK_NE(State::PaintClean, state_);
  DCHECK_NE(State::InShutdown, state_);
  DCHECK_NE(State::Shutdown, state_);
  state_ = static_cast<State>(static_cast<int>(state_) + 1);
}

bool ViewLifecycle::AllowsTreeMutaions() const {
  static bool allows[static_cast<size_t>(State::PaintClean) + 1];
  if (!allows[static_cast<size_t>(State::VisualUpdatePending)]) {
    allows[static_cast<size_t>(State::VisualUpdatePending)] = true;
    allows[static_cast<size_t>(State::LayoutClean)] = true;
    allows[static_cast<size_t>(State::PaintClean)] = true;
    allows[static_cast<size_t>(State::StyleClean)] = true;
  }
  return allows[static_cast<size_t>(state_)];
}

void ViewLifecycle::FinishShutdown() {
  DCHECK_EQ(State::InShutdown, state_);
  state_ = State::Shutdown;
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
  state_ = limit_state;
}
void ViewLifecycle::Reset() {
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::StartShutdown() {
  DCHECK_NE(State::InShutdown, state_);
  DCHECK_NE(State::Shutdown, state_);
  state_ = State::InShutdown;
}

// DocumentObserver
void ViewLifecycle::DidAddClass(const ElementNode& element,
                                const base::string16& new_name) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidAppendChild(const ContainerNode& parent,
                                   const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidChangeInlineStyle(const ElementNode& element,
                                         const css::Style* old_style) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidInsertBefore(const ContainerNode& parent,
                                    const Node& child,
                                    const Node& ref_child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidRemoveChild(const ContainerNode& parent,
                                   const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidRemoveClass(const ElementNode& element,
                                   const base::string16& old_name) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidReplaceChild(const ContainerNode& parent,
                                    const Node& new_child,
                                    const Node& old_child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::DidSetTextData(const Text& text,
                                   const base::string16& new_data,
                                   const base::string16& old_data) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void ViewLifecycle::WillRemoveChild(const ContainerNode& parent,
                                    const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
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
