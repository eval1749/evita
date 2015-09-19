// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_
#define EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_

#include "common/memory/singleton.h"
#include "evita/ui/events/event.h"

namespace ui {

// Tracking mouse click.
class MouseClickTracker final : public common::Singleton<MouseClickTracker> {
  DECLARE_SINGLETON_CLASS(MouseClickTracker);

 public:
  ~MouseClickTracker();

  int click_count() const { return click_count_; }
  void OnMousePressed(const MouseEvent& event);
  void OnMouseReleased(const MouseEvent& event);

 private:
  // These values match the Windows defaults.
  static const auto kDoubleClickTimeMS = 500;
  static const auto kDoubleClickWidth = 4;
  static const auto kDoubleClickHeight = 4;

  enum class State {
    Start,
    Pressed,
    PressedReleased,
    PressedReleasedPressed,
  };

  MouseClickTracker();

  bool is_repeated_event(const MouseEvent& event) const;
  bool is_same_location(const MouseEvent& event) const;
  bool is_same_time(const MouseEvent& event) const;

  int click_count_;
  MouseEvent last_event_;
  State state_;

  DISALLOW_COPY_AND_ASSIGN(MouseClickTracker);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_MOUSE_CLICK_TRACKER_H_
