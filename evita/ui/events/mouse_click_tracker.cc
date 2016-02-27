// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/mouse_click_tracker.h"

#include "base/logging.h"
#include "base/memory/singleton.h"

namespace ui {

namespace {
// These values match the Windows defaults.
const auto kDoubleClickTimeMS = 500;
const auto kDoubleClickWidth = 4;
const auto kDoubleClickHeight = 4;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MouseClickTracker
//
enum class MouseClickTracker::State {
  Start,
  Pressed,
  PressedReleased,
  PressedReleasedPressed,
};

MouseClickTracker::MouseClickTracker() : state_(State::Start) {}
MouseClickTracker::~MouseClickTracker() {}

bool MouseClickTracker::is_repeated_event(const MouseEvent& event) const {
  return is_same_location(event) && is_same_time(event);
}

bool MouseClickTracker::is_same_location(const MouseEvent& event) const {
  if (last_event_.target() != event.target())
    return false;
  Rect rect(last_event_.screen_location().x() - kDoubleClickWidth / 2,
            last_event_.screen_location().y() - kDoubleClickHeight / 2,
            last_event_.screen_location().x() + kDoubleClickWidth / 2,
            last_event_.screen_location().y() + kDoubleClickHeight / 2);
  return rect.Contains(event.screen_location());
}

bool MouseClickTracker::is_same_time(const MouseEvent& event) const {
  auto const diff = event.time_stamp() - last_event_.time_stamp();
  return diff.InMilliseconds() <= kDoubleClickTimeMS;
}

// static
MouseClickTracker* MouseClickTracker::GetInstance() {
  return base::Singleton<MouseClickTracker>::get();
}

void MouseClickTracker::OnMousePressed(const MouseEvent& event) {
  DCHECK_EQ(EventType::MousePressed, event.type());

  click_count_ = 0;
  auto const state = state_;
  state_ = State::Start;
  if (state == State::Start) {
    last_event_ = event;
    state_ = State::Pressed;
    return;
  }
  if (state == State::PressedReleased) {
    if (is_repeated_event(event)) {
      state_ = State::PressedReleasedPressed;
      return;
    }
    last_event_ = event;
    state_ = State::Pressed;
    return;
  }
}

void MouseClickTracker::OnMouseReleased(const MouseEvent& event) {
  DCHECK_EQ(EventType::MouseReleased, event.type());

  click_count_ = 0;
  auto const state = state_;
  state_ = State::Start;
  if (state == State::Pressed) {
    if (last_event_.target() != event.target())
      return;
    click_count_ = 1;
    if (is_repeated_event(event))
      state_ = State::PressedReleased;
    return;
  }
  if (state == State::PressedReleasedPressed) {
    if (is_repeated_event(event))
      click_count_ = 2;
    return;
  }

  // MousePressed is occurred in window which we don't managed.
}

}  // namespace ui
