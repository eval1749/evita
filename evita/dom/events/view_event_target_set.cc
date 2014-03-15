// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/view_event_target_set.h"

#include "evita/dom/events/view_event_target.h"

namespace dom {

ViewEventTargetSet::ViewEventTargetSet() : next_event_target_id_(1) {
}

ViewEventTargetSet::~ViewEventTargetSet() {
}

void ViewEventTargetSet::DidDestroyWidget(EventTargetId event_target_id) {
  DCHECK_NE(kInvalidEventTargetId, event_target_id);
  auto it = map_.find(event_target_id);
  if (it == map_.end()) {
    DVLOG(0) << "Why we don't have a widget for EventTargetId " <<
      event_target_id << " in EventTargetIdMap?";
    return;
  }
  auto const event_target = it->second.get();
  event_target->event_target_id_ = kInvalidEventTargetId;
}

ViewEventTarget* ViewEventTargetSet::Find(
    EventTargetId event_target_id) const {
  auto it = map_.find(event_target_id);
  return it == map_.end() ? nullptr : it->second.get();
}

EventTargetId ViewEventTargetSet::Register(ViewEventTarget* event_target) {
  auto event_target_id = next_event_target_id_;
  map_[event_target_id] = event_target;
  ++next_event_target_id_;
  return event_target_id;
}

void ViewEventTargetSet::ResetForTesting() {
  next_event_target_id_ = 1;
  map_.clear();
}

void ViewEventTargetSet::Unregister(EventTargetId event_target_id) {
  DCHECK_NE(kInvalidEventTargetId, event_target_id);
  map_.erase(event_target_id);
}

}  // namespace dom
