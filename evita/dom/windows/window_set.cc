// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/window_set.h"

#include "base/logging.h"
#include "evita/dom/windows/window.h"
#include "evita/dom/windows/window_ostream.h"

namespace dom {

WindowSet::WindowSet() {
}

WindowSet::~WindowSet() {
}

void WindowSet::DidDestroyWidget(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  auto it = map_.find(window_id);
  if (it == map_.end()) {
    DVLOG(0) << "Why we don't have a widget for WindowId " << window_id <<
      " in WindowIdMap?";
    return;
  }
  auto const window = it->second.get();
  DCHECK_NE(Window::State::Destroyed, window->state_);
  window->state_ = Window::State::Destroyed;
  if (auto const parent = window->parent_node()) {
    parent->RemoveChild(window);
  }
}

Window* WindowSet::Find(WindowId window_id) const {
  auto it = map_.find(window_id);
  return it == map_.end() ? nullptr : it->second.get();
}

void WindowSet::Register(Window* window) {
  map_[window->window_id()] = window;
}

void WindowSet::ResetForTesting() {
  map_.clear();
}

void WindowSet::Unregister(WindowId window_id) {
  DCHECK_NE(kInvalidWindowId, window_id);
  map_.erase(window_id);
}

}  // namespace dom
