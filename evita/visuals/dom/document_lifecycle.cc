// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>
#include <ostream>

#include "evita/visuals/dom/document_lifecycle.h"

#include "base/logging.h"
#include "evita/visuals/dom/document.h"

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
DocumentLifecycle::DocumentLifecycle(const Document& document)
    : document_(document), state_(State::VisualUpdatePending) {
  document_.AddObserver(this);
}

DocumentLifecycle::~DocumentLifecycle() {
  document_.RemoveObserver(this);
}

void DocumentLifecycle::AdvanceTo(State new_state) {
  DCHECK(static_cast<int>(new_state) == static_cast<int>(state_) + 1)
      << "Can't advance to " << new_state << " from " << state_;
  state_ = new_state;
}

bool DocumentLifecycle::AllowsTreeMutaions() const {
  return state_ == State::VisualUpdatePending || state_ == State::LayoutClean ||
         state_ == State::PaintClean;
}

bool DocumentLifecycle::IsAtLeast(State state) const {
  return static_cast<int>(state_) >= static_cast<int>(state);
}

void DocumentLifecycle::LimitTo(State limit_state) {
  if (static_cast<int>(state_) <= static_cast<int>(limit_state))
    return;
  state_ = limit_state;
}
void DocumentLifecycle::Reset() {
  state_ = State::VisualUpdatePending;
}

// DocumentObserver
void DocumentLifecycle::DidAddClass(const ElementNode& element,
                                    const base::string16& new_name) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidAppendChild(const ContainerNode& parent,
                                       const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidChangeInlineStyle(const ElementNode& element,
                                             const css::Style* old_style) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidInsertBefore(const ContainerNode& parent,
                                        const Node& child,
                                        const Node& ref_child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidRemoveChild(const ContainerNode& parent,
                                       const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidRemoveClass(const ElementNode& element,
                                       const base::string16& old_name) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidReplaceChild(const ContainerNode& parent,
                                        const Node& new_child,
                                        const Node& old_child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::DidSetTextData(const Text& text,
                                       const base::string16& new_data,
                                       const base::string16& old_data) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

void DocumentLifecycle::WillRemoveChild(const ContainerNode& parent,
                                        const Node& child) {
  DCHECK(AllowsTreeMutaions()) << state_;
  state_ = State::VisualUpdatePending;
}

// Printers
std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle& lifecycle) {
  return ostream << lifecycle.state();
}

std::ostream& operator<<(std::ostream& ostream,
                         const DocumentLifecycle* lifecycle) {
  if (!lifecycle)
    return ostream << "null";
  return ostream << *lifecycle;
}

std::ostream& operator<<(std::ostream& ostream,
                         DocumentLifecycle::State state) {
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
